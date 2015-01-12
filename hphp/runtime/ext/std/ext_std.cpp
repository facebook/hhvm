#include "hphp/runtime/ext/std/ext_std.h"
#include "hphp/runtime/version.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

const StaticString
  s_HHVM_VERSION_ID("HHVM_VERSION_ID"),
  s_HHVM_VERSION("HHVM_VERSION"),
  s_HPHP_VERSION("HPHP_VERSION"),
  s_HHVM_VERSION_string(HHVM_VERSION);

void StandardExtension::initStandard() {
  // define('HHVM_VERSION_ID', XXYYZZ);
  Native::registerConstant<KindOfInt64>(
    s_HHVM_VERSION_ID.get(),
    ((HHVM_VERSION_MAJOR * 10000) +
     (HHVM_VERSION_MINOR *   100) +
      HHVM_VERSION_PATCH)
  );

  // define('HPHP_VERSION', 'XX.YY.XX')
  // define('HHVM_VERSION', 'XX.YY.XX')
  Native::registerConstant<KindOfStaticString>(
    s_HHVM_VERSION.get(),
    s_HHVM_VERSION_string.get()
  );
  Native::registerConstant<KindOfStaticString>(
    s_HPHP_VERSION.get(),
    s_HHVM_VERSION_string.get()
  );
}

StandardExtension s_standard_extension;

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
