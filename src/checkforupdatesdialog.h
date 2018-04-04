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
// Check for updates dialog
//==============================================================================

#pragma once

//==============================================================================

#include "guiutils.h"

//==============================================================================

#include <QString>

//==============================================================================

namespace Ui {
    class CheckForUpdatesDialog;
}   // namespace Ui

//==============================================================================

class QSettings;

//==============================================================================

namespace OpenCOR {

//==============================================================================

static const auto SettingsCheckForUpdatesDialog    = QStringLiteral("CheckForUpdatesDialog");
static const auto SettingsCheckForUpdatesAtStartup = QStringLiteral("CheckForUpdatesAtStartup");
static const auto SettingsIncludeSnapshots         = QStringLiteral("IncludeSnapshots");

//==============================================================================

class CheckForUpdatesEngine : public QObject
{
    Q_OBJECT

public:
    explicit CheckForUpdatesEngine(const QString &pApplicationDate);

    void check();

    QString status() const;

    QStringList newerVersions() const;

    bool hasNewerVersion() const;
    bool hasNewerOfficialVersion() const;

private:
    QString mApplicationDate;

    QString mStatus;

    QStringList mNewerVersions;
};

//==============================================================================

class CheckForUpdatesDialog : public Dialog
{
    Q_OBJECT

public:
    explicit CheckForUpdatesDialog(QSettings *pSettings,
                                   const QString &pApplicationDate,
                                   QWidget *pParent);
    explicit CheckForUpdatesDialog(QSettings *pSettings,
                                   CheckForUpdatesEngine *pEngine);
    ~CheckForUpdatesDialog() override;

private:
    Ui::CheckForUpdatesDialog *mGui;

    CheckForUpdatesEngine *mEngine;

    bool mInitialized;

    explicit CheckForUpdatesDialog(QSettings *pSettings,
                                   const QString &pApplicationDate,
                                   CheckForUpdatesEngine *pEngine,
                                   QWidget *pParent);

    void updateGui();

private slots:
    void onStatusLabelLinkActivated(const QString &pLink);
    void onRecheckButtonClicked();
    void onCheckForUpdatesAtStartupCheckBoxToggled(bool pChecked);
    void onIncludeSnapshotsCheckBoxToggled(bool pChecked);
    void onButtonBoxAccepted();
};

//==============================================================================

}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
