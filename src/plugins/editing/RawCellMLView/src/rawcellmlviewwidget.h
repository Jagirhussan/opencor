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
// Raw CellML view widget
//==============================================================================

#ifndef RAWCELLMLVIEWWIDGET_H
#define RAWCELLMLVIEWWIDGET_H

//==============================================================================

#include "viewwidget.h"

//==============================================================================

#include <QMap>

//==============================================================================

namespace Ui {
    class RawCellmlViewWidget;
}

//==============================================================================

namespace OpenCOR {

//==============================================================================

namespace CoreCellMLEditing {
    class CoreCellmlEditingWidget;
}   // namespace QScintillaSupport

//==============================================================================

namespace QScintillaSupport {
    class QScintillaWidget;
}   // namespace QScintillaSupport

//==============================================================================

namespace RawCellMLView {

//==============================================================================

class RawCellmlViewWidget : public Core::ViewWidget
{
    Q_OBJECT

public:
    explicit RawCellmlViewWidget(QWidget *pParent = 0);
    ~RawCellmlViewWidget();

    virtual void loadSettings(QSettings *pSettings);
    virtual void saveSettings(QSettings *pSettings) const;

    bool contains(const QString &pFileName) const;

    void initialize(const QString &pFileName);
    void finalize(const QString &pFileName);

    void fileReloaded(const QString &pFileName);
    void fileRenamed(const QString &pOldFileName, const QString &pNewFileName);

    QScintillaSupport::QScintillaWidget * editor(const QString &pFileName) const;

private:
    Ui::RawCellmlViewWidget *mGui;

    CoreCellMLEditing::CoreCellmlEditingWidget *mEditingWidget;
    QMap<QString, CoreCellMLEditing::CoreCellmlEditingWidget *> mEditingWidgets;

    QList<int> mEditingWidgetSizes;
    int mEditorZoomLevel;

private Q_SLOTS:
    void splitterMoved();
    void editorZoomLevelChanged();
};

//==============================================================================

}   // namespace RawCellMLView
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
