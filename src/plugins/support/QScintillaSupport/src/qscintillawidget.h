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
// QScintillaWidget class
//==============================================================================

#ifndef QSCINTILLAWIDGET_H
#define QSCINTILLAWIDGET_H

//==============================================================================

#include "qscintillasupportglobal.h"

//==============================================================================

#include <QFont>

//==============================================================================

#include "Qsci/qsciscintilla.h"

//==============================================================================

class QLabel;

//==============================================================================

namespace OpenCOR {
namespace QScintillaSupport {

//==============================================================================

class QSCINTILLASUPPORT_EXPORT QScintillaWidget : public QsciScintilla
{
    Q_OBJECT

public:
    explicit QScintillaWidget(const QString &pContents, const bool &pReadOnly,
                              QsciLexer *pLexer, QWidget *pParent = 0);
    ~QScintillaWidget();

    QMenu * contextMenu() const;
    void setContextMenu(const QList<QAction *> &pContextMenuActions);

    QString contents() const;
    void setContents(const QString &pContents);

    bool isSelectAllAvailable() const;

    void resetUndoHistory();

    QLabel * cursorPositionWidget() const;
    QLabel * editingModeWidget() const;

protected:
    virtual void changeEvent(QEvent *pEvent);
    virtual void contextMenuEvent(QContextMenuEvent *pEvent);
    virtual void dragEnterEvent(QDragEnterEvent *pEvent);
    virtual bool event(QEvent *pEvent);
    virtual void keyPressEvent(QKeyEvent *pEvent);
    virtual void wheelEvent(QWheelEvent *pEvent);

private:
    QFont mFont;

    QMenu *mContextMenu;

    bool mCanUndo;
    bool mCanRedo;

    bool mCanSelectAll;

    bool mOverwriteMode;

    QLabel *mCursorPositionWidget;
    QLabel *mEditingModeWidget;

    void updateColors();

Q_SIGNALS:
    void canUndo(const bool &pCanUndo);
    void canRedo(const bool &pCanRedo);

    void canSelectAll(const bool &pCanSelectAll);

private Q_SLOTS:
    void updateUi();

    void checkCanSelectAll();

    void cursorPositionChanged(const int &pLine, const int &pColumn);
};

//==============================================================================

}   // namespace QScintillaSupport
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
