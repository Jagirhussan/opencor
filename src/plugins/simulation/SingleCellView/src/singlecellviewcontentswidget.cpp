/*******************************************************************************

Licensed to the OpenCOR team under one or more contributor license agreements.
See the NOTICE.txt file distributed with this work for additional information
regarding copyright ownership. The OpenCOR team licenses this file to you under
the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

*******************************************************************************/

//==============================================================================
// Single cell view contents widget
//==============================================================================

#include "borderedwidget.h"
#include "singlecellviewcontentswidget.h"
#include "singlecellviewgraphpanelswidget.h"
#include "singlecellviewinformationwidget.h"

//==============================================================================

#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>

//==============================================================================

namespace OpenCOR {
namespace SingleCellView {

//==============================================================================

SingleCellViewContentsWidget::SingleCellViewContentsWidget(QWidget *pParent) :
    QSplitter(pParent),
    CommonWidget(pParent),
    mSplitterSizes(QList<int>())
{
    // Keep track of our movement
    // Note: we need to keep track of our movement so that saveSettings() can
    //       work fine even when we are not visible (which happens when a CellML
    //       file cannot be run for some reason or another)...

    connect(this, SIGNAL(splitterMoved(int, int)),
            this, SLOT(splitterMoved()));

    // Create our information widget

    mInformationWidget = new SingleCellViewInformationWidget(this);

    mInformationWidget->setObjectName("Information");

    // Create our graph panels widget

    mGraphPanelsWidget = new SingleCellViewGraphPanelsWidget(this);

    mGraphPanelsWidget->setObjectName("GraphPanels");

    // Add our information and graph panels widgets to ourselves

    addWidget(new Core::BorderedWidget(mInformationWidget,
                                       false, false, true, true));
    addWidget(new Core::BorderedWidget(mGraphPanelsWidget,
                                       false, true, true, false));

    // Make our graph panels widget our focus proxy

    setFocusProxy(mGraphPanelsWidget);
}

//==============================================================================

void SingleCellViewContentsWidget::retranslateUi()
{
    // Retranslate our information and graph panels widgets

    mInformationWidget->retranslateUi();
    mGraphPanelsWidget->retranslateUi();
}

//==============================================================================

static const auto SettingsContentsCount = QStringLiteral("ContentsCount");
static const auto SettingsContentsSize  = QStringLiteral("ContentsSize%1");

//==============================================================================

void SingleCellViewContentsWidget::loadSettings(QSettings *pSettings)
{
    // Retrieve and set our sizes

    int sizesCount = pSettings->value(SettingsContentsCount, 0).toInt();
    mSplitterSizes = QList<int>();

    if (!sizesCount)
        // There are no previous sizes, so get some default ones

        mSplitterSizes << 0.25*qApp->desktop()->screenGeometry().width()
                       << 0.75*qApp->desktop()->screenGeometry().width();
    else
        for (int i = 0; i < sizesCount; ++i)
            mSplitterSizes << pSettings->value(SettingsContentsSize.arg(i)).toInt();

    setSizes(mSplitterSizes);

    // Retrieve the settings of our information and graph panels widgets

    pSettings->beginGroup(mInformationWidget->objectName());
        mInformationWidget->loadSettings(pSettings);
    pSettings->endGroup();

    pSettings->beginGroup(mGraphPanelsWidget->objectName());
        mGraphPanelsWidget->loadSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

void SingleCellViewContentsWidget::saveSettings(QSettings *pSettings) const
{
    // Keep track of our sizes

    pSettings->setValue(SettingsContentsCount, mSplitterSizes.count());

    for (int i = 0, iMax = mSplitterSizes.count(); i < iMax; ++i)
        pSettings->setValue(SettingsContentsSize.arg(i), mSplitterSizes[i]);

    // Keep track of the settings of our information and graph panels widgets

    pSettings->beginGroup(mInformationWidget->objectName());
        mInformationWidget->saveSettings(pSettings);
    pSettings->endGroup();

    pSettings->beginGroup(mGraphPanelsWidget->objectName());
        mGraphPanelsWidget->saveSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

SingleCellViewInformationWidget * SingleCellViewContentsWidget::informationWidget()
{
    // Return our information widget

    return mInformationWidget;
}

//==============================================================================

SingleCellViewGraphPanelsWidget * SingleCellViewContentsWidget::graphPanelsWidget()
{
    // Return our graph panels widget

    return mGraphPanelsWidget;
}

//==============================================================================

void SingleCellViewContentsWidget::splitterMoved()
{
    // Our splitter has been moved, so keep track of its new sizes

    mSplitterSizes = sizes();
}

//==============================================================================

}   // namespace SingleCellView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
