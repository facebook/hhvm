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

#include "hphp/runtime/vm/jit/trans-db.h"

#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator.h"

#include "hphp/runtime/base/runtime-option.h"

#include "hphp/util/timer.h"

#include <folly/SharedMutex.h>

#include <map>
#include <memory>
#include <vector>

namespace HPHP { namespace jit { namespace transdb { namespace {
std::map<TCA, TransID> s_transDB;
std::vector<std::unique_ptr<TransRec>> s_translations;
folly::SharedMutex s_lock;
}

bool enabled() {
  return debug || tc::dumpEnabled();
}

const TransRec* getTransRec(TCA tca) {
  if (!enabled()) return nullptr;
  TransRec* ret = nullptr;
  {
    folly::SharedMutex::ReadHolder guard(s_lock);
    auto it = s_transDB.upper_bound(tca);
    if (it == s_transDB.begin()) {
      return nullptr;
    }
    --it;                               // works for s_transDB::end()
    if (it->second >= s_translations.size()) {
      return nullptr;
    }
    ret = s_translations[it->second].get();
  }
  if (ret->contains(tca)) return ret;
  return nullptr;
}

const TransRec* getTransRec(TransID transId) {
  if (!enabled()) return nullptr;

  always_assert(transId < s_translations.size());
  return s_translations[transId].get();
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
      show(transRec.src).c_str(),
      transRec.aLen,
      transRec.acoldLen,
      static_cast<int>(transRec.kind));
  }

  if (!enabled()) return;
  tc::assertOwnsCodeLock();

  auto transRecPtr = std::make_unique<TransRec>(transRec);
  std::unique_lock guard(s_lock);
  if (transRecPtr->id == kInvalidTransID) {
    transRecPtr->id = s_translations.size();
  } else {
    // If we pre-assigned and ID to a translation, make sure that it is unique.
    assertx(transRecPtr->id >= s_translations.size() ||
            s_translations[transRecPtr->id] == nullptr);
  }
  TransID id = transRecPtr->id;
  assert_flog(transRecPtr->isConsistent(), "{}", transRecPtr->print());

  if (transRecPtr->aLen > 0) {
    s_transDB[transRecPtr->aStart] = id;
  }
  if (transRecPtr->acoldLen > 0) {
    s_transDB[transRecPtr->acoldStart] = id;
  }
  if (transRecPtr->afrozenLen > 0) {
    s_transDB[transRecPtr->afrozenStart] = id;
  }
  if (id >= s_translations.size()) {
    s_translations.resize(id + 1);
  }
  // Optimize storage of the created TransRec.
  transRecPtr->optimizeForMemory();
  s_translations[id] = std::move(transRecPtr);
}

}}}
