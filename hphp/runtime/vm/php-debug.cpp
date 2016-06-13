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

#include "hphp/runtime/vm/php-debug.h"

#include <set>
#include <string>

#include <folly/String.h>

#include "hphp/util/compilation-flags.h"
#include "hphp/util/debug.h"
#include "hphp/util/trace.h"

namespace HPHP {

struct PhpDebugger {
  std::set<std::string> enabledFunctions;

  PhpDebugger() {
    if (!debug) return;

    if (auto const env = getenv("PHPBREAKPOINTS")) {
      folly::splitTo<std::string>(
        ',',
        env,
        std::inserter(enabledFunctions, enabledFunctions.begin()),
        true /*ignoreEmpty*/
      );
    }
  }

  bool isBpFunction(const char* nm) const {
    return enabledFunctions.find(nm) != enabledFunctions.end();
  }
};

static PhpDebugger dbg;

#ifdef DEBUG
bool phpBreakpointEnabled(const char* sourceName) {
  return dbg.isBpFunction(sourceName);
}
#endif

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
