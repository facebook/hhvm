/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/back-end-x64.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/disasm.h"
#include "hphp/util/text-color.h"

#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/func-prologues-x64.h"
#include "hphp/runtime/vm/jit/unique-stubs-x64.h"
#include "hphp/runtime/vm/jit/reg-alloc-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/service-requests-x64.h"

namespace HPHP { namespace JIT {

using namespace reg;

extern "C" void enterTCHelper(Cell* vm_sp,
                              ActRec* vm_fp,
                              TCA start,
                              TReqInfo* infoPtr,
                              ActRec* firstAR,
                              void* targetCacheBase);

namespace X64 {

TRACE_SET_MOD(hhir);

struct BackEnd : public JIT::BackEnd {
  BackEnd() {}
  ~BackEnd() {}

  Abi abi() override {
    return X64::abi;
  }

  size_t cacheLineSize() override {
    return 64;
  }

  PhysReg rSp() override {
    return PhysReg(reg::rsp);
  }

  PhysReg rVmSp() override {
    return X64::rVmSp;
  }

  PhysReg rVmFp() override {
    return X64::rVmFp;
  }

  Constraint srcConstraint(const IRInstruction& inst, unsigned i) override {
    return X64::srcConstraint(inst, i);
  }

  Constraint dstConstraint(const IRInstruction& inst, unsigned i) override {
    return X64::dstConstraint(inst, i);
  }

  RegPair precolorSrc(const IRInstruction& inst, unsigned i) override;
  RegPair precolorDst(const IRInstruction& inst, unsigned i) override;

  /*
   * enterTCHelper does not save callee-saved registers except %rbp. This means
   * when we call it from C++, we have to tell gcc to clobber all the other
   * callee-saved registers.
   */
  #define CALLEE_SAVED_BARRIER() \
    asm volatile("" : : : "rbx", "r12", "r13", "r14", "r15")

  /*
   * enterTCHelper is a handwritten assembly function that transfers control in
   * and out of the TC.
   */
  static_assert(X64::rVmSp == rbx &&
                X64::rVmFp == rbp &&
                X64::rVmTl == r12 &&
                X64::rStashedAR == r15,
                "__enterTCHelper needs to be modified to use the correct ABI");
  static_assert(REQ_BIND_CALL == 0x1,
                "Update assembly test for REQ_BIND_CALL in __enterTCHelper");

  void enterTCHelper(TCA start, TReqInfo& info) override {
    // We have to force C++ to spill anything that might be in a callee-saved
    // register (aside from rbp). enterTCHelper does not save them.
    CALLEE_SAVED_BARRIER();
    auto& regs = vmRegsUnsafe();
    JIT::enterTCHelper(regs.stack.top(), regs.fp, start,
                       &info, vmFirstAR(), RDS::tl_base);
    CALLEE_SAVED_BARRIER();
  }

  JIT::CodeGenerator* newCodeGenerator(const IRUnit& unit,
                                       CodeBlock& mainCode,
                                       CodeBlock& coldCode,
                                       CodeBlock& frozenCode,
                                       CodegenState& state) override {
    return new X64::CodeGenerator(unit, mainCode, coldCode,
                                  frozenCode, state);
  }

  void moveToAlign(CodeBlock& cb,
                   MoveToAlignFlags alignment
                   = MoveToAlignFlags::kJmpTargetAlign) override {
    size_t x64Alignment;

    switch (alignment) {
    case MoveToAlignFlags::kJmpTargetAlign:
      x64Alignment = kJmpTargetAlign;
      break;
    case MoveToAlignFlags::kNonFallthroughAlign:
      x64Alignment = JIT::kNonFallthroughAlign;
      break;
    case MoveToAlignFlags::kCacheLineAlign:
      x64Alignment = kCacheLineSize;
      break;
    }
    X64::moveToAlign(cb, x64Alignment);
  }

  UniqueStubs emitUniqueStubs() override {
    return X64::emitUniqueStubs();
  }

  TCA emitServiceReqWork(CodeBlock& cb, TCA start, SRFlags flags,
                         ServiceRequest req,
                         const ServiceReqArgVec& argv) override {
    return X64::emitServiceReqWork(cb, start, flags, req, argv);
  }

