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
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
#include "hphp/runtime/vm/jit/func-prologues-x64.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/reg-alloc-x64.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/service-requests-x64.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/unique-stubs-x64.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-llvm.h"

namespace HPHP { namespace jit {

using namespace reg;

extern "C" void enterTCHelper(Cell* vm_sp,
                              ActRec* vm_fp,
                              TCA start,
                              TReqInfo* infoPtr,
                              ActRec* firstAR,
                              void* targetCacheBase);

namespace x64 {

TRACE_SET_MOD(hhir);

struct BackEnd : public jit::BackEnd {
  BackEnd() {}
  ~BackEnd() {}

  Abi abi() override {
    return x64::abi;
  }

  size_t cacheLineSize() override {
    return 64;
  }

  PhysReg rSp() override {
    return PhysReg(reg::rsp);
  }

  PhysReg rVmSp() override {
    return x64::rVmSp;
  }

  PhysReg rVmFp() override {
    return x64::rVmFp;
  }

  PhysReg rVmTl() override {
    return x64::rVmTl;
  }

  bool storesCell(const IRInstruction& inst, uint32_t srcIdx) override {
    return x64::storesCell(inst, srcIdx);
  }

  bool loadsCell(const IRInstruction& inst) override {
    return x64::loadsCell(inst.op());
  }

  /*
   * enterTCHelper does not save callee-saved registers except %rbp. This means
   * when we call it from C++, we have to tell gcc to clobber all the other
   * callee-saved registers.
   */
  #define CALLEE_SAVED_BARRIER()                                    \
      asm volatile("" : : : "rbx", "r12", "r13", "r14", "r15");

  /*
   * enterTCHelper is a handwritten assembly function that transfers control in
   * and out of the TC.
   */
  static_assert(x64::rVmSp == rbx &&
                x64::rVmFp == rbp &&
                x64::rVmTl == r12 &&
                x64::rStashedAR == r15,
                "__enterTCHelper needs to be modified to use the correct ABI");
  static_assert(REQ_BIND_CALL == 0x1,
                "Update assembly test for REQ_BIND_CALL in __enterTCHelper");

  void enterTCHelper(TCA start, TReqInfo& info) override {
    // We have to force C++ to spill anything that might be in a callee-saved
    // register (aside from rbp). enterTCHelper does not save them.
    CALLEE_SAVED_BARRIER();
    auto& regs = vmRegsUnsafe();
    jit::enterTCHelper(regs.stack.top(), regs.fp, start,
                       &info, vmFirstAR(), RDS::tl_base);
    CALLEE_SAVED_BARRIER();
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
      x64Alignment = jit::kNonFallthroughAlign;
      break;
    case MoveToAlignFlags::kCacheLineAlign:
      x64Alignment = kCacheLineSize;
      break;
    }
    x64::moveToAlign(cb, x64Alignment);
  }

  UniqueStubs emitUniqueStubs() override {
    return x64::emitUniqueStubs();
  }

  TCA emitServiceReqWork(CodeBlock& cb, TCA start, SRFlags flags,
                         ServiceRequest req,
                         const ServiceReqArgVec& argv) override {
    return x64::emitServiceReqWork(cb, start, flags, req, argv);
  }

  void emitInterpReq(CodeBlock& mainCode, CodeBlock& coldCode,
                     SrcKey sk) override {
    Asm a { mainCode };
    // Add a counter for the translation if requested
    if (RuntimeOption::EvalJitTransCounters) {
      x64::emitTransCounterInc(a);
    }
    a.    jmp(emitServiceReq(coldCode, REQ_INTERPRET, sk.offset()));
  }

  bool funcPrologueHasGuard(TCA prologue, const Func* func) override {
    return x64::funcPrologueHasGuard(prologue, func);
  }

  TCA funcPrologueToGuard(TCA prologue, const Func* func) override {
    return x64::funcPrologueToGuard(prologue, func);
  }

  SrcKey emitFuncPrologue(CodeBlock& mainCode, CodeBlock& coldCode, Func* func,
                          bool funcIsMagic, int nPassed, TCA& start,
                          TCA& aStart) override {
    return funcIsMagic
          ? x64::emitMagicFuncPrologue(func, nPassed, start)
          : x64::emitFuncPrologue(func, nPassed, start);
  }

