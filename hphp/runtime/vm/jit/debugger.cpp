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

#ifndef incl_HPHP_JIT_DEBUGGER_H_
#define incl_HPHP_JIT_DEBUGGER_H_

#include "hphp/runtime/vm/jit/debugger.h"

#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/pc-filter.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"

#include "hphp/runtime/base/req-optional.h"

#include "hphp/util/hash-set.h"
#include "hphp/util/lock.h"
#include "hphp/util/mutex.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(debugger);

namespace HPHP { namespace jit { namespace {
hphp_hash_set<const Func*> s_dbgBLFuncs;
Mutex s_dbgBlacklistLock;
}

bool isSrcKeyInDbgBL(SrcKey sk) {
  auto unit = sk.unit();
  if (unit->isInterpretOnly()) return true;
  Lock l(s_dbgBlacklistLock);
  return s_dbgBLFuncs.count(sk.func());
}

void clearDbgBL() {
  Lock l(s_dbgBlacklistLock);
  s_dbgBLFuncs.clear();
}

bool addDbgBLFunc(const Func* func) {
  Lock l(s_dbgBlacklistLock);
  return s_dbgBLFuncs.emplace(func).second;
}

struct DebuggerCatches {
  // keys could point to resumable ActRecs in req heap
  req::Optional<req::fast_map<const ActRec*, TCA>> catches;
};

THREAD_LOCAL(DebuggerCatches, tl_debuggerCatches);

void stashDebuggerCatch(const ActRec* fp) {
  if (auto const catchBlock = getCatchTrace(TCA(fp->m_savedRip))) {
    // Record the corresponding catch trace for `fp'.  There might not be one,
    // if the one we would have registered was empty.
    always_assert(*catchBlock);
    FTRACE(1, "Pushing debugger catch {} with fp {}\n", *catchBlock, fp);
    auto& catches = tl_debuggerCatches->catches;
    if (!catches) catches.emplace(); // create map
    catches->emplace(fp, *catchBlock);
  }
}

TCA unstashDebuggerCatch(const ActRec* fp) {
  if (tl_debuggerCatches.isNull()) {
    return tc::ustubs().endCatchHelper;
  }
  auto& catches = tl_debuggerCatches->catches;
  if (!catches) {
    return tc::ustubs().endCatchHelper;
  }
  auto const it = catches->find(fp);
  if (it == catches->end()) {
    return tc::ustubs().endCatchHelper;
  }

  auto const catchBlock = it->second;
  catches->erase(it);
  FTRACE(1, "Popped debugger catch {} for fp {}\n", catchBlock, fp);
  return catchBlock;
}

void clearDebuggerCatches() {
  tl_debuggerCatches.destroy();
}

}}

#endif
