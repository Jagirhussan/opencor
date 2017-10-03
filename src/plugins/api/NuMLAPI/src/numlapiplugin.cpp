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
// NuML API plugin
//==============================================================================

#include "numlapiplugin.h"

//==============================================================================

namespace OpenCOR {
namespace NuMLAPI {

//==============================================================================

PLUGININFO_FUNC NuMLAPIPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("a plugin to access <a href=\"https://github.com/NuML/NuML/\">libNuML</a>."));
    descriptions.insert("fr", QString::fromUtf8("une extension pour accéder <a href=\"https://github.com/NuML/NuML/\">libNuML</a>."));

    return new PluginInfo(PluginInfo::Api, false, false,
                          QStringList(),
                          descriptions);
}

//==============================================================================

}   // namespace NuMLAPI
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