  TCA emitCallArrayPrologue(Func* func, DVFuncletsVec& dvs) override {
    return x64::emitCallArrayPrologue(func, dvs);
  }

  void funcPrologueSmashGuard(TCA prologue, const Func* func) override {
    x64::funcPrologueSmashGuard(prologue, func);
  }

  void emitIncStat(CodeBlock& cb, intptr_t disp, int n) override {
    X64Assembler a { cb };

    a.    pushf ();
    //    addq $n, [%fs:disp]
    a.    fs().addq(n, baseless(disp));
    a.    popf  ();
  }

  void emitTraceCall(CodeBlock& cb, Offset pcOff) override {
    x64::emitTraceCall(cb, pcOff);
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
    using namespace x64;
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
        prepareForSmashImpl(cb, testBytes + kJmpccLen, testBytes);
        prepareForSmashImpl(cb, testBytes + kJmpccLen + kJmpLen,
                            testBytes + kJmpccLen);
        mcg->cgFixups().m_alignFixups.emplace(
          cb.frontier(), std::make_pair(testBytes + kJmpccLen, testBytes));
        mcg->cgFixups().m_alignFixups.emplace(
          cb.frontier(), std::make_pair(testBytes + kJmpccLen + kJmpLen,
                                        testBytes + kJmpccLen));
        assert(isSmashable(cb.frontier() + testBytes, kJmpccLen));
        assert(isSmashable(cb.frontier() + testBytes + kJmpccLen, kJmpLen));
        break;
    }
  }

  bool supportsRelocation() const override {
    return true;
  }

  typedef hphp_hash_set<void*> WideJmpSet;
  struct JmpOutOfRange : std::exception {};

  size_t relocate(RelocationInfo& rel,
                  CodeBlock& destBlock,
                  TCA start, TCA end,
                  CodeGenFixups& fixups,
                  TCA* exitAddr) override {
    WideJmpSet wideJmps;
    while (true) {
      try {
        return relocateImpl(rel, destBlock, start, end,
                            fixups, exitAddr, wideJmps);
      } catch (JmpOutOfRange& j) {
      }
    }
  }

