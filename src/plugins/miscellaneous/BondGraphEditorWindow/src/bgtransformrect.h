/*******************************************************************************

Copyright (C) The University of Auckland

OpenCOR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenCOR is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://gnu.org/licenses>.

*******************************************************************************/
#pragma once

#include <QCursor>
#include <QGraphicsItem>
#include <QList>
#include <QPainter>
#include <QPointF>
#include <QRectF>

#include "sceneeditcontrollerinterface.h"

namespace OpenCOR {
namespace BondGraphEditorWindow {

class BGEditorScene;
class SceneItem;
class BGElement;

class BGTransformRect : public QObject, public SceneEditControllerInterface
{
    Q_OBJECT

public:
    BGTransformRect();
    ~BGTransformRect() override;

    // move-only mode
    void setMoveOnly(bool on);

    // ISceneEditController
    void onActivated(BGEditorScene &scene) override;
    void onDeactivated(BGEditorScene & /*scene*/) override
    {
    }
    void onSelectionChanged(BGEditorScene & /*scene*/) override;
    void onSceneChanged(BGEditorScene &scene) override;
    void onDragItem(BGEditorScene & /*scene*/,
                    QGraphicsSceneMouseEvent * /*mouseEvent*/,
                    QGraphicsItem * /*dragItem*/) override;
    void draw(BGEditorScene &scene, QPainter *painter, const QRectF &rect) override;
    bool onMousePressed(BGEditorScene &scene,
                        QGraphicsSceneMouseEvent *mouseEvent) override;
    bool onMouseMove(BGEditorScene &scene,
                     QGraphicsSceneMouseEvent *mouseEvent) override;
    bool onMouseReleased(BGEditorScene &scene,
                         QGraphicsSceneMouseEvent *mouseEvent) override;

private:
    void doSetupItems(BGEditorScene &scene);
    void doReset();
    void doTransformBy(BGEditorScene &scene, QRectF oldRect, QRectF newRect);

    struct ControlPoint {
        QPointF pos;
        QCursor cursor;
        QRectF sceneRect;
    };

    ControlPoint m_points[8];

    int m_dragPoint = -1;
    QPointF m_dragPos;
    QRectF m_dragRect;
    QPointF m_lastPos;
    QRectF m_lastRect;

    bool m_moveOnlyMode = false;

    // transform lists
    QList<BGElement *> m_nodesTransform, m_nodesMove;
    QList<SceneItem *> m_others;
};

} // namespace BondGraphEditorWindow
} // namespace OpenCOR