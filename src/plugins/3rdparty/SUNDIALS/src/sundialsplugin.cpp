//==============================================================================
// SUNDIALS plugin
//==============================================================================

#include "sundialsplugin.h"

//==============================================================================

namespace OpenCOR {
namespace SUNDIALS {

//==============================================================================

PLUGININFO_FUNC SUNDIALSPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("a plugin to access CVODE, IDA and KINSOL from the <a href=\"http://computation.llnl.gov/casc/sundials/description/description.html\">SUNDIALS</a> library."));
    descriptions.insert("fr", QString::fromUtf8("une extension pour accéder à CVODE, IDA et KINSOL de la librairie <a href=\"http://computation.llnl.gov/casc/sundials/description/description.html\">SUNDIALS</a>."));

    return new PluginInfo(PluginInfo::ThirdParty,
                          QStringList(),
                          descriptions);
}

//==============================================================================

}   // namespace SUNDIALS
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