  void emitInterpReq(CodeBlock& mainCode, CodeBlock& coldCode,
                     const SrcKey& sk) override {
    Asm a { mainCode };
    // Add a counter for the translation if requested
    if (RuntimeOption::EvalJitTransCounters) {
      X64::emitTransCounterInc(a);
    }
    a.    jmp(emitServiceReq(coldCode, REQ_INTERPRET, sk.offset()));
  }

  bool funcPrologueHasGuard(TCA prologue, const Func* func) override {
    return X64::funcPrologueHasGuard(prologue, func);
  }

  TCA funcPrologueToGuard(TCA prologue, const Func* func) override {
    return X64::funcPrologueToGuard(prologue, func);
  }

  SrcKey emitFuncPrologue(CodeBlock& mainCode, CodeBlock& coldCode, Func* func,
                          bool funcIsMagic, int nPassed, TCA& start,
                          TCA& aStart) override {
    return funcIsMagic
          ? X64::emitMagicFuncPrologue(func, nPassed, start)
          : X64::emitFuncPrologue(func, nPassed, start);
  }

  TCA emitCallArrayPrologue(Func* func, DVFuncletsVec& dvs) override {
    return X64::emitCallArrayPrologue(func, dvs);
  }

  void funcPrologueSmashGuard(TCA prologue, const Func* func) override {
    X64::funcPrologueSmashGuard(prologue, func);
  }

  void emitIncStat(CodeBlock& cb, intptr_t disp, int n) override {
    X64Assembler a { cb };

    a.    pushf ();
    //    addq $n, [%fs:disp]
    a.    fs().addq(n, baseless(disp));
    a.    popf  ();
  }

  void emitTraceCall(CodeBlock& cb, int64_t pcOff) override {
    X64::emitTraceCall(cb, pcOff);
  }

  void emitFwdJmp(CodeBlock& cb, Block* target, CodegenState& state) override {
    X64::emitFwdJmp(cb, target, state);
  }

  void patchJumps(CodeBlock& cb, CodegenState& state, Block* block) override {
    void* list = state.patches[block];
    Address labelAddr = cb.frontier();
    while (list) {
      int32_t* toPatch = (int32_t*)list;
      int32_t diffToNext = *toPatch;
      ssize_t diff = labelAddr - ((Address)list + sizeof(int32_t));
      *toPatch = safe_cast<int32_t>(diff); // patch the jump address
      if (diffToNext == 0) break;
      void* next = (TCA)list - diffToNext;
      list = next;
    }
  }

  bool isSmashable(Address frontier, int nBytes, int offset = 0) override {
    assert(nBytes <= int(kCacheLineSize));
    uintptr_t iFrontier = uintptr_t(frontier) + offset;
    uintptr_t lastByte = uintptr_t(frontier) + nBytes - 1;
    return (iFrontier & ~kCacheLineMask) == (lastByte & ~kCacheLineMask);
  }

 private:
  void prepareForSmashImpl(CodeBlock& cb, int nBytes, int offset) {
    if (!isSmashable(cb.frontier(), nBytes, offset)) {
      X64Assembler a { cb };
      int gapSize = (~(uintptr_t(a.frontier()) + offset) & kCacheLineMask) + 1;
      a.emitNop(gapSize);
      assert(isSmashable(a.frontier(), nBytes, offset));
    }
  }

 public:
  void prepareForSmash(CodeBlock& cb, int nBytes, int offset = 0) override {
    prepareForSmashImpl(cb, nBytes, offset);
    mcg->cgFixups().m_alignFixups.emplace(cb.frontier(),
                                          std::make_pair(nBytes, offset));
  }

