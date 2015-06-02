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
#include "hphp/runtime/vm/jit/relocation.h"

#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/asm-info.h"

namespace HPHP { namespace jit { namespace x64 {

namespace {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

using WideJmpSet = hphp_hash_set<void*>;
struct JmpOutOfRange : std::exception {};

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
      assertx(src < end);
      DecodedInstruction di(src);
      asm_count++;

      int destRange = 0;
      auto af = fixups.m_alignFixups.equal_range(src);
      while (af.first != af.second) {
        auto low = src + af.first->second.second;
        auto hi = src + af.first->second.first;
        assertx(low < hi);
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
          assertx(success);
        } else {
          if (!preserveAlignment && d2.isBranch()) {
            if (wideJmps.count(src)) {
              if (d2.size() < kJmpLen) {
                d2.widenBranch();
                internalRefsNeedUpdating = true;
                // widening a branch makes the dest instruction bigger
                destBlock.setFrontier(dest + d2.size());
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
            assertx(success);
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
      assertx(dest <= destBlock.frontier());
      destBlock.setFrontier(dest);
      src += di.size();
      if (keepNopHigh && src >= keepNopHigh) {
        keepNopLow = keepNopHigh = nullptr;
      }
    } // while (src != end)

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

//////////////////////////////////////////////////////////////////////

}

/*
 * This should be called after calling relocate on all relevant ranges. It
 * will adjust all references into the original src ranges to point into the
 * corresponding relocated ranges.
 */
void adjustForRelocation(RelocationInfo& rel) {
  for (const auto& range : rel.srcRanges()) {
    adjustForRelocation(rel, range.first, range.second);
  }
}

/*
 * This will update a single range that was not relocated, but that
 * might refer to relocated code (such as the cold code corresponding
 * to a tracelet). Unless its guaranteed to be all position independent,
 * its "fixups" should have been passed into a relocate call earlier.
 */
void adjustForRelocation(RelocationInfo& rel, TCA srcStart, TCA srcEnd) {
  auto start = rel.adjustedAddressAfter(srcStart);
  auto end = rel.adjustedAddressBefore(srcEnd);
  if (!start) {
    start = srcStart;
    end = srcEnd;
  } else {
    always_assert(end);
  }
  while (start != end) {
    assertx(start < end);
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
                                 CodeGenFixups& fixups) {
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

  if (!fixups.m_bcMap.empty()) {
    /*
     * Most of the time we want to adjust to a corresponding "before" address
     * with the exception of the start of the range where "before" can point to
     * the end of a previous range.
     */
    auto const aStart = fixups.m_bcMap[0].aStart;
    auto const acoldStart = fixups.m_bcMap[0].acoldStart;
    auto const afrozenStart = fixups.m_bcMap[0].afrozenStart;
    auto adjustAddress = [&](TCA& address, TCA blockStart) {
      if (TCA adjusted = (address == blockStart
                            ? rel.adjustedAddressAfter(blockStart)
                            : rel.adjustedAddressBefore(address))) {
        address = adjusted;
      }
    };
    for (auto& tbc : fixups.m_bcMap) {
      adjustAddress(tbc.aStart, aStart);
      adjustAddress(tbc.acoldStart, acoldStart);
      adjustAddress(tbc.afrozenStart, afrozenStart);
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

  for (auto& af : fixups.m_reusedStubs) {
    if (TCA adjusted = rel.adjustedAddressAfter(af)) {
      af = adjusted;
    }
  }

  if (asmInfo) {
    fixupStateVector(asmInfo->asmInstRanges, rel);
    fixupStateVector(asmInfo->asmBlockRanges, rel);
    fixupStateVector(asmInfo->coldInstRanges, rel);
    fixupStateVector(asmInfo->coldBlockRanges, rel);
    fixupStateVector(asmInfo->frozenInstRanges, rel);
    fixupStateVector(asmInfo->frozenBlockRanges, rel);
  }
}

/*
 * Adjust potentially live references that point into the relocated
 * area.
 * Must not be called until its safe to run the relocated code.
 */
void adjustCodeForRelocation(RelocationInfo& rel, CodeGenFixups& fixups) {
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

void findFixups(TCA start, TCA end, CodeGenFixups& fixups) {
  while (start != end) {
    assert(start < end);
    DecodedInstruction di(start);
    start += di.size();

    if (di.isCall()) {
      if (auto fixup = mcg->fixupMap().findFixup(start)) {
        fixups.m_pendingFixups.push_back(PendingFixup(start, *fixup));
      }
      if (auto ct = mcg->catchTraceMap().find(start)) {
        fixups.m_pendingCatchTraces.emplace_back(start, *ct);
      }
    }
  }
}


/*
 * Relocate code in the range start, end into dest, and record
 * information about what was done to rel.
 * On exit, internal references (references into the source range)
 * will have been adjusted (ie they are still references into the
 * relocated code). External code references continue to point to
 * the same address as before relocation.
 */
size_t relocate(RelocationInfo& rel,
                CodeBlock& destBlock,
                TCA start, TCA end,
                CodeGenFixups& fixups,
                TCA* exitAddr) {
  WideJmpSet wideJmps;
  while (true) {
    try {
      return relocateImpl(rel, destBlock, start, end,
                          fixups, exitAddr, wideJmps);
    } catch (JmpOutOfRange& j) {
    }
  }
}

//////////////////////////////////////////////////////////////////////

}}}
