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

#include "hphp/runtime/vm/jit/service-requests.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/func-order.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/tc-record.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/util/arch.h"
#include "hphp/util/configs/jit.h"
#include "hphp/util/data-block.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/trace.h"

#include "hphp/vixl/a64/macro-assembler-a64.h"
#include "hphp/vixl/a64/disasm-a64.h"

namespace HPHP::jit::svcreq {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(servicereq);

///////////////////////////////////////////////////////////////////////////////

namespace {

uint64_t toStubKey(StubType type, SrcKey sk, SBInvOffset spOff) {
  auto const t = static_cast<uint8_t>(type);
  auto const bcOffOrNumArgs = sk.funcEntry() ? sk.numEntryArgs() : sk.offset();
  assertx(t < (1 << 2));
  assertx(0 <= bcOffOrNumArgs && bcOffOrNumArgs < (1LL << 30));
  assertx(0 <= spOff.offset && spOff.offset < (1LL << 31));
  return
    (static_cast<uint64_t>(t) << 62) +
    (static_cast<uint64_t>(sk.funcEntry()) << 61) +
    (static_cast<uint64_t>(bcOffOrNumArgs) << 31) +
    (static_cast<uint64_t>(spOff.offset));
}

folly_concurrent_hash_map_simd<uint64_t, TCA> s_stubMap;
folly_concurrent_hash_map_simd<TCA, bool>     s_stubSet;

TCA typeToHandler(StubType type) {
  switch (type) {
    case StubType::Translate:   return tc::ustubs().handleTranslate;
    case StubType::Retranslate: return tc::ustubs().handleRetranslate;
    default:                    not_reached();
  }
}

TCA typeToFuncEntryHandler(StubType type) {
  switch (type) {
    case StubType::Translate:   return tc::ustubs().handleTranslateFuncEntry;
    case StubType::Retranslate: return tc::ustubs().handleRetranslateFuncEntry;
    default:                    not_reached();
  }
}

std::string typeToName(StubType type) {
  switch (type) {
    case StubType::Translate:   return "translate";
    case StubType::Retranslate: return "retranslate";
    default:                    not_reached();
  }
}

std::atomic<bool> s_fullForStub{false};

TCA emitStub(StubType type, SrcKey sk, SBInvOffset spOff) {
  FTRACE(2, "svcreq::emitStub {} @{} {}\n",
         typeToName(type), showShort(sk), spOff.offset);
  assertx(!sk.prologue());

  if (s_fullForStub.load(std::memory_order_relaxed)) {
    FTRACE(4, "  no space for {}, bailing\n", showShort(sk));
    return nullptr;
  }

  tracing::Pause _p;
  tracing::Block _{"svcreq::emitStub"};

  auto codeLock = tc::lockCode();

  auto view = tc::code().view(TransKind::Anchor);
  TCA mainStart = view.main().frontier();
  TCA coldStart = view.cold().frontier();
  TCA frozenStart = view.frozen().frontier();

  auto const emit = [&] (Vout& v) {
    assertx(!sk.funcEntry());
    v << copy{v.cns(sk.offset()), rarg(0)};
    v << copy{v.cns(spOff.offset), rarg(1)};
    v << jmpi{typeToHandler(type), cross_trace_regs() | arg_regs(2)};
  };

  auto const emitFuncEntry = [&] (Vout& v) {
    assertx(sk.funcEntry());
    assertx(spOff == SBInvOffset{0});
    v << copy{v.cns(sk.numEntryArgs()), rarg(3)};
    v << jmpi{typeToFuncEntryHandler(type),
              func_entry_regs(true /* withCtx */) | rarg(3)};
  };

  auto const start = vwrap(
    view.cold(),
    view.data(),
    [&] (Vout& v) {
      if (!sk.funcEntry()) {
        emit(v);
      } else {
        emitFuncEntry(v);
      }
    },
    nullptr,
    false, /* relocate */
    true   /* nullOnFull */
  );

  // We passed true to nullOnFull, so if the TC was out of space, we
  // just get a nullptr address.
  if (!start) {
    FTRACE(4, "  ran out of space while making stub for {}\n", showShort(sk));
    s_fullForStub.store(true, std::memory_order_relaxed);
    return nullptr;
  }

  assertx(view.main().frontier() == mainStart);
  assertx(view.cold().frontier() != coldStart);
  assertx(view.frozen().frontier() == frozenStart);

  if (RuntimeOption::EvalDumpTCAnchors) {
    auto metaLock = tc::lockMetadata();
    auto const transID = profData() && transdb::enabled()
      ? profData()->allocTransID() : kInvalidTransID;
    TransRec tr(sk, transID, TransKind::Anchor, mainStart, 0,
                coldStart, view.cold().frontier() - coldStart, frozenStart, 0);
    transdb::addTranslation(tr);
    FuncOrder::recordTranslation(tr);
    if (Cfg::Jit::UseVtuneAPI) {
      reportTraceletToVtune(sk.unit(), sk.func(), tr);
    }
    tc::recordTranslationSizes(tr);

    assertx(!transdb::enabled() ||
            transdb::getTransRec(coldStart)->kind == TransKind::Anchor);
  }

  FTRACE(4, "  emitted stub {} for {}\n", start, showShort(sk));
  return start;
}

}

TCA getOrEmitStub(StubType type, SrcKey sk, SBInvOffset spOff) {
  assertx(!sk.prologue());

  auto const key = toStubKey(type, sk, spOff);
  auto const it = s_stubMap.find(key);
  if (it != s_stubMap.end()) return it->second;

  auto const stub = emitStub(type, sk, spOff);
  if (stub == nullptr) return nullptr;

  auto const pair = s_stubMap.insert({key, stub});
  if (!pair.second) return pair.first->second;
  s_stubSet.insert({stub, true});
  return stub;
}

bool isStub(TCA addr) {
  return s_stubSet.find(addr) != s_stubSet.end();
}

///////////////////////////////////////////////////////////////////////////////

TCA emit_interp_no_translate_stub(SBInvOffset spOff, SrcKey sk) {
  FTRACE(2, "interp_no_translate_stub @{} {}\n", showShort(sk), spOff.offset);

  // No point on trying to emit if we already failed once.
  if (s_fullForStub.load(std::memory_order_relaxed)) {
    FTRACE(4, "  no space for {}, bailing\n", showShort(sk));
    return nullptr;
  }

  tracing::Pause _p;
  tracing::Block _{"emit-interp-no-translate-stub"};

  auto codeLock = tc::lockCode();
  auto metaLock = tc::lockMetadata();

  auto view = tc::code().view();
  auto& cb = view.frozen();
  auto& data = view.data();

  auto const start = vwrap(
    cb,
    data,
    [&] (Vout& v) { emitInterpReqNoTranslate(v, sk, spOff); },
    nullptr,
    false,
    true /* nullOnFull */
  );

  // We passed true to nullOnFull, so if the TC was out of space, we
  // just get a nullptr address.
  if (!start) {
    FTRACE(4, "  ran out of space while making stub for {}\n", showShort(sk));
    s_fullForStub.store(true, std::memory_order_relaxed);
  }
  FTRACE(4, "  emitted stub {} for {}\n", start, showShort(sk));
  return start;
}

///////////////////////////////////////////////////////////////////////////////

}