  void prepareForTestAndSmash(CodeBlock& cb, int testBytes,
                              TestAndSmashFlags flags) override {
    using namespace X64;
    switch (flags) {
      case TestAndSmashFlags::kAlignJcc:
        prepareForSmash(cb, testBytes + kJmpccLen, testBytes);
        assert(isSmashable(cb.frontier() + testBytes, kJmpccLen));
        break;
      case TestAndSmashFlags::kAlignJccImmediate:
        prepareForSmash(cb,
                        testBytes + kJmpccLen,
                        testBytes + kJmpccLen - kJmpImmBytes);
        assert(isSmashable(cb.frontier() + testBytes, kJmpccLen,
                           kJmpccLen - kJmpImmBytes));
        break;
      case TestAndSmashFlags::kAlignJccAndJmp:
        // Ensure that the entire jcc, and the entire jmp are smashable
        // (but we dont need them both to be in the same cache line)
        prepareForSmash(cb, testBytes + kJmpccLen, testBytes);
        prepareForSmash(cb, testBytes + kJmpccLen + kJmpLen,
                        testBytes + kJmpccLen);
        assert(isSmashable(cb.frontier() + testBytes, kJmpccLen));
        assert(isSmashable(cb.frontier() + testBytes + kJmpccLen, kJmpLen));
        break;
    }
  }

  bool supportsRelocation() const override {
    return true;
  }

  size_t relocate(RelocationInfo& rel, CodeBlock& destBlock,
                  CodeGenFixups& fixups) override {
    TCA src = rel.start();
    size_t range = rel.end() - src;
    bool hasInternalRefs = false;
    bool internalRefsNeedUpdating = false;

    while (src != rel.end()) {
      assert(src < rel.end());
      DecodedInstruction di(src);

      int destRange = 0;
      auto af = fixups.m_alignFixups.equal_range(src);
      while (af.first != af.second) {
        TCA tmp = destBlock.frontier();
        prepareForSmashImpl(destBlock,
                            af.first->second.first, af.first->second.second);
        if (destBlock.frontier() != tmp) {
          destRange += destBlock.frontier() - tmp;
          internalRefsNeedUpdating = true;
        }
        ++af.first;
      }

      TCA dest = destBlock.frontier();
      destBlock.bytes(di.size(), src);
      DecodedInstruction d2(dest);
      if (di.hasPicOffset()) {
        /*
         * Rip-relative offsets that point outside the range
         * being moved need to be adjusted so they continue
         * to point at the right thing
         */
        if (size_t(di.picAddress() - rel.start()) > range) {
          bool DEBUG_ONLY success = d2.setPicAddress(di.picAddress());
          assert(success);
        } else {
          if (d2.isBranch() && d2.shrinkBranch()) {
            internalRefsNeedUpdating = true;
          }
          hasInternalRefs = true;
        }
      }
      if (di.hasImmediate()) {
        if (fixups.m_addressImmediates.count(src)) {
          if (size_t(di.immediate() - (uint64_t)rel.start()) <= range) {
            hasInternalRefs = internalRefsNeedUpdating = true;
          }
        } else {
          if (fixups.m_addressImmediates.count((TCA)~uintptr_t(src))) {
            // Handle weird, encoded offset, used by cgLdObjMethod
            always_assert(di.immediate() == ((uintptr_t(src) << 1) | 1));
            bool DEBUG_ONLY success =
              d2.setImmediate(((uintptr_t)dest << 1) | 1);
            assert(success);
          }
          /*
           * An immediate that points into the range being moved, but which
           * isn't tagged as an addressImmediate, is most likely a bug
           * and its instruction's address needs to be put into
           * fixups.m_addressImmediates. But it could just happen by bad
           * luck, so just log it.
           */
          if (size_t(di.immediate() - (uint64_t)rel.start()) <= range) {
            FTRACE(3,
                   "relocate: instruction at {} has immediate 0x{:x}"
                   "which looks like an address that needs relocating\n",
                   src, di.immediate());
          }
        }
      }

      rel.recordAddress(src, dest - destRange, destRange);
      if (di.isNop()) {
        internalRefsNeedUpdating = true;
      } else {
        dest += d2.size();
      }
      assert(dest <= destBlock.frontier());
      destBlock.setFrontier(dest);
      src += di.size();
    }

    rel.recordAddress(src, destBlock.frontier(), 0);

    if (hasInternalRefs && internalRefsNeedUpdating) {
      src = rel.start();
      while (src != rel.end()) {
        DecodedInstruction di(src);
        TCA newPicAddress = nullptr;
        int64_t newImmediate = 0;
        if (di.hasPicOffset() &&
            size_t(di.picAddress() - rel.start()) <= range) {
          newPicAddress = rel.adjustedAddressAfter(di.picAddress());
          always_assert(newPicAddress);
        }
        if (di.hasImmediate() &&
            size_t((TCA)di.immediate() - rel.start()) <= range &&
            fixups.m_addressImmediates.count(src)) {
          newImmediate = (int64_t)rel.adjustedAddressAfter((TCA)di.immediate());
          always_assert(newImmediate);
        }
        if (newImmediate || newPicAddress) {
          TCA dest = rel.adjustedAddressAfter(src);
          DecodedInstruction d2(dest);
          if (newPicAddress) {
            d2.setPicAddress(newPicAddress);
          }
          if (newImmediate) {
            d2.setImmediate(newImmediate);
          }
        }
        src += di.size();
      }
    }

    return rel.destSize();
  }

