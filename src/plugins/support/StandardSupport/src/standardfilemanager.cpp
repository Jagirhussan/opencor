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
// Standard file manager
//==============================================================================

#include "corecliutils.h"
#include "filemanager.h"
#include "standardfilemanager.h"
#include "standardsupportplugin.h"

//==============================================================================

namespace OpenCOR {
namespace StandardSupport {

//==============================================================================

StandardFileManager::StandardFileManager() :
    mFiles(Files())
{
    // Create some connections to keep track of some events related to our
    // 'global' file manager

    Core::FileManager *fileManagerInstance = Core::FileManager::instance();

    connect(fileManagerInstance, SIGNAL(fileManaged(const QString &)),
            this, SLOT(manage(const QString &)));
    connect(fileManagerInstance, SIGNAL(fileUnmanaged(const QString &)),
            this, SLOT(unmanage(const QString &)));

    connect(fileManagerInstance, SIGNAL(fileReloaded(const QString &)),
            this, SLOT(reload(const QString &)));
    connect(fileManagerInstance, SIGNAL(fileRenamed(const QString &, const QString &)),
            this, SLOT(rename(const QString &, const QString &)));

    connect(fileManagerInstance, SIGNAL(fileSaved(const QString &)),
            this, SLOT(save(const QString &)));
}

//==============================================================================

StandardFileManager::~StandardFileManager()
{
}

//==============================================================================

bool StandardFileManager::isFile(const QString &pFileName)
{
    // If the given file is already managed, then we consider that it's of the
    // right type (e.g. CellML file), even though it may not be of the right
    // type anymore after having been edited and saved, but in this case it's
    // good to keep considering the file as of the right type, so that the user
    // can continue editing it without any problem, for example

    QString nativeFileName = Core::nativeCanonicalFileName(pFileName);

    if (file(nativeFileName))
        return true;

    // The given file is not managed, so consider it of the right type if it is
    // an empty file (after having been trimmed) or it can be loaded

    QByteArray fileContents;

    if (Core::readFileContentsFromFile(nativeFileName, fileContents)) {
        if (fileContents.trimmed().isEmpty())
            return true;

        return canLoad(nativeFileName);
    } else {
        return false;
    }
}

//==============================================================================

QObject * StandardFileManager::file(const QString &pFileName)
{
    // Return the File object, if any, associated with the given file

    return mFiles.value(Core::nativeCanonicalFileName(pFileName));
}

//==============================================================================

void StandardFileManager::manage(const QString &pFileName)
{
    QString nativeFileName = Core::nativeCanonicalFileName(pFileName);

    if (!file(nativeFileName) && isFile(nativeFileName)) {
        // We are dealing with a file, which is not already managed, so we can
        // add it to our list of managed files

        mFiles.insert(nativeFileName, create(nativeFileName));
    }
}

//==============================================================================

void StandardFileManager::unmanage(const QString &pFileName)
{
    QObject *crtFile = file(pFileName);

    if (crtFile) {
        // We are dealing with a file, so we can remove it from our list of
        // managed files after having deleted it

        delete crtFile;

        mFiles.remove(Core::nativeCanonicalFileName(pFileName));
    }
}

//==============================================================================

void StandardFileManager::save(const QString &pFileName)
{
    // The file has been (modified and) saved, so we need to reload it
Q_UNUSED(pFileName);

//---ISSUE1491---    reload(pFileName);
}

//==============================================================================

void StandardFileManager::reload(const QString &pFileName)
{
    // The file is to be reloaded (either because it has been changed or because
    // one or several of its dependencies has changed), so reload it
    // Note: to reload a file here ensures that our different standard-based
    //       views won't each do it, thus saving time and ensuring that a
    //       standard-based view doesn't forget to do it...

    QObject *crtFile = file(pFileName);

    if (crtFile) {
        // The file is managed, but should it still be (i.e. can it still be
        // considered as being a file)?

        if (isFile(pFileName))
            static_cast<StandardFile *>(crtFile)->reload();
        else
            unmanage(pFileName);
    } else {
        // The file is not managed, which means that previously it wasn't
        // considered as being a file, but things may be different now, so try
        // to remanage it and load it, if possible

        manage(pFileName);

        crtFile = file(pFileName);

        if (crtFile)
            static_cast<StandardFile *>(crtFile)->load();
    }
}

//==============================================================================

void StandardFileManager::rename(const QString &pOldFileName,
                                 const QString &pNewFileName)
{
    // The file has been renamed, so we need to update our files mapping, if
    // needed

    QObject *crtFile = file(pOldFileName);

    if (!crtFile)
        return;

    QString newNativeFileName = Core::nativeCanonicalFileName(pNewFileName);

    mFiles.insert(newNativeFileName, crtFile);
    mFiles.remove(Core::nativeCanonicalFileName(pOldFileName));

    // We also need to ensure that our file object has its file name updated

    static_cast<StandardFile *>(crtFile)->setFileName(newNativeFileName);
}

//==============================================================================

}   // namespace StandardSupport
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
