/*******************************************************************************

Copyright (C) The University of Auckland

OpenCOR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenCOR is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************/

//==============================================================================
// Simulation worker
//==============================================================================

#include "cellmlfileruntime.h"
#include "corecliutils.h"
#include "simulation.h"
#include "simulationworker.h"

//==============================================================================

#include <QElapsedTimer>
#include <QMutex>
#include <QThread>

//==============================================================================

namespace OpenCOR {
namespace SimulationSupport {

//==============================================================================

SimulationWorker::SimulationWorker(Simulation *pSimulation,
                                   SimulationWorker *&pSelf) :
    mSimulation(pSimulation),
    mRuntime(pSimulation->runtime()),
    mCurrentPoint(0.0),
    mPaused(false),
    mStopped(false),
    mReset(false),
    mError(false),
    mSelf(pSelf)
{
    // Create our thread

    mThread = new QThread();

    // Move ourselves to our thread

    moveToThread(mThread);

    // Create a few connections

    connect(mThread, SIGNAL(started()),
            this, SLOT(started()));

    connect(this, SIGNAL(finished(const qint64 &)),
            mThread, SLOT(quit()));

    connect(mThread, SIGNAL(finished()),
            mThread, SLOT(deleteLater()));
    connect(mThread, SIGNAL(finished()),
            this, SLOT(deleteLater()));
}

//==============================================================================

bool SimulationWorker::isRunning() const
{
    // Return whether our thread is running

    return mThread->isRunning() && !mPaused;
}

//==============================================================================

bool SimulationWorker::isPaused() const
{
    // Return whether our thread is paused

    return mThread->isRunning() && mPaused;
}

//==============================================================================

double SimulationWorker::currentPoint() const
{
    // Return our current point

    return mThread->isRunning()?
               mCurrentPoint:
               mSimulation->data()->startingPoint();
}

//==============================================================================

bool SimulationWorker::run()
{
    // Start our thread, but only if we are not already running

    if (!mThread->isRunning()) {
        mThread->start();

        return true;
    } else {
        return false;
    }
}

//==============================================================================

bool SimulationWorker::pause()
{
    // Pause ourselves, but only if we are currently running

    if (isRunning()) {
        // Ask ourselves to pause

        mPaused = true;

        return true;
    } else {
        return false;
    }
}

//==============================================================================

bool SimulationWorker::resume()
{
    // Resume ourselves, but only if are currently paused

    if (isPaused()) {
        // Ask ourselves to resume

        mPausedCondition.wakeOne();

        return true;
    } else {
        return false;
    }
}

//==============================================================================

bool SimulationWorker::stop()
{
    // Check that we are either running or paused

    if (isRunning() || isPaused()) {
        // Ask our thread to stop

        mStopped = true;

        // Resume our thread, if needed

        if (isPaused())
            mPausedCondition.wakeOne();

        // Ask our thread to quit and wait for it to do so

        mThread->quit();
        mThread->wait();

        return true;
    } else {
        return false;
    }
}

//==============================================================================

bool SimulationWorker::reset()
{
    // Check that we are either running or paused

    if (isRunning() || isPaused()) {
        // Ask ourselves to reinitialise our solver

        mReset = true;

        return true;
    } else {
        return false;
    }
}

//==============================================================================

void SimulationWorker::started()
{
    // Let people know that we are running

    emit running(false);

    // Set up our ODE solver

    Solver::OdeSolver *odeSolver = static_cast<Solver::OdeSolver *>(mSimulation->data()->odeSolverInterface()->solverInstance());

    // Set up our NLA solver, if needed
    // Note: we unset it at the end of this method...

    Solver::NlaSolver *nlaSolver = 0;

    if (mRuntime->needNlaSolver()) {
        nlaSolver = static_cast<Solver::NlaSolver *>(mSimulation->data()->nlaSolverInterface()->solverInstance());

        Solver::setNlaSolver(mRuntime->address(), nlaSolver);
    }

    // Keep track of any error that might be reported by any of our solvers

    mStopped = false;
    mError = false;

    connect(odeSolver, SIGNAL(error(const QString &)),
            this, SLOT(emitError(const QString &)));

    if (nlaSolver) {
        connect(nlaSolver, SIGNAL(error(const QString &)),
                this, SLOT(emitError(const QString &)));
    }

    // Retrieve our simulation properties

    double startingPoint = mSimulation->data()->startingPoint();
    double endingPoint = mSimulation->data()->endingPoint();
    double pointInterval = mSimulation->data()->pointInterval();

    bool increasingPoints = endingPoint > startingPoint;
    quint64 pointCounter = 0;

    mCurrentPoint = startingPoint;

    // Initialise our ODE solver

    odeSolver->setProperties(mSimulation->data()->odeSolverProperties());

    odeSolver->initialize(mCurrentPoint, mRuntime->statesCount(),
                          mSimulation->data()->constants(),
                          mSimulation->data()->rates(),
                          mSimulation->data()->states(),
                          mSimulation->data()->algebraic(),
                          mRuntime->computeRates());

    // Initialise our NLA solver, if any

    if (nlaSolver)
        nlaSolver->setProperties(mSimulation->data()->nlaSolverProperties());

    // Now, we are ready to compute our model, but only if no error has occurred
    // so far
    // Note: we use -1 as a way to indicate that something went wrong...

    qint64 elapsedTime = 0;

    if (!mError) {
        // Start our timer

        QElapsedTimer timer;

        timer.start();

        // Add our first point after making sure that all the variables are up
        // to date

        mSimulation->data()->recomputeVariables(mCurrentPoint);

        mSimulation->results()->addPoint(mCurrentPoint);

        // Our main work loop
        // Note: for performance reasons, it is essential that the following
        //       loop doesn't emit any signal, be it directly or indirectly,
        //       unless it is to let people know that we are pausing or running.
        //       Indeed, the signal/slot mechanism adds a certain level of
        //       overhead and, here, we want things to be as fast as possible...

        QMutex pausedMutex;

        forever {
            // Determine our next point and compute our model up to it

            ++pointCounter;

            odeSolver->solve(mCurrentPoint,
                             increasingPoints?
                                 qMin(endingPoint, startingPoint+pointCounter*pointInterval):
                                 qMax(endingPoint, startingPoint+pointCounter*pointInterval));

            // Make sure that no error occurred

            if (mError)
                break;

            // Add our new point after making sure that all the variables are up
            // to date

            mSimulation->data()->recomputeVariables(mCurrentPoint);

            mSimulation->results()->addPoint(mCurrentPoint);

            // Some post-processing, if needed

            if ((mCurrentPoint == endingPoint) || mStopped) {
                // We have reached our ending point or we have been asked to
                // stop, so leave our main work loop

                break;
            } else {
                // Delay things a bit, if needed

                if (mSimulation->delay())
                    Core::doNothing(100*mSimulation->delay());

                // Pause ourselves, if needed

                if (mPaused) {
                    // We should be paused, so stop our timer

                    elapsedTime += timer.elapsed();

                    // Let people know that we are paused

                    emit paused();

                    // Actually pause ourselves

                    pausedMutex.lock();
                        mPausedCondition.wait(&pausedMutex);
                    pausedMutex.unlock();

                    // We are not paused anymore

                    mPaused = false;

                    // Let people know that we are running again

                    emit running(true);

                    // (Re)start our timer

                    timer.start();

                }

                // Reinitialise our solver, if we have an NLA solver or if the
                // model got reset
                // Note: indeed, with a solver such as CVODE, to solve an NLA
                //       system requires updating its internals...

                if (nlaSolver || mReset) {
                    odeSolver->reinitialize(mCurrentPoint);

                    mReset = false;
                }
            }
        }

        // Retrieve the total elapsed time, should no error have occurred

        if (!mError)
            elapsedTime += timer.elapsed();
    }

    // Delete our solver(s)

    delete odeSolver;

    if (nlaSolver) {
        delete nlaSolver;

        Solver::unsetNlaSolver(mRuntime->address());
    }

    // Reset our simulation owner's knowledge of us
    // Note: if we were to do it the Qt way, our simulation owner would have a
    //       slot for our finished() signal, but we want our simulation owner to
    //       know as quickly as possible that we are done...

    mSelf = 0;

    // Let people know that we are done and give them the elapsed time

    emit finished(mError?-1:elapsedTime);
}

//==============================================================================

void SimulationWorker::emitError(const QString &pMessage)
{
    // A solver error occurred, so keep track of it and let people know about it

    mError = true;

    emit error(pMessage);
}

//==============================================================================

}   // namespace SimulationSupport
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
