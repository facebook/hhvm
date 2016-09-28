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

#include "hphp/runtime/vm/jit/trans-db.h"

#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator.h"

#include "hphp/runtime/base/runtime-option.h"

#include "hphp/util/compilation-flags.h"
#include "hphp/util/timer.h"

#include <map>
#include <vector>

namespace HPHP { namespace jit { namespace transdb { namespace {
std::map<TCA, TransID> s_transDB;
std::vector<TransRec> s_translations;
}

bool enabled() {
  return debug ||
         RuntimeOption::EvalDumpTC ||
         RuntimeOption::EvalDumpIR ||
         RuntimeOption::EvalDumpRegion;
}

const TransRec* getTransRec(TCA tca) {
  if (!enabled()) return nullptr;

  auto const it = s_transDB.find(tca);
  if (it == s_transDB.end()) {
    return nullptr;
  }
  if (it->second >= s_translations.size()) {
    return nullptr;
  }
  return &s_translations[it->second];
}

const TransRec* getTransRec(TransID transId) {
  if (!enabled()) return nullptr;

  always_assert(transId < s_translations.size());
  return &s_translations[transId];
}

size_t getNumTranslations() {
  return s_translations.size();
}

void addTranslation(const TransRec& transRec) {
  if (Trace::moduleEnabledRelease(Trace::trans, 1)) {
    // Log the translation's size, creation time, SrcKey, and size
    Trace::traceRelease(
      "New translation: %" PRId64 " %s %u %u %d\n",
      HPHP::Timer::GetCurrentTimeMicros() - mcgen::jitInitTime(),
      folly::format("{}:{}:{}",
                    transRec.src.unit()->filepath(),
                    transRec.src.funcID(),
                    transRec.src.offset()).str().c_str(),
      transRec.aLen,
      transRec.acoldLen,
      static_cast<int>(transRec.kind));
  }

  if (!enabled()) return;
  tc::assertOwnsCodeLock();
  TransID id = transRec.id == kInvalidTransID ? s_translations.size()
                                              : transRec.id;
  if (id >= s_translations.size()) {
    s_translations.resize(id + 1);
  }
  s_translations[id] = transRec;
  s_translations[id].id = id;

  if (transRec.aLen > 0) {
    s_transDB[transRec.aStart] = id;
  }
  if (transRec.acoldLen > 0) {
    s_transDB[transRec.acoldStart] = id;
  }

  // Optimize storage of the created TransRec.
  s_translations[id].optimizeForMemory();
}

}}}
