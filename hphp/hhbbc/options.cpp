/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/hhbbc/options.h"

#include "hphp/util/alloc.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

struct Options options;

void profile_memory(const char* what, const char* when,
                    const std::string& extra) {
  if (options.profileMemory.empty()) return;

  auto name = folly::sformat(
    "{}_{}{}_{}",
    options.profileMemory,
    what,
    extra.empty() ? extra : folly::sformat("_{}", extra),
    when
  );

  while (true) {
    auto const pos = name.find_first_of(" :\"'");
    if (pos == std::string::npos) break;
    name[pos] = '_';
  }
  jemalloc_pprof_dump(name, true);
}

//////////////////////////////////////////////////////////////////////

}}
