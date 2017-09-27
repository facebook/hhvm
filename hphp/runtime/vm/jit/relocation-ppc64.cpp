/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2016                                   |
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

#include "hphp/runtime/vm/jit/relocation-ppc64.h"

#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/vm/jit/align-ppc64.h"
#include "hphp/runtime/vm/jit/asm-info.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"

#include "hphp/ppc64-asm/asm-ppc64.h"
#include "hphp/ppc64-asm/decoded-instr-ppc64.h"

namespace HPHP { namespace jit { namespace ppc64 {

using ppc64_asm::DecodedInstruction;

namespace {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

using JmpSet = hphp_hash_set<void*>;
struct JmpOutOfRange : std::exception {};

size_t relocateImpl(RelocationInfo& rel,
                    CodeBlock& dest_block,
                    TCA start, TCA end,
                    CodeBlock& src_block,
                    CGMeta& fixups,
                    TCA* exit_addr,
                    JmpSet& far_jmps,
                    JmpSet& near_jmps) {
  TCA src = start;
  size_t range = end - src;
  bool internal_refs_need_update = false;
  TCA dest_start = dest_block.frontier();
  size_t asm_count{0};
  TCA jmp_dest = nullptr;
  try {
    while (src != end) {
      assertx(src < end);
      DecodedInstruction di(src_block.toDestAddress(src), src);
      asm_count++;

      TCA dest = dest_block.frontier();
      dest_block.bytes(di.size(), src_block.toDestAddress(src));
      DecodedInstruction d2(dest_block.toDestAddress(dest), dest, di.size());
      if (di.isNearBranch()) {
        if (di.isBranch(ppc64_asm::AllowCond::OnlyUncond)) {
          jmp_dest = di.nearBranchTarget();
        }
        // Relative branch needs always to be readjusted
        internal_refs_need_update = true;

        if (far_jmps.count(src)) {
          TCA old_target = di.nearBranchTarget();
          TCA adjusted_target = rel.adjustedAddressAfter(old_target);
          TCA new_target = (adjusted_target) ? adjusted_target : old_target;

          // Near branch will be widen to Far branch. Update d2 in order to be
          // able to read more bytes than only the Near branch
          d2 = DecodedInstruction(dest_block.toDestAddress(dest), dest);
          d2.widenBranch(new_target);

          // widening a branch makes the dest instruction bigger
          dest_block.setFrontier(dest + d2.size());
        } else if ((size_t(di.nearBranchTarget() - start) >= range) &&
            /*
             * Rip-relative offsets that point outside the range
             * being moved need to be adjusted so they continue
             * to point at the right thing
             */
            !d2.setNearBranchTarget(di.nearBranchTarget())) {
          FTRACE(3,
              "relocate: instruction at {} has target 0x{:x} "
              "which is too far away for a near branch after relocation\n",
              dest, di.nearBranchTarget());
        }
      }
      if (di.isImmediate()) {
        auto imm = di.immediate();
        if (fixups.addressImmediates.count((TCA)imm)) {
          auto new_immediate = (int64_t)rel.adjustedAddressAfter((TCA)imm);
          if (!d2.setImmediate(new_immediate)) {
            always_assert(false && "Immediate couldn't be relocated");
          }
        } else if (fixups.addressImmediates.count(src)) {
          if (size_t(imm - (uint64_t)start) < range) {
            internal_refs_need_update = true;
          }
        } else {
          if (fixups.addressImmediates.count((TCA)~uintptr_t(src))) {
            // Handle weird, encoded offset, used by cgLdObjMethod
            always_assert(imm == ((uintptr_t(src) << 1) | 1));
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
          if (size_t(imm - (uint64_t)start) < range) {
            FTRACE(3,
                   "relocate: instruction at {} has immediate 0x{:x}"
                   "which looks like an address that needs relocating\n",
                   src, imm);
          }
        }
      }
      if (di.isFarBranch()) {
        if (near_jmps.count(src)) {
          // Shrink it to a Near branch
          if (d2.shrinkBranch()) {
            internal_refs_need_update = true;
          } else {
            // It asked to be patched but it couldn't, then something is odd...
            assertx(false && "Couldn't change Far -> Near branch");
          }
        } else if ((size_t(di.farBranchTarget() - (uint64_t)start) < range) ||
            (di.couldBeNearBranch())) {
          // It can be done with a Near branch
          internal_refs_need_update = true;
        } else {
          // re-emit it without nops, except if it's a smashable
          TCA old_target = di.farBranchTarget();
          TCA adjusted_target = rel.adjustedAddressAfter(old_target);
          TCA new_target = (adjusted_target) ? adjusted_target : old_target;

          bool keep_nops = true;
          if (RuntimeOption::EvalPPC64RelocationRemoveFarBranchesNops) {
            // if it's smashable, don't remove nops (leave it as fixed size) and
            // mark this relocated address to be used on adjustForRelocation
            keep_nops = (0 != fixups.smashableLocations.count(src));
            if (keep_nops) rel.markSmashableRelocation(dest);
          }
          if (!d2.setFarBranchTarget(new_target, keep_nops)) {
            assertx(false && "Couldn't set Far branch target");
          }
        }
      }

      if (src == start) {
        // for the start of the range, we only want to overwrite the "after"
        // address (since the "before" address could belong to the previous
        // tracelet, which could be being relocated to a completely different
        // address. recordRange will do that for us, so just make sure we
        // have the right address setup.
        dest_start = dest;
      } else {
        rel.recordAddress(src, dest, 0);
      }
      dest += d2.size();
      assertx(dest <= dest_block.frontier());
      dest_block.setFrontier(dest);
      src += di.size();
    } // while (src != end)

    if (exit_addr) {
      *exit_addr = jmp_dest;
    }

    rel.recordRange(start, end, dest_start, dest_block.frontier());

    if (internal_refs_need_update) {
      src = start;
      bool ok = true;
      while (src != end) {
        DecodedInstruction di(src_block.toDestAddress(src), src);
        TCA dest = rel.adjustedAddressAfter(src);
        // Avoid set max_size as it would fail when a branch is widen.
        DecodedInstruction d2(dest_block.toDestAddress(dest), dest);
        if (di.isNearBranch()) {
          TCA old_target = di.nearBranchTarget();
          TCA adjusted_target = rel.adjustedAddressAfter(old_target);
          TCA new_target = (adjusted_target) ? adjusted_target : old_target;
          if (d2.isNearBranch()) {
            if (!d2.setNearBranchTarget(new_target)) {
              // It doesn't fit a Near branch anymore. Convert it to Far branch
              far_jmps.insert(src);
              ok = false;
            }
          } else {
            bool keep_nops =
              !RuntimeOption::EvalPPC64RelocationRemoveFarBranchesNops;
            if (!d2.setFarBranchTarget(new_target, keep_nops)) {
              always_assert(false && "Widen branch relocation failed");
            }
          }
        }
        if (di.isImmediate()) {
          int64_t new_immediate =
            (int64_t)rel.adjustedAddressAfter((TCA)di.immediate());
          if ((new_immediate) && !d2.setImmediate(new_immediate)) {
            always_assert(false && "Immediate couldn't be relocated");
          }
        }
        if (di.isFarBranch()) {
          bool insert_near_jmp = false;
          auto new_far_target = rel.adjustedAddressAfter(di.farBranchTarget());
          if ((size_t(di.farBranchTarget() - start) < range) ||
              (d2.isNearBranch())) {
            // A Far branch is converted to Near branch.
            if (near_jmps.count(src)) {
              // Oh, it's already a near branch! Adjust the offset
              auto target = new_far_target
                            ? new_far_target
                            : di.farBranchTarget();
              if (!d2.setNearBranchTarget(target)) {
                assertx(false && "Can't change to a near branch.");
              }
            } else {
              insert_near_jmp = true;
            }
          } else if (!new_far_target && d2.couldBeNearBranch()) {
            // target is close enough, convert it to Near branch
            insert_near_jmp = true;
          } else if (new_far_target) {
            // Far target will be relocated.
            bool keep_nops = true;
            if (RuntimeOption::EvalPPC64RelocationRemoveFarBranchesNops) {
              keep_nops = rel.isSmashableRelocation(dest);
            }
            if (!d2.setFarBranchTarget(new_far_target, keep_nops)) {
              assert(false && "Far branch target setting failed");
            }
            if (d2.couldBeNearBranch()) {
              // target is close enough, convert it to Near branch
              insert_near_jmp = true;
            }
          }
          // shrinkBranch is only allowed for non-smashable branches
          if (insert_near_jmp && !fixups.smashableLocations.count(src) &&
              RuntimeOption::EvalPPC64RelocationShrinkFarBranches) {
            // Mark it as Near branch and run this again
            near_jmps.insert(src);
            ok = false;
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
    dest_block.setFrontier(dest_start);
    throw;
  }
  return asm_count;
}

//////////////////////////////////////////////////////////////////////

}

void adjustInstruction(RelocationInfo& rel, DecodedInstruction& di) {
  if (di.isNearBranch()) {
    /*
     * A pointer into something that has been relocated needs to be
     * updated.
     */
    if (TCA adjusted = rel.adjustedAddressAfter(di.nearBranchTarget())) {
      if (!di.setNearBranchTarget(adjusted)) {
        always_assert(false &&
            "Tried to patch near branch but it doesn't fit.");
      }
    }
  } else if (di.isImmediate()) {
    /*
     * Similarly for addressImmediates - and see comment above
     * for non-address immediates.
     */
    if (TCA adjusted = rel.adjustedAddressAfter((TCA)di.immediate())) {
      if (!di.setImmediate((int64_t)adjusted)) {
        always_assert(false && "Immediate couldn't be adjusted.");
      }
    }
  } else if (di.isFarBranch()) {
    if (TCA adjusted = rel.adjustedAddressAfter(di.farBranchTarget())) {
      bool keep_nops = true;
      if (RuntimeOption::EvalPPC64RelocationRemoveFarBranchesNops) {
        keep_nops = rel.isSmashableRelocation(di.ip());
      }
      if (!di.setFarBranchTarget(adjusted, keep_nops)) {
        always_assert(false && "Not an expected Far branch.");
      }
    }
  }
}

/*
 * This should be called after calling relocate on all relevant ranges. It
 * will adjust all references into the original src ranges to point into the
 * corresponding relocated ranges.
 */
void adjustForRelocation(RelocationInfo& rel) {
  for (const auto& range : rel.srcRanges()) {
    ppc64::adjustForRelocation(rel, range.first, range.second);
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

    adjustInstruction(rel, di);

    start += di.size();

    if (start == end && di.isNop() && rel.adjustedAddressAfter(srcEnd)) {
      smashJmp(start - di.size(), rel.adjustedAddressAfter(end));
    }
  }
}

void adjustMetaDataForRelocation(RelocationInfo& rel, AsmInfo* /*asmInfo*/,
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
     * The stubs are terminated by a trap. Check for it.
     */
    for (auto di = DecodedInstruction(addr); !di.isException(); ) {
      adjustInstruction(rel, di);

      addr += di.size();
      di = DecodedInstruction(addr);
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
                CodeBlock& dest_block,
                TCA start, TCA end,
                DataBlock& src_block,
                CGMeta& fixups,
                TCA* exit_addr,
                AreaIndex) {
  JmpSet far_jmps, near_jmps;
  while (true) {
    try {
      return relocateImpl(rel, dest_block, start, end, src_block,
                          fixups, exit_addr, far_jmps, near_jmps);
    } catch (JmpOutOfRange& j) {
    }
  }
}

//////////////////////////////////////////////////////////////////////

}}}
