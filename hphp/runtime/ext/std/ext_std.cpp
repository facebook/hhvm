#include "hphp/runtime/ext/std/ext_std.h"

#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/version.h"
#include "hphp/util/repo-schema.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

const StaticString
  s_HHVM_VERSION_ID("HHVM_VERSION_ID"),
  s_HHVM_VERSION("HHVM_VERSION"),
  s_HPHP_VERSION("HPHP_VERSION"),
  s_HHVM_VERSION_string(HHVM_VERSION),
  s_HHVM_DEBUG("HHVM_DEBUG"),
  s_HHVM_COMPILER_ID("HHVM_COMPILER_ID"),
  s_HHVM_COMPILER_ID_string(kCompilerId),
  s_HHVM_REPO_SCHEMA("HHVM_REPO_SCHEMA"),
  s_HHVM_REPO_SCHEMA_string(kRepoSchemaId);

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
  Native::registerConstant<KindOfPersistentString>(
    s_HHVM_VERSION.get(),
    s_HHVM_VERSION_string.get()
  );
  Native::registerConstant<KindOfPersistentString>(
    s_HPHP_VERSION.get(),
    s_HHVM_VERSION_string.get()
  );

  Native::registerConstant<KindOfBoolean>(
    s_HHVM_DEBUG.get(),
    debug
  );

  Native::registerConstant<KindOfPersistentString>(
    s_HHVM_COMPILER_ID.get(),
    s_HHVM_COMPILER_ID_string.get()
  );

  Native::registerConstant<KindOfPersistentString>(
    s_HHVM_REPO_SCHEMA.get(),
    s_HHVM_REPO_SCHEMA_string.get()
  );
}

StandardExtension s_standard_extension;

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
