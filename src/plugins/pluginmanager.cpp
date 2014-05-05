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
// Plugin manager
//==============================================================================

#include "cliinterface.h"
#include "plugin.h"
#include "pluginmanager.h"
#include "viewinterface.h"
#include "windowinterface.h"

//==============================================================================

#include <QCoreApplication>
#include <QDir>

//==============================================================================

namespace OpenCOR {

//==============================================================================

PluginManager::PluginManager(QCoreApplication *pApp, const bool &pGuiMode) :
    mPlugins(Plugins()),
    mLoadedPlugins(Plugins()),
    mCorePlugin(0)
{
    mPluginsDir =  QDir(pApp->applicationDirPath()).canonicalPath()
                  +QDir::separator()+QString("..")
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
                  +QDir::separator()+"plugins"
#elif defined(Q_OS_MAC)
                  +QDir::separator()+"PlugIns"
#else
    #error Unsupported platform
#endif
                  +QDir::separator()+pApp->applicationName();

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    // The plugins directory should be correct, but in case we try to run
    // OpenCOR on Windows or Linux AND from within Qt Creator, then the binary
    // will be running from [OpenCOR]/build/OpenCOR[.exe] rather than
    // [OpenCOR]/build/bin/OpenCOR[.exe] as it should if we were to mimic the
    // case where OpenCOR has been deployed. Then, because the plugins are in
    // [OpenCOR]/build/plugins/OpenCOR, we must skip the "../" bit. So, yes,
    // it's not neat, but... is there another solution?...

    if (!QDir(mPluginsDir).exists())
        mPluginsDir =  QDir(pApp->applicationDirPath()).canonicalPath()
                      +QDir::separator()+"plugins"
                      +QDir::separator()+pApp->applicationName();
#endif

    mPluginsDir = QDir::toNativeSeparators(QDir(mPluginsDir).canonicalPath());

    // Retrieve the list of plugins available for loading

    QFileInfoList fileInfoList = QDir(mPluginsDir).entryInfoList(QStringList("*"+PluginExtension),
                                                                 QDir::Files);

    QStringList fileNames = QStringList();

    foreach (const QFileInfo &file, fileInfoList)
        fileNames << QDir::toNativeSeparators(file.canonicalFilePath());

    // Determine which plugins, if any, are needed by others and which, if any,
    // are selectable

    QMap<QString, PluginInfo *> pluginsInfo = QMap<QString, PluginInfo *>();
    QMap<QString, QString> pluginsError = QMap<QString, QString>();

    QStringList selectablePlugins = QStringList();

    foreach (const QString &fileName, fileNames) {
        QString pluginError;
        PluginInfo *pluginInfo = Plugin::info(fileName, pluginError);
        // Note: if there is some plugin information, then it will get owned by
        //       the plugin itself. So, it's the plugin's responsibility to
        //       delete it (see Plugin::~Plugin())...
        QString pluginName = Plugin::name(fileName);

        pluginsInfo.insert(pluginName, pluginInfo);
        pluginsError.insert(pluginName, pluginError);

        if (pluginInfo) {
            // Keep track of the plugin's full dependencies

            QStringList pluginFullDependencies = Plugin::fullDependencies(mPluginsDir, pluginName);

            pluginInfo->setFullDependencies(pluginFullDependencies);

            // Keep track of the plugin itself, should it be selectable

            if (pluginInfo->isSelectable())
                selectablePlugins << pluginName;
        }
    }

    // Determine which plugins, if any, are needed or wanted
    // Note: unselectable plugins (e.g. the QScintilla plugin) don't get loaded
    //       by default, but the situation is obviously different if such a
    //       plugin is needed by another plugin (e.g. the RawView plugin
    //       indirectly requires the QScintilla plugin to be loaded), in which
    //       case the unselectable plugin must be loaded...

    QStringList neededPlugins = QStringList();
    QStringList wantedPlugins = QStringList();

    foreach (const QString &selectablePlugin, selectablePlugins)
        if (   ( pGuiMode && Plugin::load(selectablePlugin))
            || (!pGuiMode && pluginsInfo.value(selectablePlugin)->hasCliSupport())) {
            // We are in GUI mode and the user wants to load the plugin, or we
            // are not in GUI mode (i.e. CLI mode) and the plugin has CLI
            // support, so retrieve and keep track of the plugin's dependencies

            neededPlugins << pluginsInfo.value(selectablePlugin)->fullDependencies();

            // Also keep track of the plugin itself

            wantedPlugins << selectablePlugin;
        }

    // Remove possible duplicates in our list of needed plugins

    neededPlugins.removeDuplicates();

    // We now have all our needed and wanted plugins with our needed plugins
    // nicely sorted based on their dependencies with one another. So, retrieve
    // their file name

    QStringList plugins = neededPlugins+wantedPlugins;
    QStringList pluginFileNames = QStringList();

    plugins.removeDuplicates();
    // Note: we shouldn't have to remove duplicates, but better be safe than
    //       sorry (indeed, a selectable plugin may be (wrongly) needed by
    //       another plugin)...

    foreach (const QString &plugin, plugins)
        pluginFileNames << Plugin::fileName(mPluginsDir, plugin);

    // If we are in GUI mode, then we want to know about all the plugins,
    // including the ones that are not to be loaded (so that we can refer to
    // them, in the plugins window, as either not wanted or not needed)

    if (pGuiMode) {
        pluginFileNames << fileNames;

        pluginFileNames.removeDuplicates();
    }

    // Deal with all the plugins we need and want

    foreach (const QString &pluginFileName, pluginFileNames) {
        QString pluginName = Plugin::name(pluginFileName);

        Plugin *plugin = new Plugin(pluginFileName, pluginsInfo.value(pluginName),
                                    pluginsError.value(pluginName),
                                    plugins.contains(pluginName), this);

        // Keep track of the Core plugin, if it's the one we are dealing with,
        // as well as keep track of the plugin in general

        if (!pluginName.compare(CorePluginName))
            mCorePlugin = plugin;

        mPlugins << plugin;

        if (plugin->status() == Plugin::Loaded)
            mLoadedPlugins << plugin;
    }
}

//==============================================================================

PluginManager::~PluginManager()
{
    // Delete all of the plugins

    foreach (Plugin *plugin, mPlugins)
        delete plugin;
}

//==============================================================================

Plugins PluginManager::plugins() const
{
    // Return a list of all our plugins, whether loaded

    return mPlugins;
}

//==============================================================================

Plugins PluginManager::loadedPlugins() const
{
    // Return a list of our loaded plugins

    return mLoadedPlugins;
}

//==============================================================================

QString PluginManager::pluginsDir() const
{
    // Return the plugins directory

    return mPluginsDir;
}

//==============================================================================

Plugin * PluginManager::plugin(const QString &pName) const
{
    // Return the plugin which name is the one we have been passed

    foreach (Plugin *plugin, mPlugins)
        if (!pName.compare(plugin->name()))
            // This is the plugin we are after, so...

            return plugin;

    // The plugin we are after wasn't found, so...

    return 0;
}

//==============================================================================

Plugin * PluginManager::corePlugin() const
{
    // Return our Core plugin

    return mCorePlugin;
}

//==============================================================================

}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
