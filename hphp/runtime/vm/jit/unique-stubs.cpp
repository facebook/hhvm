/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/unique-stubs.h"

#include "hphp/runtime/base/arch.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/abi-x64.h" // TODO(#7969111): kill
#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/unique-stubs-x64.h" // TODO(#6730846): kill
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/generator/ext_generator.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"
#include "hphp/util/disasm.h"
#include "hphp/util/trace.h"

#include <algorithm>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(ustubs);

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * TODO(#6730846): There are some arch dependencies here to be eliminated.
 */

void alignJmpTarget(CodeBlock& cb) {
  align(cb, Alignment::JmpTarget, AlignContext::Dead);
}

/*
 * LLVM catch traces expect vmfp to be in rdx.
 */
RegSet syncForLLVMCatch(Vout& v) {
  if (arch() != Arch::X64) return RegSet{};

  v << copy{rvmfp(), reg::rdx};
  return RegSet{reg::rdx};
}

void loadSavedRIP(Vout& v, Vreg d) {
  switch (arch()) {
    case Arch::X64:
      v << load{*reg::rsp, d};
      return;
    case Arch::ARM:
      not_implemented();
      return;
  }
  not_reached();
}

/*
 * On x64, native calls assume native stack alignment.  For stubs that are
 * invoked via call (rather than a jmp), we need to take care of this manually.
 */
template<class GenFunc>
void alignNativeStack(Vout& v, GenFunc gen) {
  if (arch() != Arch::X64) return gen(v);

  v << subqi{8, reg::rsp, reg::rsp, v.makeReg()};
  gen(v);
  v << addqi{8, reg::rsp, reg::rsp, v.makeReg()};
}

///////////////////////////////////////////////////////////////////////////////

template<bool async>
void loadGenFrame(Vout& v, Vreg d) {
  auto const arOff = BaseGenerator::arOff() -
    (async ? AsyncGenerator::objectOff() : Generator::objectOff());

  auto const gen = v.makeReg();

  // We have to get the Generator object from the current frame's $this, then
  // load the embedded frame.
  v << load{rvmfp()[AROFF(m_this)], gen};
  v << lea{gen[arOff], d};
}

void debuggerRetImpl(Vout& v, Vreg ar) {
  auto const soff = v.makeReg();

  v << loadl{ar[AROFF(m_soff)], soff};
  v << storel{soff, rvmtl()[unwinderDebuggerReturnOffOff()]};
  v << store{rvmsp(), rvmtl()[unwinderDebuggerReturnSPOff()]};

  auto const ret = v.makeReg();
  v << vcall{
    CppCall::direct(popDebuggerCatch),
    v.makeVcallArgs({{ar}}),
    v.makeTuple({ret}),
    Fixup{},
    DestType::SSA
  };

  auto const args = syncForLLVMCatch(v);
  v << jmpr{ret, args};
}

TCA emitInterpRet(CodeBlock& cb) {
  alignJmpTarget(cb);

  auto const start = vwrap(cb, [] (Vout& v) {
    v << lea{rvmsp()[-AROFF(m_r)], r_svcreq_arg(0)};
    v << copy{rvmfp(), r_svcreq_arg(1)};
  });
  svcreq::emit_persistent(cb, folly::none, REQ_POST_INTERP_RET);
  return start;
}

template<bool async>
TCA emitInterpGenRet(CodeBlock& cb) {
  alignJmpTarget(cb);

  auto const start = vwrap(cb, [] (Vout& v) {
    loadGenFrame<async>(v, r_svcreq_arg(0));
    v << copy{rvmfp(), r_svcreq_arg(1)};
  });
  svcreq::emit_persistent(cb, folly::none, REQ_POST_INTERP_RET);
  return start;
}

TCA emitDebuggerInterpRet(CodeBlock& cb) {
  alignJmpTarget(cb);

  return vwrap(cb, [] (Vout& v) {
    auto const ar = v.makeReg();
    v << lea{rvmsp()[-AROFF(m_r)], ar};
    debuggerRetImpl(v, ar);
  });
}

template<bool async>
TCA emitDebuggerInterpGenRet(CodeBlock& cb) {
  alignJmpTarget(cb);

  return vwrap(cb, [] (Vout& v) {
    auto const ar = v.makeReg();
    loadGenFrame<async>(v, ar);
    debuggerRetImpl(v, ar);
  });
}

///////////////////////////////////////////////////////////////////////////////

template<bool immutable>
TCA emitBindCallStub(CodeBlock& cb) {
  return vwrap(cb, [] (Vout& v) {
    auto args = VregList { v.makeReg(), v.makeReg(),
                           v.makeReg(), v.makeReg() };

    auto const imcg = reinterpret_cast<uintptr_t>(&mcg);
    v << loadqp{reg::rip[imcg], args[0]};

    // Reconstruct the address of the call from the saved RIP.
    auto const savedRIP = v.makeReg();
    auto const callLen = safe_cast<int>(smashableCallLen());
    loadSavedRIP(v, savedRIP);
    v << subqi{callLen, savedRIP, args[1], v.makeReg()};

    v << copy{rvmfp(), args[2]};
    v << movb{v.cns(immutable), args[3]};

    auto const ret = v.makeReg();

    alignNativeStack(v, [&] (Vout& v) {
      auto const handler = reinterpret_cast<void (*)()>(
        getMethodPtr(&MCGenerator::handleBindCall)
      );
      v << vcall{
        CppCall::direct(handler),
        v.makeVcallArgs({args}),
        v.makeTuple({ret}),
        Fixup{},
        DestType::SSA
      };
    });

    v << jmpr{ret};
  });
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void UniqueStubs::emitAll() {
  auto& cold = mcg->code.cold();

#define ADD(name, stub) name = add(#name, (stub))
  ADD(retHelper,                  emitInterpRet(cold));
  ADD(retInlHelper,               emitInterpRet(cold));
  ADD(genRetHelper,               emitInterpGenRet<false>(cold));
  ADD(asyncGenRetHelper,          emitInterpGenRet<true>(cold));
  ADD(debuggerRetHelper,          emitDebuggerInterpRet(cold));
  ADD(debuggerGenRetHelper,       emitDebuggerInterpGenRet<false>(cold));
  ADD(debuggerAsyncGenRetHelper,  emitDebuggerInterpGenRet<true>(cold));

  ADD(bindCallStub,           emitBindCallStub<false>(cold));
  ADD(immutableBindCallStub,  emitBindCallStub<true>(cold));
#undef ADD

  // TODO(#6730846): Kill this once all unique stubs are emitted here.
  if (arch() == Arch::X64) x64::emitUniqueStubs(*this);
}

///////////////////////////////////////////////////////////////////////////////

TCA UniqueStubs::add(const char* name, TCA start) {
  auto& cb = mcg->code.blockFor(start);
  auto const end = cb.frontier();

  FTRACE(1, "unique stub: {} @ {} -- {:4} bytes: {}\n",
         cb.name(),
         static_cast<void*>(start),
         static_cast<size_t>(end - start),
         name);

  ONTRACE(2,
          [&]{
            Disasm dasm(Disasm::Options().indent(4));
            std::ostringstream os;
            dasm.disasm(os, start, end);
            FTRACE(2, "{}\n", os.str());
          }()
         );

  mcg->recordGdbStub(cb, start, folly::sformat("HHVM::{}", name));

  auto const newStub = StubRange{name, start, end};
  auto lower = std::lower_bound(m_ranges.begin(), m_ranges.end(), newStub);

  // We assume ranges are non-overlapping.
  assertx(lower == m_ranges.end() || newStub.end <= lower->start);
  assertx(lower == m_ranges.begin() || (lower - 1)->end <= newStub.start);
  m_ranges.insert(lower, newStub);
  return start;
}

std::string UniqueStubs::describe(TCA address) {
  auto raw = [address] { return folly::sformat("{}", address); };
  if (m_ranges.empty()) return raw();

  auto const dummy = StubRange{"", address, nullptr};
  auto lower = std::upper_bound(m_ranges.begin(), m_ranges.end(), dummy);
  if (lower == m_ranges.begin()) return raw();

  --lower;
  if (lower->contains(address)) {
    return folly::sformat("{}+{:#x}", lower->name, address - lower->start);
  }
  return raw();
}

///////////////////////////////////////////////////////////////////////////////

}}
