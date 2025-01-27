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

#include <QObject>
class QTreeWidgetItem;

namespace OpenCOR {
namespace BondGraphEditorWindow {

class PageEventFilter : public QObject {
  Q_OBJECT
public:
  PageEventFilter(QObject *parent, QTreeWidgetItem *item);

protected:
  bool eventFilter(QObject *obj, QEvent *event);

Q_SIGNALS:
  void itemClicked(QTreeWidgetItem *item, int column);

private:
  QTreeWidgetItem *mItem;
};

class PageResizeFilter : public QObject {
  Q_OBJECT
public:
  PageResizeFilter(QObject *parent, QTreeWidgetItem *item);

protected:
  bool eventFilter(QObject *obj, QEvent *event);

private:
  QTreeWidgetItem *mItem;
};

} // namespace BondGraphEditorWindow
} // namespace OpenCOR