  void adjustForRelocation(TCA start, TCA end,
                           RelocationInfo& rel,
                           CodeGenFixups& fixups) override {
    assert(rel.relocated());
    while (start != end) {
      assert(start < end);
      DecodedInstruction di(start);

      if (di.hasPicOffset()) {
        /*
         * A pointer into something that has been relocated needs to be
         * updated.
         */
        if (TCA adjusted = rel.adjustedAddressAfter(di.picAddress())) {
          di.setPicAddress(adjusted);
        }
      }

      if (di.hasImmediate()) {
        /*
         * Similarly for addressImmediates - and see comment above
         * for non-address immediates.
         */
        if (TCA adjusted = rel.adjustedAddressAfter((TCA)di.immediate())) {
          if (fixups.m_addressImmediates.count(start)) {
            di.setImmediate((int64_t)adjusted);
          } else {
            FTRACE(3,
                   "relocate: instruction at {} has immediate 0x{:x}"
                   "which looks like an address that needs relocating\n",
                   start, di.immediate());
          }
        }
      }

      start += di.size();
    }
  }

  template <typename T>
  void fixupStateVector(StateVector<T, TcaRange>& sv, RelocationInfo& rel) {
    for (auto& ii : sv) {
      if (!ii.empty()) {
        auto s = rel.adjustedAddressBefore(ii.begin());
        auto e = rel.adjustedAddressAfter(ii.end());
        if (e || s) {
          if (!s) s = ii.begin();
          if (!e) e = ii.end();
          ii = TcaRange(s, e);
        }
      }
    }
  }

