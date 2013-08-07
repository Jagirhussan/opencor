//==============================================================================
// Viewer plugin
//==============================================================================

#include "viewerplugin.h"

//==============================================================================

namespace OpenCOR {
namespace Viewer {

//==============================================================================

PLUGININFO_FUNC ViewerPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("a plugin to graphically visualise various modelling concepts (e.g. mathematical equations)."));
    descriptions.insert("fr", QString::fromUtf8("une extension pour visualiser graphiquement différents concepts de modélisation (par exemple des équations mathématiques)."));

    return new PluginInfo(PluginInfo::InterfaceVersion001,
                          PluginInfo::Miscellaneous,
                          false,
                          false,
                          QStringList() << "Qwt",
                          descriptions);
}

//==============================================================================

}   // namespace Viewer
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
