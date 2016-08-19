/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/member-reflection.h"

#include "hphp/util/embedded-data.h"
#include "hphp/util/logger.h"

#include <cstdio>
#include <dlfcn.h>
#include <string>
#include <unordered_map>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

std::unordered_map<std::string, const char*(*)(const void*, const void*)> const*
  g_member_reflection_vtable = nullptr;

#define X(name) \
  const char* nameof_member(const HPHP::name* base, const void* internal) { \
    if (!g_member_reflection_vtable) return nullptr; \
    auto it = g_member_reflection_vtable->find("HPHP::"#name); \
    if (it == std::end(*g_member_reflection_vtable)) return nullptr; \
    return it->second(base, internal); \
  }
HPHP_REFLECTABLES
#undef X

bool init_member_reflection() {
  embedded_data desc;
  if (!get_embedded_data("member_reflection", &desc)) {
    // We might not be embedding the shared object, depending on platform, so
    // don't cry too loudly if we don't find it.
    Logger::Verbose("init_member_reflection: Unable to find embedded data\n");
    return false;
  }

  char tmp_filename[] = "/tmp/hhvm_member_reflection_XXXXXX";
  auto const handle = dlopen_embedded_data(desc, tmp_filename);
  if (!handle) {
    Logger::Warning("init_member_reflection: "
                    "Failed to dlopen embedded data\n");
    return false;
  }

  g_member_reflection_vtable =
    reinterpret_cast<decltype(g_member_reflection_vtable)>(
      dlsym(handle, detail::kMemberReflectionTableName)
    );
  if (!g_member_reflection_vtable) {
    Logger::Warning("init_member_reflection: dlsym failed: %s\n", dlerror());
    return false;
  }

  return true;
}

#define X(name)

#undef X

///////////////////////////////////////////////////////////////////////////////

}
