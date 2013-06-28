//==============================================================================
// Single cell view information widget
//==============================================================================

#include "collapsiblewidget.h"
#include "coreutils.h"
#include "singlecellviewinformationparameterswidget.h"
#include "singlecellviewinformationsimulationwidget.h"
#include "singlecellviewinformationsolverswidget.h"
#include "singlecellviewinformationtraceswidget.h"
#include "singlecellviewinformationwidget.h"

//==============================================================================

#include <QSettings>

//==============================================================================

#include "ui_singlecellviewinformationwidget.h"

//==============================================================================

namespace OpenCOR {
namespace SingleCellView {

//==============================================================================

SingleCellViewInformationWidget::SingleCellViewInformationWidget(QWidget *pParent) :
    QScrollArea(pParent),
    CommonWidget(pParent),
    mGui(new Ui::SingleCellViewInformationWidget)
{
    // Set up the GUI

    mGui->setupUi(this);

    // Remove the frame around our scroll area

    setFrameShape(QFrame::NoFrame);

    // Create our collapsible widget

    mCollapsibleWidget = new Core::CollapsibleWidget(this);

    mCollapsibleWidget->setObjectName("Collapsible");

    // Create our simulation widget

    mSimulationWidget = new SingleCellViewInformationSimulationWidget(mCollapsibleWidget);

    mSimulationWidget->setObjectName("Simulation");

    // Create our solvers widget

    mSolversWidget = new SingleCellViewInformationSolversWidget(mCollapsibleWidget);

    mSolversWidget->setObjectName("Solvers");

    // Create our traces widget

    mTracesWidget = new SingleCellViewInformationTracesWidget(mCollapsibleWidget);

    mTracesWidget->setObjectName("Traces");

    // Create our parameters widget

    mParametersWidget = new SingleCellViewInformationParametersWidget(mCollapsibleWidget);

    mParametersWidget->setObjectName("Parameters");

    // Add our simulation, solvers, traces and parameters widgets to our
    // collapsible widget

    mCollapsibleWidget->addWidget(mSimulationWidget);
    mCollapsibleWidget->addWidget(mSolversWidget);
    mCollapsibleWidget->addWidget(mTracesWidget);
    mCollapsibleWidget->addWidget(mParametersWidget);

    // Add our collapsible widget to our layout

    mGui->layout->addWidget(mCollapsibleWidget);

    // Some further initialisations which are done as part of retranslating the
    // GUI (so that they can be updated when changing languages)

    retranslateUi();
}

//==============================================================================

SingleCellViewInformationWidget::~SingleCellViewInformationWidget()
{
    // Delete the GUI

    delete mGui;
}

//==============================================================================

void SingleCellViewInformationWidget::retranslateUi()
{
    // Retranslate the whole widget

    mGui->retranslateUi(this);

    // Retranslate the different titles of our collapsible widget

    mCollapsibleWidget->setHeaderTitle(0, tr("Simulation"));
    mCollapsibleWidget->setHeaderTitle(1, tr("Solvers"));
    mCollapsibleWidget->setHeaderTitle(2, tr("Traces"));
    mCollapsibleWidget->setHeaderTitle(3, tr("Parameters"));

    // Retranslate our simulation, solvers, traces and parameters widgets

    mSimulationWidget->retranslateUi();
    mSolversWidget->retranslateUi();
    mTracesWidget->retranslateUi();
    mParametersWidget->retranslateUi();
}

//==============================================================================

void SingleCellViewInformationWidget::loadSettings(QSettings *pSettings)
{
    // Retrieve the settings of our collapsible widget

    pSettings->beginGroup(mCollapsibleWidget->objectName());
        mCollapsibleWidget->loadSettings(pSettings);
    pSettings->endGroup();

    // Retrieve the settings of our simulation widget

    pSettings->beginGroup(mSimulationWidget->objectName());
        mSimulationWidget->loadSettings(pSettings);
    pSettings->endGroup();

    // Retrieve the settings of our solvers widget

    pSettings->beginGroup(mSolversWidget->objectName());
        mSolversWidget->loadSettings(pSettings);
    pSettings->endGroup();

    // Retrieve the settings of our traces widget

    pSettings->beginGroup(mTracesWidget->objectName());
        mTracesWidget->loadSettings(pSettings);
    pSettings->endGroup();

    // Retrieve the settings of our parameters widget

    pSettings->beginGroup(mParametersWidget->objectName());
        mParametersWidget->loadSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

void SingleCellViewInformationWidget::saveSettings(QSettings *pSettings) const
{
    // Keep track of the settings of our collapsible widget

    pSettings->beginGroup(mCollapsibleWidget->objectName());
        mCollapsibleWidget->saveSettings(pSettings);
    pSettings->endGroup();

    // Keep track of the settings of our simulation widget

    pSettings->beginGroup(mSimulationWidget->objectName());
        mSimulationWidget->saveSettings(pSettings);
    pSettings->endGroup();

    // Keep track of the settings of our solvers widget

    pSettings->beginGroup(mSolversWidget->objectName());
        mSolversWidget->saveSettings(pSettings);
    pSettings->endGroup();

    // Keep track of the settings of our traces widget

    pSettings->beginGroup(mTracesWidget->objectName());
        mTracesWidget->saveSettings(pSettings);
    pSettings->endGroup();

    // Keep track of the settings of our parameters widget

    pSettings->beginGroup(mParametersWidget->objectName());
        mParametersWidget->saveSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

SingleCellViewInformationSimulationWidget * SingleCellViewInformationWidget::simulationWidget()
{
    // Return our simulation widget

    return mSimulationWidget;
}

//==============================================================================

SingleCellViewInformationSolversWidget * SingleCellViewInformationWidget::solversWidget()
{
    // Return our solvers widget

    return mSolversWidget;
}

//==============================================================================

SingleCellViewInformationTracesWidget * SingleCellViewInformationWidget::tracesWidget()
{
    // Return our traces widget

    return mTracesWidget;
}

//==============================================================================

SingleCellViewInformationParametersWidget * SingleCellViewInformationWidget::parametersWidget()
{
    // Return our parameters widget

    return mParametersWidget;
}

//==============================================================================

void SingleCellViewInformationWidget::finishEditing()
{
    // Finish the editing of any of the information we support

    mSimulationWidget->finishPropertyEditing();
    mSolversWidget->finishPropertyEditing();
    mTracesWidget->finishPropertyEditing();
    mParametersWidget->finishPropertyEditing();
}

//==============================================================================

}   // namespace SingleCellView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
