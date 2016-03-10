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
// GUI application
//==============================================================================

#ifndef GUIAPPLICATION_H
#define GUIAPPLICATION_H

//==============================================================================

#include <QtSingleApplication>

//==============================================================================

namespace OpenCOR {

//==============================================================================

class GuiApplication : public QtSingleApplication
{
    Q_OBJECT

public:
    GuiApplication(const QString &pId, int &pArgC, char **pArgV);

    bool hasFileNamesOrOpencorUrls() const;
    QString firstFileNameOrOpencorUrl();

    void updateCanEmitFileOpenRequestSignal();

protected:
    virtual bool event(QEvent *pEvent);

private:
    bool mCanEmitFileOpenRequestSignal;

    QStringList mFileNamesOrOpencorUrls;

Q_SIGNALS:
    void fileOpenRequest(const QString &file);
};

//==============================================================================

}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================