  void adjustForRelocation(AsmInfo* asmInfo,
                           RelocationInfo& rel,
                           CodeGenFixups& fixups) override {
    assert(rel.relocated());

    auto& ip = fixups.m_inProgressTailJumps;
    for (size_t i = 0; i < ip.size(); ++i) {
      IncomingBranch& ib = const_cast<IncomingBranch&>(ip[i]);
      if (TCA adjusted = rel.adjustedAddressAfter(ib.toSmash())) {
        ib.adjust(adjusted);
      }
    }

    for (auto& fixup : fixups.m_pendingFixups) {
      /*
       * Pending fixups always point after the call instruction,
       * so use the "before" address, since there may be nops
       * before the next actual instruction.
       */
      if (TCA adjusted = rel.adjustedAddressBefore(fixup.m_tca)) {
        fixup.m_tca = adjusted;
      }
    }

    for (auto& ct : fixups.m_pendingCatchTraces) {
      /*
       * Similar to fixups - this is a return address
       */
      if (CTCA adjusted = rel.adjustedAddressBefore(ct.first)) {
        ct.first = adjusted;
      }
      if (TCA adjusted = rel.adjustedAddressAfter(ct.second)) {
        ct.second = adjusted;
      }
    }

    for (auto& jt : fixups.m_pendingJmpTransIDs) {
      if (TCA adjusted = rel.adjustedAddressAfter(jt.first)) {
        jt.first = adjusted;
      }
    }

    for (auto addr : fixups.m_reusedStubs) {
      /*
       * The stubs are terminated by a ud2. Check for it.
       */
      while (addr[0] != 0x0f || addr[1] != 0x0b) {
        DecodedInstruction di(addr);
        if (di.hasPicOffset()) {
          if (TCA adjusted = rel.adjustedAddressAfter(di.picAddress())) {
            di.setPicAddress(adjusted);
          }
        }
        addr += di.size();
      }
    }

    for (auto& tbc : fixups.m_bcMap) {
      if (TCA adjusted = rel.adjustedAddressBefore(tbc.aStart)) {
        tbc.aStart = adjusted;
      }
      if (TCA adjusted = rel.adjustedAddressBefore(tbc.acoldStart)) {
        tbc.acoldStart = adjusted;
      }
      if (TCA adjusted = rel.adjustedAddressBefore(tbc.afrozenStart)) {
        tbc.afrozenStart = adjusted;
      }
    }

    std::set<TCA> updated;
    for (auto addrImm : fixups.m_addressImmediates) {
      if (TCA adjusted = rel.adjustedAddressAfter(addrImm)) {
        updated.insert(adjusted);
      } else if (TCA odd = rel.adjustedAddressAfter((TCA)~uintptr_t(addrImm))) {
        // just for cgLdObjMethod
        updated.insert((TCA)~uintptr_t(odd));
      } else {
        updated.insert(addrImm);
      }
    }
    updated.swap(fixups.m_addressImmediates);

    for (auto codePtr : fixups.m_codePointers) {
      if (TCA adjusted = rel.adjustedAddressAfter(*codePtr)) {
        *codePtr = adjusted;
      }
    }

    if (asmInfo) {
      fixupStateVector(asmInfo->instRanges, rel);
      fixupStateVector(asmInfo->asmRanges, rel);
      fixupStateVector(asmInfo->acoldRanges, rel);
      fixupStateVector(asmInfo->afrozenRanges, rel);
    }
  }

 private:
  void smashJmpOrCall(TCA addr, TCA dest, bool isCall) {
    // Unconditional rip-relative jmps can also be encoded with an EB as the
    // first byte, but that means the delta is 1 byte, and we shouldn't be
    // encoding smashable jumps that way.
    assert(kJmpLen == kCallLen);
    always_assert(isSmashable(addr, X64::kJmpLen));

    auto& cb = mcg->code.blockFor(addr);
    CodeCursor cursor { cb, addr };
    X64Assembler a { cb };
    if (dest > addr && dest - addr <= X64::kJmpLen) {
      assert(!isCall);
      a.  emitNop(dest - addr);
    } else if (isCall) {
      a.  call   (dest);
    } else {
      a.  jmp    (dest);
    }
  }

 public:
  void smashJmp(TCA jmpAddr, TCA newDest) override {
    assert(MCGenerator::canWrite());
    FTRACE(2, "smashJmp: {} -> {}\n", jmpAddr, newDest);
    smashJmpOrCall(jmpAddr, newDest, false);
  }

  void smashCall(TCA callAddr, TCA newDest) override {
    assert(MCGenerator::canWrite());
    FTRACE(2, "smashCall: {} -> {}\n", callAddr, newDest);
    smashJmpOrCall(callAddr, newDest, true);
  }

  void smashJcc(TCA jccAddr, TCA newDest) override {
    assert(MCGenerator::canWrite());
    FTRACE(2, "smashJcc: {} -> {}\n", jccAddr, newDest);
    // Make sure the encoding is what we expect. It has to be a rip-relative jcc
    // with a 4-byte delta.
    assert(*jccAddr == 0x0F && (*(jccAddr + 1) & 0xF0) == 0x80);
    assert(isSmashable(jccAddr, X64::kJmpccLen));

    // Can't use the assembler to write out a new instruction, because we have
    // to preserve the condition code.
    auto newDelta = safe_cast<int32_t>(newDest - jccAddr - X64::kJmpccLen);
    auto deltaAddr = reinterpret_cast<int32_t*>(jccAddr
                                                + X64::kJmpccLen
                                                - X64::kJmpImmBytes);
    *deltaAddr = newDelta;
  }

