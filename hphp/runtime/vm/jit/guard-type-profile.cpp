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

#include "hphp/runtime/vm/jit/guard-type-profile.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"

#include "hphp/util/struct-log.h"

#include <folly/AtomicHashMap.h>

namespace HPHP { namespace jit {

namespace {
folly::AtomicHashMap<const StringData*, rds::Link<int64_t, rds::Mode::Normal>>
  s_map{20};

rds::Handle guardProfileHandle(Type t) {
  auto const name = makeStaticString(
    t.unspecialize().toString() + (t.isSpecialized() ? "<specialized>" : "")
  );
  auto const pair = s_map.emplace(name);
  if (pair.second) pair.first->second.bind(rds::Mode::Normal);
  return pair.first->second.handle();
}
}

void emitProfileGuardType(Vout& v, Type t) {
  auto const handle = guardProfileHandle(t);
  auto const sf = checkRDSHandleInitialized(v, handle);
  ifThen(v, CC_NE, sf, [&](Vout& v) {
    v << storeqi{0, rvmtl()[handle]};
    markRDSHandleInitialized(v, handle);
  });
  v << incqm{rvmtl()[handle], v.makeReg()};
}

void logGuardProfileData() {
  for (auto& pair : s_map) {
    auto& link = pair.second;
    // It's possible to see the Link after insertion but before it's bound, so
    // make sure it's bound before trying to read from it.
    link.bind(rds::Mode::Normal);
    if (!link.isInit() || *link == 0) continue;

    StructuredLogEntry log;
    log.setStr("guard_type", pair.first->slice());
    log.setInt("guard_count", *link);
    StructuredLog::log("hhvm_guard_types", log);
  }
}

}}
