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
// CoreCellMLEditing plugin
//==============================================================================

#include "cellmleditinginterface.h"
#include "corecellmleditingplugin.h"
#include "filemanager.h"
#include "guiutils.h"

//==============================================================================

#include <QAction>
#include <QMainWindow>

//==============================================================================

namespace OpenCOR {
namespace CoreCellMLEditing {

//==============================================================================

PLUGININFO_FUNC CoreCellMLEditingPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("the core CellML editing plugin."));
    descriptions.insert("fr", QString::fromUtf8("l'extension d'édition CellML de base."));

    return new PluginInfo(PluginInfo::Editing, false,
                          QStringList() << "CoreEditing" << "CellMLSupport" << "Viewer",
                          descriptions);
}

//==============================================================================
// Core interface
//==============================================================================

void CoreCellMLEditingPlugin::initialize()
{
    // Create our different File|New actions

    mFileNewCellml1_0FileAction = new QAction(mMainWindow);
    mFileNewCellml1_1FileAction = new QAction(mMainWindow);

    // Set our settings

    mGuiSettings->addMenuAction(GuiMenuActionSettings::FileNew, mFileNewCellml1_0FileAction);
    mGuiSettings->addMenuAction(GuiMenuActionSettings::FileNew, mFileNewCellml1_1FileAction);

    // Some connections to handle our different editing actions

    connect(mFileNewCellml1_0FileAction, SIGNAL(triggered()),
            this, SLOT(newCellml1_0File()));
    connect(mFileNewCellml1_1FileAction, SIGNAL(triggered()),
            this, SLOT(newCellml1_1File()));
}

//==============================================================================

void CoreCellMLEditingPlugin::finalize()
{
    // We don't handle this interface...
}

//==============================================================================

void CoreCellMLEditingPlugin::initialized(const Plugins &pLoadedPlugins)
{
    Q_UNUSED(pLoadedPlugins);

    // We don't handle this interface...
}

//==============================================================================

void CoreCellMLEditingPlugin::loadSettings(QSettings *pSettings)
{
    Q_UNUSED(pSettings);

    // We don't handle this interface...
}

//==============================================================================

void CoreCellMLEditingPlugin::saveSettings(QSettings *pSettings) const
{
    Q_UNUSED(pSettings);

    // We don't handle this interface...
}

//==============================================================================

void CoreCellMLEditingPlugin::settingsLoaded(const Plugins &pLoadedPlugins)
{
    Q_UNUSED(pLoadedPlugins);

    // We don't handle this interface...
}

//==============================================================================

void CoreCellMLEditingPlugin::handleArguments(const QStringList &pArguments)
{
    Q_UNUSED(pArguments);

    // We don't handle this interface...
}

//==============================================================================

void CoreCellMLEditingPlugin::handleAction(const QUrl &pUrl)
{
    Q_UNUSED(pUrl);

    // We don't handle this interface...
}

//==============================================================================
// GUI interface
//==============================================================================

void CoreCellMLEditingPlugin::changeEvent(QEvent *pEvent)
{
    Q_UNUSED(pEvent);

    // We don't handle this interface...
}

//==============================================================================

void CoreCellMLEditingPlugin::updateGui(Plugin *pViewPlugin,
                                        const QString &pFileName)
{
    Q_UNUSED(pFileName);

    // Show/enable or hide/disable various actions, depending on whether the
    // view plugin handles the CellML editing interface

    CellmlEditingInterface *cellmlEditingInterface = pViewPlugin?qobject_cast<CellmlEditingInterface *>(pViewPlugin->instance()):0;

    Core::showEnableAction(mFileNewCellml1_0FileAction, cellmlEditingInterface);
    Core::showEnableAction(mFileNewCellml1_1FileAction, cellmlEditingInterface);
}

//==============================================================================

void CoreCellMLEditingPlugin::initializeView()
{
    // We don't handle this interface...
}

//==============================================================================

void CoreCellMLEditingPlugin::finalizeView()
{
    // We don't handle this interface...
}

//==============================================================================

bool CoreCellMLEditingPlugin::hasViewWidget(const QString &pFileName)
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...

    return false;
}

//==============================================================================

QWidget * CoreCellMLEditingPlugin::viewWidget(const QString &pFileName,
                                              const bool &pCreate)
{
    Q_UNUSED(pFileName);
    Q_UNUSED(pCreate);

    // We don't handle this interface...

    return 0;
}

