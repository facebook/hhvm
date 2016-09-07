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

#include "hphp/runtime/base/perf-warning.h"

#include <folly/String.h>

#include "hphp/util/stack-trace.h"

namespace HPHP {

void logAHMSubMapWarning(folly::StringPiece mapName) {
  StackTrace st;
  logPerfWarning(
    "AtomicHashMap overflow",
    [&](StructuredLogEntry& cols) {
      cols.setStr("map_name", mapName);
      cols.setStackTrace("stack", st);
    }
  );
}

}
