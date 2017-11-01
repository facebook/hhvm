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

#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"

#include "hphp/runtime/base/request-injection-data.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/code-gen-tls.h"
#include "hphp/runtime/vm/jit/debugger.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/arch.h"
#include "hphp/util/timer.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace tc {

namespace {

void addDbgGuardImpl(SrcKey sk, SrcRec* sr, CodeBlock& cb, DataBlock& data,
                     CGMeta& fixups) {
  TCA realCode = sr->getTopTranslation();
  if (!realCode) return;  // No translations, nothing to do.

  auto const dbgGuard = vwrap(cb, data, fixups, [&] (Vout& v) {
    if (sk.resumeMode() == ResumeMode::None) {
      auto const off = sr->nonResumedSPOff();
      v << lea{rvmfp()[-cellsToBytes(off.offset)], rvmsp()};
    }

    auto const tinfo = v.makeReg();
    auto const attached = v.makeReg();
    auto const sf = v.makeReg();

    auto const done = v.makeBlock();

    constexpr size_t dbgOff =
      offsetof(ThreadInfo, m_reqInjectionData) +
      RequestInjectionData::debuggerReadOnlyOffset();

    v << ldimmq{reinterpret_cast<uintptr_t>(sk.pc()), rarg(0)};

    emitTLSLoad(v, tls_datum(ThreadInfo::s_threadInfo), tinfo);
    v << loadb{tinfo[dbgOff], attached};
    v << testbi{static_cast<int8_t>(0xffu), attached, sf};

    v << jcci{CC_NZ, sf, done, ustubs().interpHelper};

    v = done;
    v << fallthru{};
  }, CodeKind::Helper);

  // Emit a jump to the actual code.
  auto const dbgBranchGuardSrc = emitSmashableJmp(cb, fixups, realCode);

  // Add the guard to the SrcRec.
  sr->addDebuggerGuard(dbgGuard, dbgBranchGuardSrc);
}

///////////////////////////////////////////////////////////////////////////////
}

bool addDbgGuards(const Unit* unit) {
  // TODO refactor
  // It grabs the write lease and iterates through whole SrcDB...
  struct timespec tsBegin, tsEnd;
  {
    auto codeLock = lockCode();
    auto metaLock = lockMetadata();

    auto view = code().view();
    auto& main = view.main();
    auto& data = view.data();

    HPHP::Timer::GetMonotonicTime(tsBegin);
    // Doc says even find _could_ invalidate iterator, in practice it should
    // be very rare, so go with it now.
    CGMeta fixups;
    for (auto& pair : srcDB()) {
      SrcKey const sk = SrcKey::fromAtomicInt(pair.first);
      // We may have a SrcKey to a deleted function. NB: this may miss a
      // race with deleting a Func. See task #2826313.
      if (!Func::isFuncIdValid(sk.funcID())) continue;
      SrcRec* sr = pair.second;
      auto srLock = sr->writelock();
      if (sr->unitMd5() == unit->md5() &&
          !sr->hasDebuggerGuard() &&
          isSrcKeyInDbgBL(sk)) {
        addDbgGuardImpl(sk, sr, main, data, fixups);
      }
    }
    fixups.process(nullptr);
  }

  HPHP::Timer::GetMonotonicTime(tsEnd);
  int64_t elapsed = gettime_diff_us(tsBegin, tsEnd);
  if (Trace::moduleEnabledRelease(Trace::mcg, 5)) {
    Trace::traceRelease("addDbgGuards got lease for %" PRId64 " us\n", elapsed);
  }
  return true;
}

bool addDbgGuardHelper(const Func* func, Offset offset,
                       ResumeMode resumeMode, bool hasThis) {
  SrcKey sk{func, offset, resumeMode, hasThis};
  if (auto const sr = srcDB().find(sk)) {
    if (sr->hasDebuggerGuard()) {
      return true;
    }
  } else {
    // no translation yet
    return true;
  }
  if (debug) {
    if (!isSrcKeyInDbgBL(sk)) {
      TRACE(5, "calling addDbgGuard on PC that is not in blacklist");
      return false;
    }
  }

  auto codeLock = lockCode();
  auto metaLock = lockMetadata();

  CGMeta fixups;
  if (auto sr = srcDB().find(sk)) {
    auto view = code().view();
    addDbgGuardImpl(sk, sr, view.main(), view.data(), fixups);
  }
  fixups.process(nullptr);
  return true;
}

bool addDbgGuard(const Func* func, Offset offset, ResumeMode resumeMode) {
  auto const ret = addDbgGuardHelper(func, offset, resumeMode, false);
  if (!ret || !func->cls() || func->isStatic()) return ret;
  return addDbgGuardHelper(func, offset, resumeMode, true);
}

}}}