//==============================================================================

void CoreCellMLEditingPlugin::removeViewWidget(const QString &pFileName)
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...
}

//==============================================================================

QString CoreCellMLEditingPlugin::viewName() const
{
    // We don't handle this interface...

    return QString();
}

//==============================================================================

QIcon CoreCellMLEditingPlugin::fileTabIcon(const QString &pFileName) const
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...

    return QIcon();
}

//==============================================================================

bool CoreCellMLEditingPlugin::saveFile(const QString &pOldFileName,
                                       const QString &pNewFileName)
{
    Q_UNUSED(pOldFileName);
    Q_UNUSED(pNewFileName);

    // We don't handle this interface...

    return false;
}

//==============================================================================

void CoreCellMLEditingPlugin::fileOpened(const QString &pFileName)
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...
}

//==============================================================================

void CoreCellMLEditingPlugin::filePermissionsChanged(const QString &pFileName)
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...
}

//==============================================================================

void CoreCellMLEditingPlugin::fileModified(const QString &pFileName,
                                           const bool &pModified)
{
    Q_UNUSED(pFileName);
    Q_UNUSED(pModified);

    // We don't handle this interface...
}

//==============================================================================

void CoreCellMLEditingPlugin::fileReloaded(const QString &pFileName)
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...
}

//==============================================================================

void CoreCellMLEditingPlugin::fileRenamed(const QString &pOldFileName,
                                          const QString &pNewFileName)
{
    Q_UNUSED(pOldFileName);
    Q_UNUSED(pNewFileName);

    // We don't handle this interface...
}

//==============================================================================

void CoreCellMLEditingPlugin::fileClosed(const QString &pFileName)
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...
}

//==============================================================================

bool CoreCellMLEditingPlugin::canClose()
{
    // We don't handle this interface...

    return true;
}

//==============================================================================
// I18n interface
//==============================================================================

void CoreCellMLEditingPlugin::retranslateUi()
{
    // Retranslate our different File|New actions

    retranslateAction(mFileNewCellml1_0FileAction, tr("CellML 1.0 File"),
                      tr("Create a new CellML 1.0 file"));
    retranslateAction(mFileNewCellml1_1FileAction, tr("CellML 1.1 File"),
                      tr("Create a new CellML 1.1 file"));
}

//==============================================================================
// Plugin specific
//==============================================================================

void CoreCellMLEditingPlugin::newCellmlFile(const CellMLSupport::CellmlFile::Version &pVersion)
{
    // Determine some version-specific information

    QString version = QString();
    QString modelName = QString();

    switch (pVersion) {
    case CellMLSupport::CellmlFile::Cellml_1_1:
        version = "1.1";

        modelName = "new_cellml_1_1_model";

        break;
    default:   // CellMLSupport::CellmlFile::Cellml_1_0
        version = "1.0";

        modelName = "new_cellml_1_0_model";
    }

    // Ask our file manager to create a new file

    QString fileContents = "<?xml version=\"1.0\"?>\n"
                           "<model xmlns=\"http://www.cellml.org/cellml/%1#\" name=\"%2\">\n"
                           "    <!-- Your code goes here -->\n"
                           "</model>\n";
    Core::FileManager *fileManagerInstance = Core::FileManager::instance();
#ifdef QT_DEBUG
    Core::FileManager::Status createStatus =
#endif
    fileManagerInstance->create(QString(), fileContents.arg(version, modelName));

#ifdef QT_DEBUG
    // Make sure that the file has indeed been created

    if (createStatus != Core::FileManager::Created)
        qFatal("FATAL ERROR | %s:%d: the file was not created", __FILE__, __LINE__);
#endif
}

//==============================================================================

void CoreCellMLEditingPlugin::newCellml1_0File()
{
    // Create a new CellML 1.0 file

    newCellmlFile(CellMLSupport::CellmlFile::Cellml_1_0);
}

//==============================================================================

void CoreCellMLEditingPlugin::newCellml1_1File()
{
    // Create a new CellML 1.1 file

    newCellmlFile(CellMLSupport::CellmlFile::Cellml_1_1);
}

//==============================================================================

}   // namespace CoreCellMLEditing
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