  size_t relocateImpl(RelocationInfo& rel,
                      CodeBlock& destBlock,
                      TCA start, TCA end,
                      CodeGenFixups& fixups,
                      TCA* exitAddr,
                      WideJmpSet& wideJmps) {
    TCA src = start;
    size_t range = end - src;
    bool hasInternalRefs = false;
    bool internalRefsNeedUpdating = false;
    TCA destStart = destBlock.frontier();
    size_t asm_count{0};
    TCA jmpDest = nullptr;
    TCA keepNopLow = nullptr;
    TCA keepNopHigh = nullptr;
    try {
      while (src != end) {
        assert(src < end);
        DecodedInstruction di(src);
        asm_count++;

        int destRange = 0;
        auto af = fixups.m_alignFixups.equal_range(src);
        while (af.first != af.second) {
          auto low = src + af.first->second.second;
          auto hi = src + af.first->second.first;
          assert(low < hi);
          if (!keepNopLow || keepNopLow > low) keepNopLow = low;
          if (!keepNopHigh || keepNopHigh < hi) keepNopHigh = hi;
          TCA tmp = destBlock.frontier();
          prepareForSmashImpl(destBlock,
                              af.first->second.first, af.first->second.second);
          if (destBlock.frontier() != tmp) {
            destRange += destBlock.frontier() - tmp;
            internalRefsNeedUpdating = true;
          }
          ++af.first;
        }

        bool preserveAlignment = keepNopLow && keepNopHigh &&
          keepNopLow <= src && keepNopHigh > src;
        TCA target = nullptr;
        TCA dest = destBlock.frontier();
        destBlock.bytes(di.size(), src);
        DecodedInstruction d2(dest);
        if (di.hasPicOffset()) {
          if (di.isBranch(false)) {
            target = di.picAddress();
          }
          /*
           * Rip-relative offsets that point outside the range
           * being moved need to be adjusted so they continue
           * to point at the right thing
           */
          if (size_t(di.picAddress() - start) >= range) {
            bool DEBUG_ONLY success = d2.setPicAddress(di.picAddress());
            assert(success);
          } else {
            if (!preserveAlignment && d2.isBranch()) {
              if (wideJmps.count(src)) {
                if (d2.size() < kJmpLen) {
                  d2.widenBranch();
                  internalRefsNeedUpdating = true;
                }
              } else if (d2.shrinkBranch()) {
                internalRefsNeedUpdating = true;
              }
            }
            hasInternalRefs = true;
          }
        }
        if (di.hasImmediate()) {
          if (fixups.m_addressImmediates.count(src)) {
            if (size_t(di.immediate() - (uint64_t)start) < range) {
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
            if (size_t(di.immediate() - (uint64_t)start) < range) {
              FTRACE(3,
                     "relocate: instruction at {} has immediate 0x{:x}"
                     "which looks like an address that needs relocating\n",
                     src, di.immediate());
            }
          }
        }

        if (src == start) {
          // for the start of the range, we only want to overwrite the "after"
          // address (since the "before" address could belong to the previous
          // tracelet, which could be being relocated to a completely different
          // address. recordRange will do that for us, so just make sure we
          // have the right address setup.
          destStart = dest;
        } else {
          rel.recordAddress(src, dest - destRange, destRange);
        }
        if (preserveAlignment && di.size() == kJmpLen &&
            di.isNop() && src + kJmpLen == end) {
          smashJmp(dest, src + kJmpLen);
          dest += kJmpLen;
        } else if (di.isNop() && !preserveAlignment) {
          internalRefsNeedUpdating = true;
        } else {
          dest += d2.size();
        }
        jmpDest = target;
        assert(dest <= destBlock.frontier());
        destBlock.setFrontier(dest);
        src += di.size();
        if (keepNopHigh && src >= keepNopHigh) {
          keepNopLow = keepNopHigh = nullptr;
        }
      }

      if (exitAddr) {
        *exitAddr = jmpDest;
      }

      rel.recordRange(start, end, destStart, destBlock.frontier());

      if (hasInternalRefs && internalRefsNeedUpdating) {
        src = start;
        bool ok = true;
        while (src != end) {
          DecodedInstruction di(src);
          TCA newPicAddress = nullptr;
          int64_t newImmediate = 0;
          if (di.hasPicOffset() &&
              size_t(di.picAddress() - start) < range) {
            newPicAddress = rel.adjustedAddressAfter(di.picAddress());
            always_assert(newPicAddress);
          }
          if (di.hasImmediate() &&
              size_t((TCA)di.immediate() - start) < range &&
              fixups.m_addressImmediates.count(src)) {
            newImmediate =
              (int64_t)rel.adjustedAddressAfter((TCA)di.immediate());
            always_assert(newImmediate);
          }
          if (newImmediate || newPicAddress) {
            TCA dest = rel.adjustedAddressAfter(src);
            DecodedInstruction d2(dest);
            if (newPicAddress) {
              if (!d2.setPicAddress(newPicAddress)) {
                always_assert(d2.isBranch() && d2.size() == 2);
                wideJmps.insert(src);
                ok = false;
              }
            }
            if (newImmediate) {
              if (!d2.setImmediate(newImmediate)) {
                always_assert(false);
              }
            }
          }
          src += di.size();
        }
        if (!ok) {
          throw JmpOutOfRange();
        }
      }
      rel.markAddressImmediates(fixups.m_addressImmediates);
    } catch (...) {
      rel.rewind(start, end);
      destBlock.setFrontier(destStart);
      throw;
    }
    return asm_count;
  }

  template <typename T>
  void fixupStateVector(StateVector<T, TcaRange>& sv, RelocationInfo& rel) {
    for (auto& ii : sv) {
      if (!ii.empty()) {
        /*
         * We have to be careful with before/after here.
         * If we relocate two consecutive regions of memory,
         * but relocate them to two different destinations, then
         * the end address of the first region is also the start
         * address of the second region; so adjustedAddressBefore(end)
         * gives us the relocated address of the end of the first
         * region, while adjustedAddressAfter(end) gives us the
         * relocated address of the start of the second region.
         */
        auto s = rel.adjustedAddressAfter(ii.begin());
        auto e = rel.adjustedAddressBefore(ii.end());
        if (e || s) {
          if (!s) s = ii.begin();
          if (!e) e = ii.end();
          ii = TcaRange(s, e);
        }
      }
    }
  }

  void adjustForRelocation(RelocationInfo& rel) override {
    for (const auto& range : rel.srcRanges()) {
      adjustForRelocation(rel, range.first, range.second);
    }
  }

  void adjustForRelocation(RelocationInfo& rel,
                           TCA srcStart, TCA srcEnd) override {
    auto start = rel.adjustedAddressAfter(srcStart);
    auto end = rel.adjustedAddressBefore(srcEnd);
    if (!start) {
      start = srcStart;
      end = srcEnd;
    } else {
      always_assert(end);
    }
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
          if (rel.isAddressImmediate(start)) {
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

      if (start == end && di.isNop() &&
          di.size() == kJmpLen &&
          rel.adjustedAddressAfter(srcEnd)) {

        smashJmp(start - di.size(), rel.adjustedAddressAfter(end));
      }
    }
  }

  /*
   * Adjusts the addresses in asmInfo and fixups to match the new
   * location of the code.
   * This will not "hook up" the relocated code in any way, so is safe
   * to call before the relocated code is ready to run.
   */
  void adjustMetaDataForRelocation(RelocationInfo& rel,
                                   AsmInfo* asmInfo,
                                   CodeGenFixups& fixups) override {
    auto& ip = fixups.m_inProgressTailJumps;
    for (size_t i = 0; i < ip.size(); ++i) {
      IncomingBranch& ib = const_cast<IncomingBranch&>(ip[i]);
      TCA adjusted = rel.adjustedAddressAfter(ib.toSmash());
      always_assert(adjusted);
      ib.adjust(adjusted);
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
       * Similar to fixups - this is a return address so get
       * the address returned to.
       */
      if (CTCA adjusted = rel.adjustedAddressBefore(ct.first)) {
        ct.first = adjusted;
      }
      /*
       * But the target is an instruction, so skip over any nops
       * that might have been inserted (eg for alignment).
       */
      if (TCA adjusted = rel.adjustedAddressAfter(ct.second)) {
        ct.second = adjusted;
      }
    }

    for (auto& jt : fixups.m_pendingJmpTransIDs) {
      if (TCA adjusted = rel.adjustedAddressAfter(jt.first)) {
        jt.first = adjusted;
      }
    }

    /*
     * Most of the time we want to adjust to a corresponding "before" address
     * with the exception of the start of the range where "before" can point to
     * the end of a previous range.
     */
    if (!fixups.m_bcMap.empty()) {
      auto const aStart = fixups.m_bcMap[0].aStart;
      auto const acoldStart = fixups.m_bcMap[0].acoldStart;
      auto const afrozenStart = fixups.m_bcMap[0].afrozenStart;
      for (auto& tbc : fixups.m_bcMap) {
        if (TCA adjusted = (tbc.aStart == aStart
                              ? rel.adjustedAddressAfter(aStart)
                              : rel.adjustedAddressBefore(tbc.aStart))) {
          tbc.aStart = adjusted;
        }
        if (TCA adjusted = (tbc.acoldStart == acoldStart
                              ? rel.adjustedAddressAfter(acoldStart)
                              : rel.adjustedAddressBefore(tbc.acoldStart))) {
          tbc.acoldStart = adjusted;
        }
        if (TCA adjusted = (tbc.afrozenStart == afrozenStart
                              ? rel.adjustedAddressAfter(afrozenStart)
                              : rel.adjustedAddressBefore(tbc.afrozenStart))) {
          tbc.afrozenStart = adjusted;
        }
      }
    }

    decltype(fixups.m_addressImmediates) updatedAI;
    for (auto addrImm : fixups.m_addressImmediates) {
      if (TCA adjusted = rel.adjustedAddressAfter(addrImm)) {
        updatedAI.insert(adjusted);
      } else if (TCA odd = rel.adjustedAddressAfter((TCA)~uintptr_t(addrImm))) {
        // just for cgLdObjMethod
        updatedAI.insert((TCA)~uintptr_t(odd));
      } else {
        updatedAI.insert(addrImm);
      }
    }
    updatedAI.swap(fixups.m_addressImmediates);

    decltype(fixups.m_alignFixups) updatedAF;
    for (auto af : fixups.m_alignFixups) {
      if (TCA adjusted = rel.adjustedAddressAfter(af.first)) {
        updatedAF.emplace(adjusted, af.second);
      } else {
        updatedAF.emplace(af);
      }
    }
    updatedAF.swap(fixups.m_alignFixups);

    if (asmInfo) {
      fixupStateVector(asmInfo->asmInstRanges, rel);
      fixupStateVector(asmInfo->asmBlockRanges, rel);
      fixupStateVector(asmInfo->coldInstRanges, rel);
      fixupStateVector(asmInfo->coldBlockRanges, rel);
      fixupStateVector(asmInfo->frozenInstRanges, rel);
      fixupStateVector(asmInfo->frozenBlockRanges, rel);
    }
  }

  void adjustCodeForRelocation(RelocationInfo& rel,
                               CodeGenFixups& fixups) override {
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

    for (auto codePtr : fixups.m_codePointers) {
      if (TCA adjusted = rel.adjustedAddressAfter(*codePtr)) {
        *codePtr = adjusted;
      }
    }
  }

 private:
  void smashJmpOrCall(TCA addr, TCA dest, bool isCall) {
    // Unconditional rip-relative jmps can also be encoded with an EB as the
    // first byte, but that means the delta is 1 byte, and we shouldn't be
    // encoding smashable jumps that way.
    assert(kJmpLen == kCallLen);
    always_assert(isSmashable(addr, x64::kJmpLen));

    auto& cb = mcg->code.blockFor(addr);
    CodeCursor cursor { cb, addr };
    X64Assembler a { cb };
    if (dest > addr && dest - addr <= x64::kJmpLen) {
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
    assert(isSmashable(jccAddr, x64::kJmpccLen));

    // Can't use the assembler to write out a new instruction, because we have
    // to preserve the condition code.
    auto newDelta = safe_cast<int32_t>(newDest - jccAddr - x64::kJmpccLen);
    auto deltaAddr = reinterpret_cast<int32_t*>(jccAddr
                                                + x64::kJmpccLen
                                                - x64::kJmpImmBytes);
    *deltaAddr = newDelta;
  }

  void emitSmashableJump(CodeBlock& cb, TCA dest, ConditionCode cc) override {
    X64Assembler a { cb };
    if (cc == CC_None) {
      assert(isSmashable(cb.frontier(), x64::kJmpLen));
      a.  jmp(dest);
    } else {
      assert(isSmashable(cb.frontier(), x64::kJmpccLen));
      a.  jcc(cc, dest);
    }
  }

  TCA smashableCallFromReturn(TCA retAddr) override {
    auto addr = retAddr - x64::kCallLen;
    assert(isSmashable(addr, x64::kCallLen));
    return addr;
  }

  void emitSmashableCall(CodeBlock& cb, TCA dest) override {
    X64Assembler a { cb };
    assert(isSmashable(cb.frontier(), x64::kCallLen));
    a.  call(dest);
  }

  TCA jmpTarget(TCA jmp) override {
    if (jmp[0] != 0xe9) {
      if (jmp[0] == 0x0f &&
          jmp[1] == 0x1f &&
          jmp[2] == 0x44) {
        // 5 byte nop
        return jmp + 5;
      }
      return nullptr;
    }
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

  void streamPhysReg(std::ostream& os, PhysReg reg) override {
    auto name = (reg.type() == PhysReg::GP) ? reg::regname(Reg64(reg)) :
      (reg.type() == PhysReg::SIMD) ? reg::regname(RegXMM(reg)) :
      /* (reg.type() == PhysReg::SF) ? */ reg::regname(RegSF(reg));
    os << name;
  }

  void disasmRange(std::ostream& os, int indent, bool dumpIR, TCA begin,
                   TCA end) override {
    Disasm disasm(Disasm::Options().indent(indent + 4)
                  .printEncoding(dumpIR)
                  .color(color(ANSI_COLOR_BROWN)));
    disasm.disasm(os, begin, end);
  }

  void genCodeImpl(IRUnit& unit, AsmInfo*) override;
};

std::unique_ptr<jit::BackEnd> newBackEnd() {
  return folly::make_unique<BackEnd>();
}

static size_t genBlock(CodegenState& state, Vout& v, Vout& vc, Block* block) {
  FTRACE(6, "genBlock: {}\n", block->id());
  CodeGenerator cg(state, v, vc);
  size_t hhir_count{0};
  for (IRInstruction& inst : *block) {
    hhir_count++;
    if (inst.is(EndGuards)) state.pastGuards = true;
    v.setOrigin(&inst);
    vc.setOrigin(&inst);
    cg.cgInst(&inst);
  }
  return hhir_count;
}

auto const vasm_gp = x64::abi.gpUnreserved | RegSet(rAsm).add(r11);
auto const vasm_simd = x64::kXMMRegs;
UNUSED const Abi vasm_abi {
  .gpUnreserved = vasm_gp,
  .gpReserved = x64::abi.gp() - vasm_gp,
  .simdUnreserved = vasm_simd,
  .simdReserved = x64::abi.simd() - vasm_simd,
  .calleeSaved = x64::kCalleeSaved,
  .sf = x64::abi.sf
};

void BackEnd::genCodeImpl(IRUnit& unit, AsmInfo* asmInfo) {
  Timer _t(Timer::codeGen);
  CodeBlock& mainCodeIn   = mcg->code.main();
  CodeBlock& coldCodeIn   = mcg->code.cold();
  CodeBlock* frozenCode   = &mcg->code.frozen();

  CodeBlock mainCode;
  CodeBlock coldCode;
  const bool useLLVM = mcg->useLLVM();
  bool relocate = false;
  if (!useLLVM &&
      RuntimeOption::EvalJitRelocationSize &&
      supportsRelocation() &&
      coldCodeIn.canEmit(RuntimeOption::EvalJitRelocationSize * 3)) {
    /*
     * This is mainly to exercise the relocator, and ensure that its
     * not broken by new non-relocatable code. Later, it will be
     * used to do some peephole optimizations, such as reducing branch
     * sizes.
     * Allocate enough space that the relocated cold code doesn't
     * overlap the emitted cold code.
     */

    static unsigned seed = 42;
    auto off = rand_r(&seed) & (cacheLineSize() - 1);
    coldCode.init(coldCodeIn.frontier() +
                   RuntimeOption::EvalJitRelocationSize + off,
                   RuntimeOption::EvalJitRelocationSize - off, "cgRelocCold");

    mainCode.init(coldCode.frontier() +
                  RuntimeOption::EvalJitRelocationSize + off,
                  RuntimeOption::EvalJitRelocationSize - off, "cgRelocMain");

    relocate = true;
  } else {
    /*
     * Use separate code blocks, so that attempts to use the mcg's
     * code blocks directly will fail (eg by overwriting the same
     * memory being written through these locals).
     */
    coldCode.init(coldCodeIn.frontier(), coldCodeIn.available(),
                  coldCodeIn.name().c_str());
    mainCode.init(mainCodeIn.frontier(), mainCodeIn.available(),
                  mainCodeIn.name().c_str());
  }

  if (frozenCode == &coldCodeIn) {
    frozenCode = &coldCode;
  }

  auto frozenStart = frozenCode->frontier();
  auto coldStart DEBUG_ONLY = coldCodeIn.frontier();
  auto mainStart DEBUG_ONLY = mainCodeIn.frontier();
  size_t hhir_count{0};

  {
    mcg->code.lock();
    mcg->cgFixups().setBlocks(&mainCode, &coldCode, frozenCode);

    SCOPE_EXIT {
      mcg->cgFixups().setBlocks(nullptr, nullptr, nullptr);
      mcg->code.unlock();
    };

    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      emitTraceCall(mainCode, unit.bcOff());
    }

    CodegenState state(unit, asmInfo, *frozenCode);
    auto const blocks = rpoSortCfg(unit);
    Vasm vasm;
    auto& vunit = vasm.unit();
    // create the initial set of vasm numbered the same as hhir blocks.
    for (uint32_t i = 0, n = unit.numBlocks(); i < n; ++i) {
      state.labels[i] = vunit.makeBlock(AreaIndex::Main);
    }
    // create vregs for all relevant SSATmps
    assignRegs(unit, vunit, state, blocks, this);
    vunit.entry = state.labels[unit.entry()];
    vasm.main(mainCode);
    vasm.cold(coldCode);
    vasm.frozen(*frozenCode);
    for (auto block : blocks) {
      auto& v = block->hint() == Block::Hint::Unlikely ? vasm.cold() :
               block->hint() == Block::Hint::Unused ? vasm.frozen() :
               vasm.main();
      FTRACE(6, "genBlock {} on {}\n", block->id(),
             area_names[(unsigned)v.area()]);
      auto b = state.labels[block];
      vunit.blocks[b].area = v.area();
      v.use(b);
      hhir_count += genBlock(state, v, vasm.cold(), block);
      assert(v.closed());
      assert(vasm.main().empty() || vasm.main().closed());
      assert(vasm.cold().empty() || vasm.cold().closed());
      assert(vasm.frozen().empty() || vasm.frozen().closed());
    }
    printUnit(kInitialVasmLevel, "after initial vasm generation", vunit);
    assert(check(vunit));

    if (useLLVM) {
      try {
        genCodeLLVM(vunit, vasm.areas(), sortBlocks(vunit));
      } catch (const FailedLLVMCodeGen& e) {
        FTRACE(1, "LLVM codegen failed ({}); falling back to x64 backend\n",
               e.what());
        vasm.finishX64(vasm_abi, state.asmInfo);
      }
    } else {
      vasm.finishX64(vasm_abi, state.asmInfo);
    }
  }

  auto bcMap = &mcg->cgFixups().m_bcMap;
  if (relocate && !bcMap->empty()) {
    TRACE(1, "BCMAPS before relocation\n");
    for (UNUSED auto& map : *bcMap) {
      TRACE(1, "%s %-6d %p %p %p\n", map.md5.toString().c_str(),
             map.bcStart, map.aStart, map.acoldStart, map.afrozenStart);
    }
  }

  assert(coldCodeIn.frontier() == coldStart);
  assert(mainCodeIn.frontier() == mainStart);

  if (relocate) {
    if (asmInfo) {
      printUnit(kRelocationLevel, unit, " before relocation ", asmInfo);
    }

    auto& be = mcg->backEnd();
    RelocationInfo rel;
    size_t asm_count{0};
    asm_count += be.relocate(rel, mainCodeIn,
                             mainCode.base(), mainCode.frontier(),
                             mcg->cgFixups(), nullptr);

    asm_count += be.relocate(rel, coldCodeIn,
                             coldCode.base(), coldCode.frontier(),
                             mcg->cgFixups(), nullptr);
    TRACE(1, "hhir-inst-count %ld asm %ld\n", hhir_count, asm_count);

    if (frozenCode != &coldCode) {
      rel.recordRange(frozenStart, frozenCode->frontier(),
                      frozenStart, frozenCode->frontier());
    }
    be.adjustForRelocation(rel);
    be.adjustMetaDataForRelocation(rel, asmInfo, mcg->cgFixups());
    be.adjustCodeForRelocation(rel, mcg->cgFixups());

    if (asmInfo) {
      static int64_t mainDeltaTot = 0, coldDeltaTot = 0;
      int64_t mainDelta =
        (mainCodeIn.frontier() - mainStart) -
        (mainCode.frontier() - mainCode.base());
      int64_t coldDelta =
        (coldCodeIn.frontier() - coldStart) -
        (coldCode.frontier() - coldCode.base());

      mainDeltaTot += mainDelta;
      HPHP::Trace::traceRelease("main delta after relocation: "
                                "%" PRId64 " (%" PRId64 ")\n",
                                mainDelta, mainDeltaTot);
      coldDeltaTot += coldDelta;
      HPHP::Trace::traceRelease("cold delta after relocation: "
                                "%" PRId64 " (%" PRId64 ")\n",
                                coldDelta, coldDeltaTot);
    }
#ifndef NDEBUG
    auto& ip = mcg->cgFixups().m_inProgressTailJumps;
    for (size_t i = 0; i < ip.size(); ++i) {
      const auto& ib = ip[i];
      assert(!mainCode.contains(ib.toSmash()));
      assert(!coldCode.contains(ib.toSmash()));
    }
    memset(mainCode.base(), 0xcc, mainCode.frontier() - mainCode.base());
    memset(coldCode.base(), 0xcc, coldCode.frontier() - coldCode.base());
#endif
  } else {
    coldCodeIn.skip(coldCode.frontier() - coldCodeIn.frontier());
    mainCodeIn.skip(mainCode.frontier() - mainCodeIn.frontier());
  }

  if (asmInfo) {
    printUnit(kCodeGenLevel, unit, " after code gen ", asmInfo);
  }
}

}}}
