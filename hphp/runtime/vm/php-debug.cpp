/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <stdlib.h>

#include <set>
#include <string>
#include <vector>

#include "hphp/util/debug.h"
#include "hphp/util/util.h"
#include "hphp/util/trace.h"

#ifndef DEBUG
/*
 * Forcefully always compile this unit with debug enabled, to permit mixing
 * and matching of optimized and unoptimized binaries.
 */
#define DEBUG
#endif

using std::string;
using std::vector;

#include "hphp/runtime/vm/php-debug.h"

namespace HPHP {

struct PhpDebugger {
  std::set<string> enabledFunctions;

  PhpDebugger() {
    const char *env = getenv("PHPBREAKPOINTS");
    if (env) {
      vector<string> names;
      Util::split(',', env, names, true /*ignoreEmpty*/);
      for (size_t i = 0; i < names.size(); ++i) {
        enabledFunctions.insert(names[i]);
      }
    }
  }

  bool isBpFunction(const char* nm) const {
    std::set<string>::const_iterator i = enabledFunctions.find(string(nm));
    return i != enabledFunctions.end();
  }
};

static PhpDebugger dbg;
bool phpBreakpointEnabled(const char* sourceName) {
  return dbg.isBpFunction(sourceName);
}

}

/*
 * Add or remove functions from the debug-set from gdb. Not exposed in
 * header files.
 *
 * Usage:
 *   (gdb) call phpbreak("idx")
 *   (gdb) continue
 */

void phpbreak(const char* name) {
  HPHP::dbg.enabledFunctions.insert(std::string(name));
}

void phpdisable(const char *name) {
  HPHP::dbg.enabledFunctions.erase(std::string(name));
}
