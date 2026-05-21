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

#include "hphp/runtime/vm/live-func-stats.h"

#include "hphp/util/struct-log.h"

namespace HPHP {
  LiveFunctionKind inferKindToLog() {
    return Cfg::Repo::Authoritative
      ? LiveFunctionKind::RepoAuthoritative
      : LiveFunctionKind::Sandbox;
  }
  const char* liveFunctionKindToString(LiveFunctionKind kind) {
    switch (kind) {
      case LiveFunctionKind::RepoAuthoritative: return "prod";
      case LiveFunctionKind::Sandbox: return "sandbox";
      case LiveFunctionKind::Test: return "test";
    }
    not_reached();
  }

  void markFunctionAsLive(const Func* func, Optional<LiveFunctionKind> kind) {
    if (!kind) {
      kind = inferKindToLog();
    }

    StructuredLogEntry entry;
    entry.force_init = true;
    entry.setStr("function_name", func->fullName()->data());
    entry.setStr("mode", liveFunctionKindToString(*kind));
    StructuredLog::log("hhvm_live_functions", entry);
  }
}