  void emitSmashableJump(CodeBlock& cb, TCA dest, ConditionCode cc) override {
    X64Assembler a { cb };
    if (cc == CC_None) {
      assert(isSmashable(cb.frontier(), X64::kJmpLen));
      a.  jmp(dest);
    } else {
      assert(isSmashable(cb.frontier(), X64::kJmpccLen));
      a.  jcc(cc, dest);
    }
  }

  void emitSmashableCall(CodeBlock& cb, TCA dest) override {
    X64Assembler a { cb };
    assert(isSmashable(cb.frontier(), X64::kCallLen));
    a.  call(dest);
  }

  TCA jmpTarget(TCA jmp) override {
    if (jmp[0] != 0xe9) return nullptr;
    return jmp + 5 + ((int32_t*)(jmp + 5))[-1];
  }

  TCA jccTarget(TCA jmp) override {
    if (jmp[0] != 0x0F || (jmp[1] & 0xF0) != 0x80) return nullptr;
    return jmp + 6 + ((int32_t*)(jmp + 6))[-1];
  }

  TCA callTarget(TCA call) override {
    if (call[0] != 0xE8) return nullptr;
    return call + 5 + ((int32_t*)(call + 5))[-1];
  }

  void addDbgGuard(CodeBlock& codeMain, CodeBlock& codeCold,
                   SrcKey sk, size_t dbgOff) override {
    Asm a { codeMain };

    // Emit the checks for debugger attach
    auto rtmp = rAsm;
    emitTLSLoad<ThreadInfo>(a, ThreadInfo::s_threadInfo, rtmp);
    a.   loadb  (rtmp[dbgOff], rbyte(rtmp));
    a.   testb  ((int8_t)0xff, rbyte(rtmp));

    // Branch to a special REQ_INTERPRET if attached
    auto const fallback =
      emitServiceReq(codeCold, REQ_INTERPRET, sk.offset());
    a.   jnz    (fallback);
  }

  void streamPhysReg(std::ostream& os, PhysReg& reg) override {
    auto name = reg.type() == PhysReg::GP ? reg::regname(Reg64(reg)) :
      reg::regname(RegXMM(reg));
    os << name;
  }

