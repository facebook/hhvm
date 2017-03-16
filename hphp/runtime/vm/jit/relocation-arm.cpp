/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017 Qualcomm Datacenter Technologies, Inc.            |
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

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/align-arm.h"
#include "hphp/runtime/vm/jit/asm-info.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/smashable-instr-arm.h"

#include "hphp/vixl/a64/macro-assembler-a64.h"

namespace HPHP { namespace jit { namespace arm {

using namespace vixl;

namespace {

TRACE_SET_MOD(mcg);

//////////////////////////////////////////////////////////////////////

/*
 * See relocation.h for an overview. A note about adjusting for
 * relocation: for ARM, this includes the following
 *
 *   PC Relative
 *     ADR/ADRP - Builds PC relatative addresses.
 *     B[.<cc>] (immediate) - Branch to a PC relative address.
 *     LDR (literal) - Loads a literal from a PC relative address.
 *
 *   Immediates
 *     MOV/MOK - Used to build up targets 16 bits at a time.
 *     LDR (literal) - The literal loaded may be a target address.
 *
 * adjustCodeForRelocation must support updating live instructions.
 * For ARM, we can update a single instruction or a 64 bit literal
 * in the instruction stream atomically. We can't, however, update
 * multiple instructions atomically since there may be a thread
 * executing instructions within that range. Therefore, MOV/MOVK
 * instructions can't be adjusted for live ranges of instructions.
 * Any emitted code that uses MOV/MOVK and which must be adjusted
 * while live, should instead use LDR (literal). Smashables use this
 * approach; they embed a literal in the instruction stream which
 * can then be updated atomically. At the time of this writing, only
 * the vasm opcode (leap) produces a live region that must be updated,
 * and so it uses an approach with LDR (literal) instead of MOV/MOVK.
 */

//////////////////////////////////////////////////////////////////////

using InstrSet = hphp_hash_set<Instruction*>;
struct JmpOutOfRange : std::exception {};

/*
 * Identifies all of the instruction words in a range which are embedded
 * literals. This set of literals is used during relocation and adjusting to
 * indicate that they should not be analyzed as instructions.
 */
InstrSet findLiterals(Instruction* start, Instruction* end) {
  InstrSet literals;
  auto instr = start;
  while (instr != end) {
    assertx(instr < end);
    if (!literals.count(instr)) {
      if (instr->IsLoadLiteral() && instr->Mask(LoadLiteralMask) == LDR_x_lit) {
        /*
         * Get the address of the literal instruction words. Add both words.
         * Also check if these instruction words are themselves LDR literals
         * whose literals may have been accidentally added.
         */
        auto addLiteral = [&] (Instruction* lit) {
          if (lit >= start && lit < end) {
            if (!literals.count(lit)) {
              literals.insert(lit);
            }
            if (lit->IsLoadLiteral() && lit->Mask(LoadLiteralMask) == LDR_x_lit) {
              auto oops = Instruction::Cast(lit->LiteralAddress());
              literals.erase(oops);
              literals.erase(oops->NextInstruction());
            }
          }
        };

        auto la = Instruction::Cast(instr->LiteralAddress());
        addLiteral(la);
        addLiteral(la->NextInstruction());
      }
    }
    instr = instr->NextInstruction();
  }
  return literals;
}

/*
 * Returns true if the instruction sequence is a smashable jump/call and the
 * sequence is relocated as a simple PC relative branch. Otherwise return false
 * so that the individual instructions in the sequence can be relocated
 * one-by-one.
 *
 * NOTE: This helper will optimize sequences that are not smashables but which
 *       are identical to smashables. Namely the sequences emitted for
 *       jmp{} and jcc{} which use LDR literals for easy of patching at the
 *       end of the translation. It's important to note that a smashable-like
 *       sequence can be optimized during static relocation, but that an
 *       actual smashable can't be optimized until live relocation and then
 *       only if it has been smashed. Currently there is no means to know
 *       if a smashable has been smashed, and so this optimization of
 *       smashables during live relocation is not yet performed at all.
 *
 * dest_count, src_count and rewrites are updated to reflect when an
 * instruction(s) is rewritten to a different instruction sequence.
 */
bool relocateSmashableHelper(RelocationInfo& rel, CGMeta& meta,
                             CodeBlock& dest_block, TCA start, TCA end,
                             TCA src, TCA dest, TCA jmp_dest,
                             size_t& src_count, size_t& dest_count,
                             const InstrSet& far, InstrSet& rewrites,
                             bool& update_internal_refs) {

  auto instr = Instruction::Cast(src);

  /*
   * A smashable jcc looks exactly like a smashable jmp with a preceding
   * conditional branch. The checks below determine if there is a smashable
   * jmp that can be optimized. It's important to not simply analyze this
   * smashable jmp without first considering if it's part of a jcc.
   */
  auto target = smashableJmpTarget(src);
  if (target) {
    auto sl = getSmashableFromTargetAddr(src + kSmashJmpTargetOff);
    if (sl && meta.smashableLocations.count(sl)) {
      target = nullptr;
    }
  }

  assertx(((uint64_t)target & 3) == 0);

  if (target) {
    auto adjusted = rel.adjustedAddressAfter(target);
    if (!adjusted) adjusted = target;
    int imm = (adjusted - dest) >> vixl::kInstructionSizeLog2;
    if (is_int26(imm) && !far.count(instr)) {
      vixl::MacroAssembler a { dest_block };
      dest_block.setFrontier(dest);
      a.b(imm);
      src_count = smashableJmpLen() >> vixl::kInstructionSizeLog2;
      dest_count = 1;
      for (auto i = Instruction::Cast(src);
           i < Instruction::Cast(src + src_count * kInstructionSize);
           i = i->NextInstruction()) {
        rewrites.insert(i);
      }
      update_internal_refs = true;
      FTRACE(3,
             "Relocated smashable at src 0x{:08x} with target 0x{:08x} to 0x{:08x} (0x{:08x})\n",
             (uint64_t)src, (uint64_t)target, *((uint32_t*)dest), (uint64_t)dest);

      return true;
    }
  }

  /*
   * Smashable calls are not yet supported, because we don't have any
   * reloc debug info logged when smashing a call.
   */

  return false;
}

/*
 * Returns true if the source is a PC relative instruction. Relocates the
 * that instruction, adjusting the offsets in the instruction. If the new
 * offset cannot be encoded, then the instruction is changed into a multi-
 * instruction sequence, overwritting the origina PC relative instruction
 * that was initially copied. The following are the PC relative instructions:
 *   ADR/ADRP
 *   LDR (literal)
 *   B.<cc> (immediate)
 *   B (immediate)
 *
 * dest_count, src_count and rewrites are updated to reflect when an
 * instruction(s) is rewritten to a different instruction sequence.
 */
bool relocatePCRelativeHelper(CodeBlock& dest_block, TCA start, TCA end,
                              TCA src, TCA dest, TCA jmp_dest,
                              size_t& src_count, size_t& dest_count,
                              const InstrSet& far, InstrSet& rewrites,
                              bool& update_internal_refs) {

  auto instr = Instruction::Cast(src);
  auto instr2 = Instruction::Cast(dest);

  if (instr->IsPCRelAddressing() ||
      instr->IsLoadLiteral() ||
      instr->IsCondBranchImm() ||
      instr->IsUncondBranchImm()) {

    auto target = reinterpret_cast<TCA>(instr->ImmPCOffsetTarget());

    // If the target is outside of the range of this relocation,
    // then update it.
    if ((target < start) || (target >= end)) {
      /*
       * Calculate the new offset and determine if it can be encoded
       * in a PC relative instruciton or if it needs to be converted
       * to make use of an absolute target.
       * Note: Use the VIXL scratch registers when transforming. Their
       *       scope is just a single macroassembler directive, whereas
       *       the scope of rAsm is an entire vasm instruction.
       */
      int imm = instr->ImmPCOffsetTarget() - instr2;
      bool is_relative = true;
      if (instr->IsPCRelAddressing()) {
        if (!is_int21(imm) || far.count(instr)) {
          dest_block.setFrontier(dest);
          dest_count--;

          vixl::MacroAssembler a { dest_block };
          auto const dst = vixl::Register(instr->Rd(), 64);
          a.Mov(dst, instr->ImmPCOffsetTarget());

          dest_count += (dest_block.frontier() - dest) / kInstructionSize;
          is_relative = false;
        }
      } else if (instr->IsLoadLiteral()) {
        if (!is_int19(imm) || far.count(instr)) {
          dest_block.setFrontier(dest);
          dest_count--;

          vixl::MacroAssembler a { dest_block };
          a.SetScratchRegisters(vixl::NoReg, vixl::NoReg);
          auto const dst = vixl::Register(instr->Rd(), 64);
          auto const tmp = dst.Is(rVixlScratch0)
            ? rVixlScratch1 : rVixlScratch0;
          a.Mov(tmp, instr->ImmPCOffsetTarget());
          a.Ldr(dst, vixl::MemOperand(tmp));
          a.SetScratchRegisters(rVixlScratch0, rVixlScratch1);

          dest_count += (dest_block.frontier() - dest) / kInstructionSize;
          is_relative = false;
        }
      } else if (instr->IsCondBranchImm()) {
        imm >>= vixl::kInstructionSizeLog2;
        if (!is_int19(imm) || far.count(instr)) {
          dest_block.setFrontier(dest);
          dest_count--;

          vixl::MacroAssembler a { dest_block };
          vixl::Label end;
          auto const cond = static_cast<Condition>(instr->ConditionBranch());
          auto const tmp = rVixlScratch0;
          a.SetScratchRegisters(vixl::NoReg, vixl::NoReg);
          a.B(&end, vixl::InvertCondition(cond));
          a.Mov(tmp, instr->ImmPCOffsetTarget());
          a.Br(tmp);
          a.bind(&end);
          a.SetScratchRegisters(rVixlScratch0, rVixlScratch1);

          dest_count += (dest_block.frontier() - dest) / kInstructionSize;
          is_relative = false;
        }
      } else if (instr->IsUncondBranchImm()) {
        imm >>= vixl::kInstructionSizeLog2;
        if (!is_int26(imm) || far.count(instr)) {
          dest_block.setFrontier(dest);
          dest_count--;

          vixl::MacroAssembler a { dest_block };
          a.SetScratchRegisters(vixl::NoReg, vixl::NoReg);
          auto const tmp = rVixlScratch0;
          a.Mov(tmp, instr->ImmPCOffsetTarget());
          a.Br(tmp);
          a.SetScratchRegisters(rVixlScratch0, rVixlScratch1);

          dest_count += (dest_block.frontier() - dest) / kInstructionSize;
          is_relative = false;
        }
      }

      // Update offset if it was NOT converted from relative to absolute
      if (is_relative) {
        instr2->SetImmPCOffsetTarget(instr->ImmPCOffsetTarget());
      } else {
        // Otherwise it was rewritten to absolute, and this
        // internal reference must be updated later.
        rewrites.insert(instr);
        update_internal_refs = true;
      }
    }

    if (instr->IsCondBranchImm() ||
        instr->IsUncondBranchImm()) {
      jmp_dest = target;
    }
    return true;
  }

  return false;
}

/*
 * Returns true if the instruction(s) contain an immediates. The immediates
 * are adjusted as part of the relocation. If an immediate no longer fits
 * into the instruciton, then the instruction is rewritten as a longer
 * sequence. The following instructions are supported:
 *   MOV/MOVK
 *   LDR (literal)
 *
 * dest_count, src_count and rewrites are updated to reflect when an
 * instruction(s) is rewritten to a different instruction sequence.
 */
size_t relocateImmediateHelper(RelocationInfo& rel,
                               CodeBlock& dest_block, TCA end,
                               TCA src, TCA dest,
                               size_t& src_count, size_t& dest_count,
                               InstrSet &rewrites,
                               bool& update_internal_refs) {
  auto instr = Instruction::Cast(src);
  auto instr2 = Instruction::Cast(dest);

  if (instr->IsMovz()) {
    const auto rd = instr->Rd();
    uint64_t target = instr->ImmMoveWide() << (16 * instr->ShiftMoveWide());
    auto next = instr->NextInstruction();
    while (next->IsMovk()) {
      if (next->Rd() == rd) {
        target |= next->ImmMoveWide() << (16 * next->ShiftMoveWide());
      }
      next = next->NextInstruction();
    }

    auto adjusted = rel.adjustedAddressAfter((TCA)target);
    if (!adjusted) { adjusted = (TCA)target; }
    int imm = Instruction::Cast(adjusted) - instr2;
    bool is_absolute = true;

    /*
     * If the next instruction after a sequence of MOV/MOVK uses the
     * target, see if the entire sequence can be converted to a
     * single PC relative instruction. Supported transformations include:
     *   MOV/MOVK, LDR to LDR literal
     *   MOV/MOVK, BR<L> to B<L>
     *   MOV/MOVK absolute to ADR
     */
    if (next->Mask(LoadStoreUnsignedOffsetMask) == LDR_x_unsigned &&
        next->Rn() == rd && !next->ImmShiftLS()) {
      // Only transform if the MOV/MOVK sequence def'd a vixl scratch,
      // otherwise we run the risk of not def'ing a live register.
      auto const tmp = vixl::Register(rd, 64);
      if (tmp.Is(rVixlScratch0) || tmp.Is(rVixlScratch1)) {
        dest_block.setFrontier(dest);
        dest_count--;

        vixl::MacroAssembler a { dest_block };
        vixl::Label target;
        auto const dst = vixl::Register(next->Rd(), 64);
        a.Ldr(dst, &target);

        auto savedFrontier = dest_block.frontier();
        dest_block.setFrontier(adjusted);
        a.bind(&target);
        dest_block.setFrontier(savedFrontier);

        dest_count += (dest_block.frontier() - dest) / kInstructionSize;
        src_count = (next - instr) / kInstructionSize + 1;
        is_absolute = false;

        // Add range of source instructions to rewrites (MOV/MOVKs/LDR)
        for (auto i = instr;
             i < next->NextInstruction();
             i = i->NextInstruction()) {
          rewrites.insert(i);
        }
      }
    } else if (next->IsUncondBranchReg() && next->Rn() == rd) {
      imm >>= vixl::kInstructionSizeLog2;
      // Only transform if the MOV/MOVK sequence def'd a vixl scratch,
      // otherwise we run the risk of not def'ing a live register.
      auto const dst = vixl::Register(rd, 64);
      if (dst.Is(rVixlScratch0) || dst.Is(rVixlScratch1)) {
        if (is_int26(imm)) {
          dest_block.setFrontier(dest);
          dest_count--;

          vixl::MacroAssembler a { dest_block };
          if (next->Mask(UnconditionalBranchToRegisterMask) == BR) {
            a.b(imm);
          } else {
            a.bl(imm);
          }

          dest_count += (dest_block.frontier() - dest) / kInstructionSize;
          src_count = (next - instr) / kInstructionSize + 1;
          is_absolute = false;

          // Add range of source instructions to rewrites (MOV/MOVKs/B<L>)
          // Note: next points past the MOV/MOVK sequence to the B<L>. We're
          //       replacing all of them so iterate past next for the B<L>.
          for (auto i = instr;
               i < next->NextInstruction();
               i = i->NextInstruction()) {
            rewrites.insert(i);
          }
        }
      }
    } else {
      if (is_int21(imm)) {
        dest_block.setFrontier(dest);
        dest_count--;

        vixl::MacroAssembler a { dest_block };
        auto const dst = vixl::Register(rd, 64);
        a.adr(dst, imm);

        dest_count += (dest_block.frontier() - dest) / kInstructionSize;
        src_count = (next - instr) / kInstructionSize;
        is_absolute = false;

        // Add range of source instructions to rewrites (MOV/MOVKs)
        for (auto i = instr; i < next; i = i->NextInstruction()) {
          rewrites.insert(i);
        }
      }
    }
    // If we did NOT convert to a PC relative instruction sequence, then
    // see if we need to adjust the target of the mov/movk sequence now.
    if (is_absolute) {
      if (adjusted != (TCA)target) {
        dest_block.setFrontier(dest);
        dest_count--;

        vixl::MacroAssembler a { dest_block };
        auto const dst = vixl::Register(rd, 64);
        a.Mov(dst, adjusted);

        dest_count += (dest_block.frontier() - dest) / kInstructionSize;
        src_count = (next - instr) / kInstructionSize;

        // Add range of source instructions to rewrites (MOV/MOVKs)
        for (auto i = instr; i < next; i = i->NextInstruction()) {
          rewrites.insert(i);
        }
      } else if (((TCA)target >= dest_block.frontier()) || ((TCA)target < end)) {
        // Otherwise if the target wasn't adjusted because it's an addresses
        // within this translation that we have yet relocated, then mark the
        // internal ref to be updated later.
        update_internal_refs = true;
      }
    } else {
      // Otherwise we rewrote the instruction sequence which can trigger internal
      // reference to need updating later.
      update_internal_refs = true;
    }
    return true;
  }

  return false;
}

size_t relocateImpl(RelocationInfo& rel,
                    CodeBlock& dest_block,
                    TCA start, TCA end,
                    CGMeta& meta,
                    TCA* exit_addr,
                    InstrSet& far) {
  auto src = start;
  bool internal_refs_need_update = false;
  auto dest_start = dest_block.frontier();
  size_t asm_count{0};
  TCA jmp_dest = nullptr;

  /*
   * These sets track instruction words within the source sequence which should
   * be ignored during the analysis. Literals are copied and ignored even though
   * they sometimes coincedently hold valid encodings of instructions. Any
   * source instructions which are transformed should also be ignored when
   * adjusting internal refs since they have correct PC relative offsets and
   * immediate already.
   */
  try {
    // Simple relocation just copies src instructions to dest instructions.
    // If a src instruction(s) are actually rewritten, then those src
    // instructions are tracked in this list.
    InstrSet rewrites;

    // Find the literals embedded within the range
    InstrSet literals = findLiterals(Instruction::Cast(start),
                                     Instruction::Cast(end));

    // Relocate each instruction to the destination.
    while (src != end) {
      assertx(src < end);

      // Initially copy the instruction word
      auto dest = dest_block.frontier();
      dest_block.bytes(kInstructionSize, src);
      size_t src_count = 1, dest_count = 1;

      // If it's not a literal, then attempt any special relocations
      if (!literals.count(Instruction::Cast(src)) &&
          !relocateSmashableHelper(rel, meta, dest_block,
                                   start, end, src, dest, jmp_dest,
                                   src_count, dest_count, far, rewrites,
                                   internal_refs_need_update) &&
          !relocatePCRelativeHelper(dest_block,
                                    start, end, src, dest, jmp_dest,
                                    src_count, dest_count, far, rewrites,
                                    internal_refs_need_update) &&
          !relocateImmediateHelper(rel, dest_block, end,
                                   src, dest,
                                   src_count, dest_count, rewrites,
                                   internal_refs_need_update)) {
        // Do nothing, as the instruction word was initially copied above
      }

      if (src == start) {
        /*
         * For the start of the range, we only want to overwrite the "after"
         * address (since the "before" address could belong to the previous
         * tracelet, which could be being relocated to a completely different
         * address. recordRange will do that for us, so just make sure we
         * have the right address setup.
         */
        dest_start = dest;
      } else {
        rel.recordAddress(src, dest, 0);
      }

      src += src_count * kInstructionSize;
      dest += dest_count * kInstructionSize;
      asm_count += dest_count;
      assertx(dest <= dest_block.frontier());
      dest_block.setFrontier(dest);
    } // while (src != end)

    if (exit_addr) {
      *exit_addr = jmp_dest;
    }

    rel.recordRange(start, end, dest_start, dest_block.frontier());

    /*
     * Finally update any internal refs if needed. This indicates that the
     * range of instructions grew/shrank and therefore the internal refs
     * may be off.
     */
    if (internal_refs_need_update) {
      src = start;
      bool ok = true;
      while (src != end) {
        auto instr = Instruction::Cast(src);
        auto dest = rel.adjustedAddressAfter(src);
        auto instr2 = Instruction::Cast(dest);

        // Adjust this instruction if A) it wasn't written from a pc relative
        // instruction to an absolute (or vice-versa) and B) it isn't a literal.
        if (!rewrites.count(instr) && !literals.count(instr)) {
          /*
           * PC Relative
           *   ADR/ADRP
           *   LDR (literal)
           *   B[.<cc>] (immediate)
           */
          if (instr->IsPCRelAddressing() ||
              instr->IsLoadLiteral() ||
              instr->IsCondBranchImm() ||
              instr->IsUncondBranchImm()) {
            auto old_target = reinterpret_cast<TCA>(instr->ImmPCOffsetTarget());
            auto adjusted_target = rel.adjustedAddressAfter(old_target);
            auto new_target = (adjusted_target) ? adjusted_target : old_target;

            /*
             * Calculate the new offset and update. At this stage, we've already
             * relocated and now We're just adjusting an internal reference.
             * Therefore we can't change relative instructions to absolute, as
             * that would change the code size. Our only recourse is to mark it
             * as far and then retry the entire relocation again.
             */
            int imm = Instruction::Cast(new_target) - instr2;
            if ((instr->IsPCRelAddressing() && !is_int21(imm)) ||
                (instr->IsLoadLiteral() && !is_int19(imm)) ||
                (instr->IsCondBranchImm() &&
                 !is_int19(imm >> vixl::kInstructionSizeLog2)) ||
                (instr->IsUncondBranchImm() &&
                 !is_int26(imm >> vixl::kInstructionSizeLog2))) {
              FTRACE(3,
                     "relocate: PC relative instruction at {} has",
                     "internal reference 0x{:08x} which can't be adjusted.",
                     "Will try again and far.\n",
                     (uint64_t)instr, imm);
              far.insert(instr);
              ok = false;
            } else {
              instr2->SetImmPCOffsetTarget(Instruction::Cast(new_target));
            }
          }

          /*
           * Immediates
           *   LDR (literal)
           *   MOV/MOVK
           */
          if (instr->IsLoadLiteral()) {
            auto addr = reinterpret_cast<TCA*>(instr2->LiteralAddress());
            auto target = *addr;
            auto adjusted = rel.adjustedAddressAfter(target);

            if (!adjusted) {
              // Consider the case of a non-initialized mcprep smashableMovq
              target = reinterpret_cast<TCA>((uint64_t(target) >> 1));
              adjusted = rel.adjustedAddressAfter(target);
              if (adjusted) {
                adjusted = reinterpret_cast<TCA>((uint64_t(adjusted) << 1) | 1);
              }
            }

            if (adjusted) {
              *addr = adjusted;
              __builtin___clear_cache(reinterpret_cast<char*>(addr),
                                      reinterpret_cast<char*>(addr) + 8);
            }
          } else if (instr->IsMovz()) {
            size_t length = 1;
            const auto rd = instr->Rd();
            uint64_t target = instr->ImmMoveWide() << (16 * instr->ShiftMoveWide());
            auto next = instr->NextInstruction();
            while (next->IsMovk()) {
              if (next->Rd() == rd) {
                target |= next->ImmMoveWide() << (16 * next->ShiftMoveWide());
                length++;
              }
              next = next->NextInstruction();
            }
            // Adjust the mov/movk sequence if necessary
            auto adjusted = rel.adjustedAddressAfter((TCA)target);
            if (adjusted) {
              // Save the frontier for restoration below.
              auto savedFrontier = dest_block.frontier();
              dest_block.setFrontier(dest);

              // Write the new mov/movk sequence.
              vixl::MacroAssembler a { dest_block };
              auto const dst = vixl::Register(rd, 64);
              a.Mov(dst, adjusted);

              // If the new sequence is longer than the original, then we must
              // gracefully fail.
              length -= (dest_block.frontier() - dest) / kInstructionSize;
              if (length < 0) {
                ok = false;
                far.insert(instr);
              }

              // If the sequence is shorter, then pad with nops
              while (length > 0) {
                a.nop();
                length--;
              }

              // Restore the frontier
              dest_block.setFrontier(savedFrontier);
            }
          }
        }

        src += kInstructionSize;
      }
      if (!ok) {
        throw JmpOutOfRange();
      }
    }
    rel.markAddressImmediates(meta.addressImmediates);
  } catch (...) {
    rel.rewind(start, end);
    dest_block.setFrontier(dest_start);
    throw;
  }
  __builtin___clear_cache(reinterpret_cast<char*>(dest_start),
                          reinterpret_cast<char*>(dest_block.frontier()));

  return asm_count;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void adjustInstruction(RelocationInfo& rel, Instruction* instr,
                       bool live) {
  /*
   * PC Relative
   *   ADR/ADRP
   *   LDR (literal)
   *   B[.<cc>] (immediate)
   */
  if (instr->IsPCRelAddressing() ||
      instr->IsLoadLiteral() ||
      instr->IsCondBranchImm() ||
      instr->IsUncondBranchImm()) {

    auto adjusted = rel.adjustedAddressAfter((TCA)instr->ImmPCOffsetTarget());
    if (adjusted) {
      /*
       * Calculate the new offset and determine if it can be encoded.
       * We're adjusting, not relocating. So if the offset can't be
       * encoded, our only recourse is to assert.
       */
      int imm = Instruction::Cast(adjusted) - instr;
      if (instr->IsPCRelAddressing()) {
        always_assert_flog(is_int21(imm),
          "Can't adjust ADR, imm won't fit in 21 bits.\n");
      } else if (instr->IsLoadLiteral()) {
        always_assert_flog(is_int19(imm),
          "Can't adjust LDR literal, imm won't fit in 19 bits.\n");
      } else if (instr->IsCondBranchImm()) {
        imm >>= vixl::kInstructionSizeLog2;
        always_assert_flog(is_int19(imm),
          "Can't adjust B.<cc>, imm won't fit in 19 bits.\n");
      } else if (instr->IsUncondBranchImm()) {
        imm >>= vixl::kInstructionSizeLog2;
        always_assert_flog(is_int26(imm),
          "Can't adjust B, imm won't fit in 26 bits.\n");
      }

      // Update offset
      instr->SetImmPCOffsetTarget(Instruction::Cast(adjusted));
      __builtin___clear_cache(reinterpret_cast<char*>(instr),
                              reinterpret_cast<char*>(instr) + 4);
    }
  }

  /*
   * Immediates
   *   LDR (literal)
   *   MOV/MOVK
   *
   * Note: We can't atomically rewrite multiple instructions, so we
   *       assert when attempting to adjust MOV/MOVK when live.
   */
  if (instr->IsLoadLiteral()) {
    auto addr = reinterpret_cast<TCA*>(instr->LiteralAddress());
    auto target = *addr;
    auto adjusted = rel.adjustedAddressAfter(target);
    if (adjusted) {
      *addr = adjusted;
      __builtin___clear_cache(reinterpret_cast<char*>(addr),
                              reinterpret_cast<char*>(addr) + 8);
    }
  } else if (instr->IsMovz()) {
    const auto rd = instr->Rd();
    uint64_t target = instr->ImmMoveWide() << (16 * instr->ShiftMoveWide());
    auto next = instr->NextInstruction();
    size_t length = 1;
    while (next->IsMovk()) {
      if (next->Rd() == rd) {
        target |= next->ImmMoveWide() << (16 * next->ShiftMoveWide());
        length++;
      }
      next = next->NextInstruction();
    }
    auto adjusted = (uint64_t)rel.adjustedAddressAfter((TCA)target);
    if (adjusted) {
      always_assert_flog(!live, "Can't adjust MOV/MOVK for a live region.\n");

      // Rewrite each MOV/MOVK immediate
      auto tmp = adjusted;
      next = instr;
      size_t adjLength = 0, shift = 0;
      do {
        if (tmp & 0xffff) {
          Instr bits = next->InstructionBits();
          bits &= ~(Assembler::ImmMoveWide(0xffff) |
                    Assembler::ShiftMoveWide(0x3));
          bits |= (Assembler::ImmMoveWide(tmp & 0xffff) |
                   Assembler::ShiftMoveWide(adjLength));
          next->SetInstructionBits(bits);
          adjLength++;
        }
        always_assert_flog(length >= adjLength,
                           "Can't adjust MOV/MOVK, new sequence is longer.\n");
        shift++;
        next = next->NextInstruction();
        tmp >>= 16;
      }        while (tmp);

      // Pad out with nops.
      while (adjLength < length) {
        next->SetInstructionBits(HINT |
                                 Assembler::ImmHint(NOP) |
                                 Assembler::Rt(xzr));
        next = next->NextInstruction();
        adjLength++;
      }
    }
  }
}

void adjustInstructions(RelocationInfo& rel,
                        Instruction* start, Instruction* end,
                        bool live) {
  // Find the literals
  InstrSet literals = findLiterals(start, end);

  // Adjust the instructions
  auto instr = start;
  while (instr != end) {
    assertx(instr < end);
    if (!literals.count(instr)) {
      adjustInstruction(rel, instr, live);
    }
    instr = instr->NextInstruction();
  }
}

//////////////////////////////////////////////////////////////////////

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
  auto start = Instruction::Cast(rel.adjustedAddressAfter(srcStart));
  auto end = Instruction::Cast(rel.adjustedAddressBefore(srcEnd));

  if (!start) {
    start = Instruction::Cast(srcStart);
    end = Instruction::Cast(srcEnd);
  } else {
    always_assert(end);
  }

  adjustInstructions(rel, start, end, false);
}

/*
 * Adjust potentially live references that point into the relocated area. Must
 * not be called until its safe to run the relocated code.
 */
void adjustCodeForRelocation(RelocationInfo& rel, CGMeta& meta) {
  for (auto addr : meta.reusedStubs) {
    auto start = Instruction::Cast(addr);
    auto end = start;

    while (end->Mask(ExceptionMask) != BRK) {
      end = end->NextInstruction();
    }

    adjustInstructions(rel, start, end, true);
  }

  for (auto codePtr : meta.codePointers) {
    if (auto adjusted = rel.adjustedAddressAfter(*codePtr)) {
      *codePtr = adjusted;
    }
  }
}

void adjustMetaDataForRelocation(RelocationInfo& rel,
                                 AsmInfo* asmInfo,
                                 CGMeta& meta) {
  decltype(meta.smashableLocations) updatedSL;
  for (auto sl : meta.smashableLocations) {
    if (auto adjusted = rel.adjustedAddressAfter(sl)) {
      updatedSL.insert(adjusted);
    } else {
      updatedSL.insert(sl);
    }
  }
  updatedSL.swap(meta.smashableLocations);
}

void findFixups(TCA start, TCA end, CGMeta& meta) {
  while (start != end) {
    assert(start < end);
    auto instr = Instruction::Cast(start);
    start = reinterpret_cast<TCA>(instr->NextInstruction());

    // If instruction is a call
    if ((instr->Mask(UnconditionalBranchMask) == BL) ||
        (instr->Mask(UnconditionalBranchToRegisterMask) == BLR)) {
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
                CodeBlock&,
                CGMeta& meta,
                TCA* exit_addr,
                AreaIndex) {
  InstrSet far;
  while (true) {
    try {
      return relocateImpl(rel, dest_block, start, end,
                          meta, exit_addr, far);
    } catch (JmpOutOfRange& j) {
    }
  }
}

//////////////////////////////////////////////////////////////////////

}}}
