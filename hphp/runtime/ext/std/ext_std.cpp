#include "hphp/runtime/ext/std/ext_std.h"

#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/version.h"
#include "hphp/util/build-info.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

void StandardExtension::initStandard() {
  // define('HHVM_VERSION_ID', XXYYZZ);
  HHVM_RC_INT(
    HHVM_VERSION_ID,
    ((HHVM_VERSION_MAJOR * 10000) +
     (HHVM_VERSION_MINOR *   100) +
      HHVM_VERSION_PATCH)
  );

  // define('HPHP_VERSION', 'XX.YY.XX')
  // define('HHVM_VERSION', 'XX.YY.XX')
  HHVM_RC_STR_SAME(HHVM_VERSION);
  HHVM_RC_STR(HPHP_VERSION, HHVM_VERSION);
  HHVM_RC_BOOL(HHVM_DEBUG, debug);
  HHVM_RC_STR(HHVM_COMPILER_ID, compilerId());
  HHVM_RC_STR(HHVM_REPO_SCHEMA, repoSchemaId());
}

StandardExtension s_standard_extension;

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