  void disasmRange(std::ostream& os, int indent, bool dumpIR, TCA begin,
                   TCA end) override {
    Disasm disasm(Disasm::Options().indent(indent + 4)
                  .printEncoding(dumpIR)
                  .color(color(ANSI_COLOR_BROWN)));
    disasm.disasm(os, begin, end);
  }
};

std::unique_ptr<JIT::BackEnd> newBackEnd() {
  return std::unique_ptr<JIT::BackEnd>{ folly::make_unique<BackEnd>() };
}

using NativeCalls::CallMap;
using NativeCalls::Arg;
using NativeCalls::ArgType;

// return the number of registers needed to pass this arg
int argSize(const Arg& arg, const IRInstruction& inst) {
  switch (arg.type) {
    case ArgType::SSA:
    case ArgType::Imm:
    case ArgType::ExtraImm:
      return 1;
    case ArgType::TV:
      return 2;
    case ArgType::MemberKeyS:
      return inst.src(arg.ival)->isA(Type::Str) ? 1 : 2;
    case ArgType::MemberKeyIS:
      return inst.src(arg.ival)->isA(Type::Str) ? 1 :
             inst.src(arg.ival)->isA(Type::Int) ? 1 : 2;
  }
  return 1;
}

// Return the argument-register hint for a native-call opcode.
RegPair hintNativeCallSrc(const IRInstruction& inst, unsigned i) {
  auto const& args = CallMap::info(inst.op()).args;
  auto pos = 0;
  for (auto& arg : args) {
    if (argSize(arg, inst) == 1) {
      if (arg.ival == i && pos < kNumRegisterArgs) {
        return {argNumToRegName[pos], InvalidReg};
      }
      pos++;
    } else {
      if (arg.ival == i && pos + 1 < kNumRegisterArgs) {
        return {argNumToRegName[pos], argNumToRegName[pos + 1]};
      }
      pos += 2;
    }
  }
  return InvalidRegPair;
}

// return the return-value register hint for a NativeCall opcode.
RegPair hintNativeCallDst(const IRInstruction& inst, unsigned i) {
  if (i != 0) return InvalidRegPair;
  auto const& info = CallMap::info(inst.op());
  switch (info.dest) {
    case DestType::SSA:
      return {rax, InvalidReg};
    case DestType::Dbl:
      return {xmm0, InvalidReg};
    case DestType::TV:
      return {rax, rdx};
    case DestType::None:
      return InvalidRegPair;
  }
  return InvalidRegPair;
}

// Return the arg-register hint for a CallBuiltin instruction.
RegPair hintCallBuiltinSrc(const IRInstruction& inst, unsigned srcNum) {
  auto callee = inst.extra<CallBuiltin>()->callee;
  auto ipos = 0, dpos = 0, spos = 0;
  if (isCppByRef(callee->returnType())) {
    if (srcNum == 0) {
      return {argNumToRegName[0], InvalidReg};
    }
    ipos = spos = 1;
  }
  // Iterate through the builtin params, keeping track of the HHIR src
  // pos (spos) and the corresponding int (ipos) and double (dpos)
  // register argument positions.  When spos == srcNum, return a hint.
  auto& params = callee->params();
  int i = 0, n = callee->numParams();
  for (; i < n && spos < srcNum; ++i, ++spos) {
    if (params[i].builtinType == KindOfDouble) {
      dpos++;
    } else {
      ipos++;
    }
  }
  if (i < n && spos == srcNum) {
    if (params[i].builtinType == KindOfDouble) {
      if (dpos < kNumSIMDRegisterArgs) {
        return {argNumToSIMDRegName[dpos], InvalidReg};
      }
    } else {
      if (ipos < kNumRegisterArgs) {
        return {argNumToRegName[ipos], InvalidReg};
      }
    }
  }
  return InvalidRegPair;
}

// return the return-register hint for a CallBuiltin instruction
RegPair hintCallBuiltinDst(const IRInstruction& inst, unsigned i) {
  // the decision logic here is distilled from CodeGenerator::cgCallBuiltin()
  if (!RuntimeOption::EvalHHIREnablePreColoring) return InvalidRegPair;
  if (i != 0) return InvalidRegPair;
  auto returnType = inst.typeParam();
  if (returnType <= Type::Dbl) {
    return {xmm0, InvalidReg};
  }
  if (returnType.isSimpleType()) {
    return {rax, InvalidReg};
  }
  // other return types are passed via stack; generated code reloads
  // them, so using the ABI assigned registers doesn't matter.
  return InvalidRegPair;
}

RegPair BackEnd::precolorSrc(const IRInstruction& inst, unsigned i) {
  if (!RuntimeOption::EvalHHIREnablePreColoring) return InvalidRegPair;
  if (CallMap::hasInfo(inst.op())) {
    return hintNativeCallSrc(inst, i);
  }
  switch (inst.op()) {
    case CallBuiltin:
      return hintCallBuiltinSrc(inst, i);
    case Shl:
    case Shr:
      if (i == 1) return {PhysReg(rcx), InvalidReg};
      break;
    case Mod:
      // x86 idiv does: rdx:rax/r => quotent:rax, remainder:rdx
      if (i == 0) return {PhysReg(rax), InvalidReg};
    default:
      break;
  }
  return InvalidRegPair;
}

RegPair BackEnd::precolorDst(const IRInstruction& inst, unsigned i) {
  if (!RuntimeOption::EvalHHIREnablePreColoring) return InvalidRegPair;
  if (CallMap::hasInfo(inst.op())) {
    return hintNativeCallDst(inst, i);
  }
  switch (inst.op()) {
    case CallBuiltin:
      return hintCallBuiltinDst(inst, i);
    case Mod:
      // x86 idiv does: rdx:rax/r => quotent:rax, remainder:rdx
      if (i == 0) return {PhysReg(rdx), InvalidReg};
      break;
    default:
      break;
  }
  return InvalidRegPair;
}

}}}
