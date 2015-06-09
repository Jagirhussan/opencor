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
// Web view widget
//==============================================================================

#ifndef WEBVIEWWIDGET_H
#define WEBVIEWWIDGET_H

//==============================================================================

#include "coreglobal.h"

//==============================================================================

#include <QString>
#include <QWebView>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

class CORE_EXPORT WebViewWidget : public QWebView
{
    Q_OBJECT

public:
    explicit WebViewWidget(QWidget *pParent);

    QWebElement retrieveLinkInformation(QString &pLink, QString &pTextContent);

protected:
    virtual bool event(QEvent *pEvent);

protected:
    bool mResettingCursor;
};

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================