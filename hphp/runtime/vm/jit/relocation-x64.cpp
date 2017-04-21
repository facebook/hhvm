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

#include "hphp/runtime/vm/jit/relocation.h"

#include "hphp/runtime/vm/jit/align-x64.h"
#include "hphp/runtime/vm/jit/asm-info.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"

namespace HPHP { namespace jit { namespace x64 {

namespace {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

constexpr int kJmpLen = 5;

using WideJmpSet = hphp_hash_set<void*>;
struct JmpOutOfRange : std::exception {};

size_t relocateImpl(RelocationInfo& rel,
                    CodeBlock& destBlock,
                    TCA start, TCA end,
                    DataBlock& srcBlock,
                    CGMeta& fixups,
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
      DecodedInstruction di(srcBlock.toDestAddress(src), src);
      asm_count++;

      int destRange = 0;
      auto af = fixups.alignments.equal_range(src);
      while (af.first != af.second) {
        auto const alignPair = af.first->second;
        auto const alignInfo = alignment_info(alignPair.first);

        auto const low = src + alignInfo.offset;
        auto const hi = src + alignInfo.nbytes;
        assertx(low < hi);

        if (!keepNopLow || keepNopLow > low) keepNopLow = low;
        if (!keepNopHigh || keepNopHigh < hi) keepNopHigh = hi;

        TCA tmp = destBlock.frontier();
        align(destBlock, nullptr, alignPair.first, alignPair.second);

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
      destBlock.bytes(di.size(), srcBlock.toDestAddress(src));
      DecodedInstruction d2(destBlock.toDestAddress(dest), dest);
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
        if (fixups.addressImmediates.count(src)) {
          if (size_t(di.immediate() - (uint64_t)start) < range) {
            hasInternalRefs = internalRefsNeedUpdating = true;
          }
        } else {
          if (fixups.addressImmediates.count((TCA)~uintptr_t(src))) {
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
           * fixups.addressImmediates. But it could just happen by bad
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
        DecodedInstruction di(srcBlock.toDestAddress(src), src);
        TCA newPicAddress = nullptr;
        int64_t newImmediate = 0;
        if (di.hasPicOffset() &&
            size_t(di.picAddress() - start) < range) {
          newPicAddress = rel.adjustedAddressAfter(di.picAddress());
          always_assert(newPicAddress);
        }
        if (di.hasImmediate() &&
            size_t((TCA)di.immediate() - start) < range &&
            fixups.addressImmediates.count(src)) {
          newImmediate =
            (int64_t)rel.adjustedAddressAfter((TCA)di.immediate());
          always_assert(newImmediate);
        }
        if (newImmediate || newPicAddress) {
          TCA dest = rel.adjustedAddressAfter(src);
          DecodedInstruction d2(destBlock.toDestAddress(dest), dest);
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
    rel.markAddressImmediates(fixups.addressImmediates);
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

void adjustMetaDataForRelocation(RelocationInfo& rel,
                                 AsmInfo* asmInfo,
                                 CGMeta& meta) {
  for (auto& li : meta.literals) {
    if (auto adjusted = rel.adjustedAddressAfter((TCA)li.second)) {
      li.second = (uint64_t*)adjusted;
    }
  }

  decltype(meta.codePointers) updatedCP;
  for (auto cp : meta.codePointers) {
    if (auto adjusted = (TCA*)rel.adjustedAddressAfter((TCA)cp)) {
      updatedCP.emplace(adjusted);
    } else {
      updatedCP.emplace(cp);
    }
  }
  updatedCP.swap(meta.codePointers);
}

/*
 * Adjust potentially live references that point into the relocated
 * area.
 * Must not be called until its safe to run the relocated code.
 */
void adjustCodeForRelocation(RelocationInfo& rel, CGMeta& fixups) {
  for (auto addr : fixups.reusedStubs) {
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

  for (auto codePtr : fixups.codePointers) {
    if (TCA adjusted = rel.adjustedAddressAfter(*codePtr)) {
      *codePtr = adjusted;
    }
  }
}

void findFixups(TCA start, TCA end, CGMeta& meta) {
  while (start != end) {
    assert(start < end);
    DecodedInstruction di(start);
    start += di.size();

    if (di.isCall()) {
      if (auto fixup = FixupMap::findFixup(start)) {
        meta.fixups.emplace_back(start, *fixup);
      }
      if (auto ct = getCatchTrace(start)) {
        meta.catches.emplace_back(start, *ct);
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
                DataBlock& srcBlock,
                CGMeta& fixups,
                TCA* exitAddr) {
  WideJmpSet wideJmps;
  while (true) {
    try {
      return relocateImpl(rel, destBlock, start, end, srcBlock,
                          fixups, exitAddr, wideJmps);
    } catch (JmpOutOfRange& j) {
    }
  }
}

//////////////////////////////////////////////////////////////////////

}}}
