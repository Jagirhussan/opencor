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
// Web Browser window plugin
//==============================================================================

#include "corecliutils.h"
#include "coreguiutils.h"
#include "webbrowserwindowplugin.h"
#include "webbrowserwindowwindow.h"

//==============================================================================

#include <QMainWindow>
#include <QSettings>

//==============================================================================

#include "pythonbegin.h"
    #include "webbrowserwindowpythonwrapper.h"
#include "pythonend.h"

//==============================================================================

namespace OpenCOR {
namespace WebBrowserWindow {

//==============================================================================

PLUGININFO_FUNC WebBrowserWindowPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("a plugin to browse the Web."));
    descriptions.insert("fr", QString::fromUtf8("une extension pour naviguer sur le Web."));

    return new PluginInfo(PluginInfo::Category::Miscellaneous, true, false,
                          QStringList() << "Core" << "PythonQtSupport" << "WebViewerWidget",
                          descriptions,
                          QStringList() << "PythonConsoleWindow");
}

//==============================================================================
// I18n interface
//==============================================================================

void WebBrowserWindowPlugin::retranslateUi()
{
    // Retranslate our Web Browser window action

    retranslateAction(mWebBrowserWindowAction,
                      tr("Web Browser"),
                      tr("Show/hide the Web Browser window"));

    // Retranslate our Web Browser window

    mWebBrowserWindowWindow->retranslateUi();
}

//==============================================================================
// Plugin interface
//==============================================================================

bool WebBrowserWindowPlugin::definesPluginInterfaces()
{
    // We don't handle this interface...

    return false;
}

//==============================================================================

bool WebBrowserWindowPlugin::pluginInterfacesOk(const QString &pFileName,
                                                QObject *pInstance)
{
    Q_UNUSED(pFileName)
    Q_UNUSED(pInstance)

    // We don't handle this interface...

    return false;
}

//==============================================================================

void WebBrowserWindowPlugin::initializePlugin()
{
    // Create an action to show/hide our Web Browser window

    mWebBrowserWindowAction = Core::newAction(true, Core::mainWindow());

    // Create our Web Browser window

    mWebBrowserWindowWindow = new WebBrowserWindowWindow(Core::mainWindow());

    // Get the Web Browser window's widget and save it for our Python wrapper

    mWebBrowserWindowWidget = mWebBrowserWindowWindow->widget();

    instance()->mWebBrowserWindowWidget = mWebBrowserWindowWidget;
}

//==============================================================================

void WebBrowserWindowPlugin::finalizePlugin()
{
    // We don't handle this interface...
}

//==============================================================================

void WebBrowserWindowPlugin::pluginsInitialized(const Plugins &pLoadedPlugins)
{
    Q_UNUSED(pLoadedPlugins)

    // We don't handle this interface...
}

//==============================================================================

void WebBrowserWindowPlugin::loadSettings(QSettings &pSettings)
{
    // Retrieve our Web Browser window settings

    pSettings.beginGroup(mWebBrowserWindowWindow->objectName());
        mWebBrowserWindowWindow->loadSettings(pSettings);
    pSettings.endGroup();
}

//==============================================================================

void WebBrowserWindowPlugin::saveSettings(QSettings &pSettings) const
{
    // Keep track of our Web Browser window settings

    pSettings.beginGroup(mWebBrowserWindowWindow->objectName());
        mWebBrowserWindowWindow->saveSettings(pSettings);
    pSettings.endGroup();
}

//==============================================================================

void WebBrowserWindowPlugin::handleUrl(const QUrl &pUrl)
{
    Q_UNUSED(pUrl)

    // We don't handle this interface...
}

//==============================================================================
// Window interface
//==============================================================================

Qt::DockWidgetArea WebBrowserWindowPlugin::windowDefaultDockArea() const
{
    // Return our default dock area

    return Qt::RightDockWidgetArea;
}

//==============================================================================

QAction * WebBrowserWindowPlugin::windowAction() const
{
    // Return our Web Browser window action

    return mWebBrowserWindowAction;
}

//==============================================================================

QDockWidget * WebBrowserWindowPlugin::windowWidget() const
{
    // Return our Web Browser window widget

    return mWebBrowserWindowWindow;
}

//==============================================================================
// Python interface
//==============================================================================

void WebBrowserWindowPlugin::registerPythonClasses(PyObject *pModule)
{
    // Register our Python classes

    mWebBrowserWindowPythonWrapper = new WebBrowserWindowPythonWrapper(pModule, this);
}

//==============================================================================
// Plugin specific
//==============================================================================

WebBrowserWindowPlugin * WebBrowserWindowPlugin::instance(void)
{
    // Return the 'global' instance of our plugin

    static WebBrowserWindowPlugin pluginInstance;

    return static_cast<WebBrowserWindowPlugin *>(Core::globalInstance("OpenCOR::WebBrowserWindow::WebBrowserWindowPlugin",
                                                 &pluginInstance));
}


//==============================================================================

WebBrowserWindowWidget * WebBrowserWindowPlugin::browserWidget(void) const
{
    // Return our Web Browser widget

    return mWebBrowserWindowWidget;
}

//==============================================================================

} // namespace WebBrowserWindow
} // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
