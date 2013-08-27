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
// FileBrowser plugin
//==============================================================================

#include "filebrowserplugin.h"
#include "filebrowserwindow.h"

//==============================================================================

#include <QMainWindow>

//==============================================================================

namespace OpenCOR {
namespace FileBrowser {

//==============================================================================

PLUGININFO_FUNC FileBrowserPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("a plugin to access your local files."));
    descriptions.insert("fr", QString::fromUtf8("une extension pour accéder à vos fichiers locaux."));

    return new PluginInfo(PluginInfo::InterfaceVersion001,
                          PluginInfo::Organisation,
                          false,
                          true,
                          QStringList() << "Core",
                          descriptions);
}

//==============================================================================

void FileBrowserPlugin::initialize()
{
    // Create an action to show/hide our file browser window

    mFileBrowserAction = newAction(mMainWindow, true);

    // Create our file browser window

    mFileBrowserWindow = new FileBrowserWindow(mMainWindow);

    // Set our settings

    mGuiSettings->addWindow(Qt::LeftDockWidgetArea, mFileBrowserWindow,
                            GuiWindowSettings::Organisation,
                            mFileBrowserAction);
}

//==============================================================================

void FileBrowserPlugin::loadSettings(QSettings *pSettings)
{
    // Retrieve our file browser window settings

    loadWindowSettings(pSettings, mFileBrowserWindow);
}

//==============================================================================

void FileBrowserPlugin::saveSettings(QSettings *pSettings) const
{
    // Keep track of our file browser window settings

    saveWindowSettings(pSettings, mFileBrowserWindow);
}

//==============================================================================

void FileBrowserPlugin::retranslateUi()
{
    // Retranslate our file browser action

    retranslateAction(mFileBrowserAction, tr("File Browser"),
                      tr("Show/hide the File Browser window"));

    // Retranslate our file browser window

    mFileBrowserWindow->retranslateUi();
}

//==============================================================================

}   // namespace FileBrowser
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
