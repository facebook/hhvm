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

#pragma once

#include <cstddef>
#include <cstdint>

#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/assertions.h"

#include "hphp/vixl/hphp-compat.h"

namespace HPHP::jit::arm {

///////////////////////////////////////////////////////////////////////////////

// ARM LDP/STP addressing-range predicates. Pure addressing math (no ARM
// assembler), used by the ARM vasm simplifier (to fuse adjacent loads/stores)
// and by the ARM emitter (to decide whether a loadpair/storepair fits the STP
// immediate range or must fall back to two single loads/stores).
//
// LDP/STP use a signed imm7 displacement scaled by the accessed element size.
inline bool encodablePair(const Vptr& ptr, int32_t scale) {
  auto const minDisp = -64 * scale;
  auto const maxDisp = 63 * scale;
  return ptr.base.isValid() &&
         !ptr.index.isValid() &&
         ptr.disp >= minDisp &&
         ptr.disp <= maxDisp &&
         ptr.disp % scale == 0;
}

inline bool encodablePair64(const Vptr& ptr) {
  return encodablePair(ptr, 8);
}

inline bool encodablePair32(const Vptr& ptr) {
  return encodablePair(ptr, 4);
}

inline bool encodablePair128(const Vptr& ptr) {
  return encodablePair(ptr, 16);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * A decoded ARM compare-and-branch (cbz/cbnz). Shared by the emitter's veneer
 * generation and the relocator's far-branch expand/shrink paths so they agree
 * on operand width and condition. `reg` is sized W (cbzl/cbnzl) or X
 * (cbzq/cbnzq) per the instruction's sf bit; `isCbnz` is true for cbnz.
 */
struct CompareAndBranchDetails {
  vixl::aarch64::Register reg;
  bool isCbnz;
};
CompareAndBranchDetails getCompareAndBranchDetails(
  const vixl::aarch64::Instruction* cb);

///////////////////////////////////////////////////////////////////////////////

/*
 * Represents a literal-pool load, either a single LDR (literal) ("near" form,
 * +/-1MB range) or an ADRP/LDR pair ("far" form, +/-4GB range). Encapsulates
 * the two-instruction-vs-one ambiguity so callers don't have to thread a
 * separate `maybe_ldr` pointer through their helpers.
 *
 * Layout invariant matches the constructor contract:
 *   far == true  : instrs_[0] = ADRP, instrs_[1] = LDR
 *   far == false : instrs_[0] = LDR,  instrs_[1] = nullptr
 *
 * A default-constructed (or failed-lookup) instance reports valid() == false
 * and must not be queried beyond that.
 */
class LoadLiteral {
public:
  using Instruction = vixl::aarch64::Instruction;

  /*
   * Try to recognize a literal load starting at `start`. `end`, when non-null,
   * bounds the lookup of the second instruction for the far form. Returns an
   * invalid LoadLiteral if `start` is not the beginning of a near LDR or an
   * ADRP/LDR pair.
   */
  static LoadLiteral at(Instruction* start,
                        const Instruction* end = nullptr);

  /*
   * Construct a near-form view of an LDR slot without validating its current
   * encoding. Use this when about to patch a placeholder LDR (e.g., the LDR
   * half of an existing ADRP/LDR pair) into a near LDR (literal) via
   * setTarget(). The slot's current bits must already encode the desired load
   * width (32 vs 64) — setTarget() preserves that choice.
   */
  static LoadLiteral nearAt(Instruction* ldr) {
    return LoadLiteral{ldr, nullptr, false};
  }

  LoadLiteral() = default;

  bool valid() const {
    return instrs_[0] != nullptr && (far_ == (instrs_[1] != nullptr));
  }
  explicit operator bool() const { return valid(); }

  /* True if this is the ADRP/LDR pair form. */
  bool isFar() const { return far_; }

  /* First instruction of the sequence (ADRP for far, LDR for near). */
  Instruction* start() const {
    assertx(valid());
    return instrs_[0];
  }

  /* The LDR for either form. */
  Instruction* ldr() const {
    assertx(valid());
    return far_ ? instrs_[1] : instrs_[0];
  }

  /* The ADRP for the far form; nullptr for the near form. */
  Instruction* adrp() const {
    assertx(valid());
    return far_ ? instrs_[0] : nullptr;
  }

  /* Number of instructions in the sequence (1 for near, 2 for far). */
  size_t instrCount() const {
    assertx(valid());
    return far_ ? 2 : 1;
  }

  /* Width of the loaded literal in bits (32 or 64). */
  uint8_t width() const;

  /* Destination register number of the LDR. */
  uint32_t destReg() const;

  /* Byte offset from start() to ldr() (0 for near, kInstructionSize for far). */
  uint8_t offset() const;

  /*
   * Address of the literal the load currently encodes, computed as if start()
   * were located at `logicalStart`. With no argument, uses start() itself.
   */
  uint8_t* literalAddress(const Instruction* logicalStart) const;
  uint8_t* literalAddress() const {
    return literalAddress(static_cast<const Instruction*>(start()));
  }

  /*
   * Patch the load to point at `target`, with PC-relative offsets computed
   * against `from`. Returns false if the new offset cannot be encoded — for
   * the near form, the +/-1MB imm19 range; for the far form, the +/-4GB imm21
   * page range or page-offset misalignment for the load width.
   *
   * Caller must sync the I-cache.
   */
  bool setTarget(const Instruction* target, const Instruction* from) const;

  /*
   * True iff a near LDR (literal) at `from` can reach `target`: the byte
   * offset must be 4-byte aligned and fit in the imm19 PC-relative range
   * (+/-1MB).
   */
  static bool reachableByNearLiteral(const Instruction* target,
                                     const Instruction* from);

  /*
   * True iff an ADRP at `from` can reach the page containing `target`: the
   * page diff must fit in imm21 (+/-4GB).
   */
  static bool reachableByFarLiteral(const Instruction* target,
                                    const Instruction* from);

private:
  LoadLiteral(Instruction* first, Instruction* second, bool far)
    : instrs_{first, second}, far_(far) {}

  Instruction* instrs_[2] = { nullptr, nullptr };
  bool far_ = false;
};

}
