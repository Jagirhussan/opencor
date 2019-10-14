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
// PythonQt support functions
//==============================================================================

#include <Qt>

//==============================================================================

#include "pythonbegin.h"

//==============================================================================

#include "pythonqtsupport.h"

//==============================================================================

namespace OpenCOR {
namespace PythonQtSupport {

//==============================================================================

void addInstanceDecorators(QObject *pQObject)
{
    PythonQt::self()->addInstanceDecorators(pQObject);
}

//==============================================================================

void addObject(PyObject *pObject, const QString &pName, QObject *pQObject)
{
    PythonQt::self()->addObject(pObject, pName, pQObject);
}

//==============================================================================

void evalFile(const QString &pFilename)
{
    PythonQt::self()->evalFile(PythonQt::self()->getMainModule(), pFilename);
}

//==============================================================================

QVariant evalScript(const QString &pScript)
{
    return PythonQt::self()->evalScript(PythonQt::self()->getMainModule(), pScript);
}

//==============================================================================

PythonQtObjectPtr importModule(const QString &pModule)
{
    return PythonQt::self()->importModule(pModule);
}

//==============================================================================

void registerClass(const QMetaObject *pMetaObject)
{
    PythonQt::self()->registerClass(pMetaObject);
}

//==============================================================================

PyObject *wrapQObject(QObject *pQObject)
{
    return PythonQt::priv()->wrapQObject(pQObject);
}

//==============================================================================

PythonQtInstanceWrapper *getInstanceWrapper(PyObject *self)
{
    if (self && PyObject_TypeCheck(self, &PythonQtInstanceWrapper_Type)) {
        return reinterpret_cast<PythonQtInstanceWrapper*>(self);
    }

    return nullptr;
}

//==============================================================================

}   // namespace PythonQtSupport
}   // namespace OpenCOR

//==============================================================================

#include "pythonend.h"

//==============================================================================
// End of file
//==============================================================================

