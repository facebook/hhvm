
#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <sys/system_globals.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

SystemGlobals::SystemGlobals() {
  memset(&stgv_bool, 0, sizeof(stgv_bool));
  memset(&stgv_int, 0, sizeof(stgv_int));
  memset(&stgv_int64, 0, sizeof(stgv_int64));
  memset(&stgv_double, 0, sizeof(stgv_double));
  memset(&stgv_RedeclaredCallInfoConstPtr, 0, sizeof(stgv_RedeclaredCallInfoConstPtr));

  // Redeclared Classes
}

void SystemGlobals::initialize() {
  Globals::initialize();
  Globals *globals = get_globals();
  pm_php$globals$symbols_php(false, globals, globals);
}

///////////////////////////////////////////////////////////////////////////////
}
