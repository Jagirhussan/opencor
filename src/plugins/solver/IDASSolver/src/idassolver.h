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
// IDAS solver
//==============================================================================

#pragma once

//==============================================================================

#include "solverinterface.h"

//==============================================================================

#include "nvector/nvector_serial.h"
#include "sundials/sundials_linearsolver.h"
#include "sundials/sundials_matrix.h"

//==============================================================================

namespace OpenCOR {
namespace IDASSolver {

//==============================================================================

static const auto MaximumStepId          = QStringLiteral("MaximumStep");
static const auto MaximumNumberOfStepsId = QStringLiteral("MaximumNumberOfSteps");
static const auto LinearSolverId         = QStringLiteral("LinearSolver");
static const auto UpperHalfBandwidthId   = QStringLiteral("UpperHalfBandwidth");
static const auto LowerHalfBandwidthId   = QStringLiteral("LowerHalfBandwidth");
static const auto RelativeToleranceId    = QStringLiteral("RelativeTolerance");
static const auto AbsoluteToleranceId    = QStringLiteral("AbsoluteTolerance");
static const auto InterpolateSolutionId  = QStringLiteral("InterpolateSolution");

//==============================================================================

static const auto DenseLinearSolver    = QStringLiteral("Dense");
static const auto BandedLinearSolver   = QStringLiteral("Banded");
static const auto GmresLinearSolver    = QStringLiteral("GMRES");
static const auto BiCgStabLinearSolver = QStringLiteral("BiCGStab");
static const auto TfqmrLinearSolver    = QStringLiteral("TFQMR");

//==============================================================================

// Default IDAS parameter values
// Note #1: a maximum step of 0 means that there is no maximum step as such and
//          that IDAS can use whatever step it sees fit...
// Note #2: IDAS' default maximum number of steps is 500 which ought to be big
//          enough in most cases...

static const double MaximumStepDefaultValue = 0.0;

enum {
    MaximumNumberOfStepsDefaultValue = 500
};

static const auto LinearSolverDefaultValue = DenseLinearSolver;

enum {
    UpperHalfBandwidthDefaultValue = 0,
    LowerHalfBandwidthDefaultValue = 0
};

static const double RelativeToleranceDefaultValue = 1.0e-7;
static const double AbsoluteToleranceDefaultValue = 1.0e-7;

static const bool InterpolateSolutionDefaultValue = true;

//==============================================================================

class IdasSolverUserData
{
public:
    explicit IdasSolverUserData(double *pConstants, double *pOldRates,
                                double *pOldStates, double *pAlgebraic,
                                double *pCondVar,
                                Solver::DaeSolver::ComputeRatesFunction pComputeRates,
                                Solver::DaeSolver::ComputeEssentialVariablesFunction pComputeEssentialVariables,
                                Solver::DaeSolver::ComputeRootInformationFunction pComputeRootInformation);

    double * constants() const;
    double * oldRates() const;
    double * oldStates() const;
    double * algebraic() const;
    double * condVar() const;

    Solver::DaeSolver::ComputeRatesFunction computeRates() const;
    Solver::DaeSolver::ComputeEssentialVariablesFunction computeEssentialVariables() const;
    Solver::DaeSolver::ComputeRootInformationFunction computeRootInformation() const;

private:
    double *mConstants;
    double *mOldRates;
    double *mOldStates;
    double *mAlgebraic;
    double *mCondVar;

    Solver::DaeSolver::ComputeRatesFunction mComputeRates;
    Solver::DaeSolver::ComputeEssentialVariablesFunction mComputeEssentialVariables;
    Solver::DaeSolver::ComputeRootInformationFunction mComputeRootInformation;
};

//==============================================================================

class IdasSolver : public OpenCOR::Solver::DaeSolver
{
    Q_OBJECT

public:
    explicit IdasSolver();
    ~IdasSolver();

    virtual void initialize(const double &pVoiStart, const double &pVoiEnd,
                            const int &pRatesStatesCount,
                            const int &pCondVarCount, double *pConstants,
                            double *pRates, double *pStates, double *pAlgebraic,
                            double *pCondVar,
                            ComputeRatesFunction pComputeRates,
                            ComputeEssentialVariablesFunction pComputeEssentialVariables,
                            ComputeRootInformationFunction pComputeRootInformation,
                            ComputeStateInformationFunction pComputeStateInformation);

    virtual void solve(double &pVoi, const double &pVoiEnd) const;

private:
    void *mSolver;
    N_Vector mRatesVector;
    N_Vector mStatesVector;
    SUNMatrix mMatrix;
    SUNLinearSolver mLinearSolver;
    IdasSolverUserData *mUserData;

    bool mInterpolateSolution;
};

//==============================================================================

}   // namespace IDASSolver
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
