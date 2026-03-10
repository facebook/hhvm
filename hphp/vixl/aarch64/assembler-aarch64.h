// Copyright 2015, VIXL authors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of ARM Limited nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef VIXL_AARCH64_ASSEMBLER_AARCH64_H_
#define VIXL_AARCH64_ASSEMBLER_AARCH64_H_

#include "../assembler-base-vixl.h"
#include "../code-generation-scopes-vixl.h"
#include "../cpu-features.h"
#include "../globals-vixl.h"
#include "../invalset-vixl.h"
#include "../utils-vixl.h"

#include "operands-aarch64.h"

namespace vixl {
namespace aarch64 {

class LabelTestHelper;  // Forward declaration.


class Label {
 public:
  Label() : location_(kLocationUnbound) {}
  ~Label() {
    // All links to a label must have been resolved before it is destructed.
    VIXL_ASSERT(!IsLinked());
  }

  bool IsBound() const { return location_ >= 0; }
  bool IsLinked() const { return !links_.empty(); }

  ptrdiff_t GetLocation() const { return location_; }
  VIXL_DEPRECATED("GetLocation", ptrdiff_t location() const) {
    return GetLocation();
  }

  static const int kNPreallocatedLinks = 4;
  static const ptrdiff_t kInvalidLinkKey = PTRDIFF_MAX;
  static const size_t kReclaimFrom = 512;
  static const size_t kReclaimFactor = 2;

  typedef InvalSet<ptrdiff_t,
                   kNPreallocatedLinks,
                   ptrdiff_t,
                   kInvalidLinkKey,
                   kReclaimFrom,
                   kReclaimFactor>
      LinksSetBase;
  typedef InvalSetIterator<LinksSetBase> LabelLinksIteratorBase;

 private:
  class LinksSet : public LinksSetBase {
   public:
    LinksSet() : LinksSetBase() {}
  };

  // Allows iterating over the links of a label. The behaviour is undefined if
  // the list of links is modified in any way while iterating.
  class LabelLinksIterator : public LabelLinksIteratorBase {
   public:
    explicit LabelLinksIterator(Label* label)
        : LabelLinksIteratorBase(&label->links_) {}

    // TODO: Remove these and use the STL-like interface instead.
    using LabelLinksIteratorBase::Advance;
    using LabelLinksIteratorBase::Current;
  };

  void Bind(ptrdiff_t location) {
    // Labels can only be bound once.
    VIXL_ASSERT(!IsBound());
    location_ = location;
  }

  void AddLink(ptrdiff_t instruction) {
    // If a label is bound, the assembler already has the information it needs
    // to write the instruction, so there is no need to add it to links_.
    VIXL_ASSERT(!IsBound());
    links_.insert(instruction);
  }

  void DeleteLink(ptrdiff_t instruction) { links_.erase(instruction); }

  void ClearAllLinks() { links_.clear(); }

  // TODO: The comment below considers average case complexity for our
  // usual use-cases. The elements of interest are:
  // - Branches to a label are emitted in order: branch instructions to a label
  // are generated at an offset in the code generation buffer greater than any
  // other branch to that same label already generated. As an example, this can
  // be broken when an instruction is patched to become a branch. Note that the
  // code will still work, but the complexity considerations below may locally
  // not apply any more.
  // - Veneers are generated in order: for multiple branches of the same type
  // branching to the same unbound label going out of range, veneers are
  // generated in growing order of the branch instruction offset from the start
  // of the buffer.
  //
  // When creating a veneer for a branch going out of range, the link for this
  // branch needs to be removed from this `links_`. Since all branches are
  // tracked in one underlying InvalSet, the complexity for this deletion is the
  // same as for finding the element, ie. O(n), where n is the number of links
  // in the set.
  // This could be reduced to O(1) by using the same trick as used when tracking
  // branch information for veneers: split the container to use one set per type
  // of branch. With that setup, when a veneer is created and the link needs to
  // be deleted, if the two points above hold, it must be the minimum element of
  // the set for its type of branch, and that minimum element will be accessible
  // in O(1).

  // The offsets of the instructions that have linked to this label.
  LinksSet links_;
  // The label location.
  ptrdiff_t location_;

  static const ptrdiff_t kLocationUnbound = -1;

// It is not safe to copy labels, so disable the copy constructor and operator
// by declaring them private (without an implementation).
#if __cplusplus >= 201103L
  Label(const Label&) = delete;
  void operator=(const Label&) = delete;
#else
  Label(const Label&);
  void operator=(const Label&);
#endif

  // The Assembler class is responsible for binding and linking labels, since
  // the stored offsets need to be consistent with the Assembler's buffer.
  friend class Assembler;
  // The MacroAssembler and VeneerPool handle resolution of branches to distant
  // targets.
  friend class MacroAssembler;
  friend class VeneerPool;
};


class Assembler;
class LiteralPool;

// A literal is a 32-bit or 64-bit piece of data stored in the instruction
// stream and loaded through a pc relative load. The same literal can be
// referred to by multiple instructions but a literal can only reside at one
// place in memory. A literal can be used by a load before or after being
// placed in memory.
//
// Internally an offset of 0 is associated with a literal which has been
// neither used nor placed. Then two possibilities arise:
//  1) the label is placed, the offset (stored as offset + 1) is used to
//     resolve any subsequent load using the label.
//  2) the label is not placed and offset is the offset of the last load using
//     the literal (stored as -offset -1). If multiple loads refer to this
//     literal then the last load holds the offset of the preceding load and
//     all loads form a chain. Once the offset is placed all the loads in the
//     chain are resolved and future loads fall back to possibility 1.
class RawLiteral {
 public:
  enum DeletionPolicy {
    kDeletedOnPlacementByPool,
    kDeletedOnPoolDestruction,
    kManuallyDeleted
  };

  RawLiteral(size_t size,
             LiteralPool* literal_pool,
             DeletionPolicy deletion_policy = kManuallyDeleted);

  // The literal pool only sees and deletes `RawLiteral*` pointers, but they are
  // actually pointing to `Literal<T>` objects.
  virtual ~RawLiteral() {}

  size_t GetSize() const {
    VIXL_STATIC_ASSERT(kDRegSizeInBytes == kXRegSizeInBytes);
    VIXL_STATIC_ASSERT(kSRegSizeInBytes == kWRegSizeInBytes);
    VIXL_ASSERT((size_ == kXRegSizeInBytes) || (size_ == kWRegSizeInBytes) ||
                (size_ == kQRegSizeInBytes));
    return size_;
  }
  VIXL_DEPRECATED("GetSize", size_t size()) { return GetSize(); }

  uint64_t GetRawValue128Low64() const {
    VIXL_ASSERT(size_ == kQRegSizeInBytes);
    return low64_;
  }
  VIXL_DEPRECATED("GetRawValue128Low64", uint64_t raw_value128_low64()) {
    return GetRawValue128Low64();
  }

  uint64_t GetRawValue128High64() const {
    VIXL_ASSERT(size_ == kQRegSizeInBytes);
    return high64_;
  }
  VIXL_DEPRECATED("GetRawValue128High64", uint64_t raw_value128_high64()) {
    return GetRawValue128High64();
  }

  uint64_t GetRawValue64() const {
    VIXL_ASSERT(size_ == kXRegSizeInBytes);
    VIXL_ASSERT(high64_ == 0);
    return low64_;
  }
  VIXL_DEPRECATED("GetRawValue64", uint64_t raw_value64()) {
    return GetRawValue64();
  }

  uint32_t GetRawValue32() const {
    VIXL_ASSERT(size_ == kWRegSizeInBytes);
    VIXL_ASSERT(high64_ == 0);
    VIXL_ASSERT(IsUint32(low64_) || IsInt32(low64_));
    return static_cast<uint32_t>(low64_);
  }
  VIXL_DEPRECATED("GetRawValue32", uint32_t raw_value32()) {
    return GetRawValue32();
  }

  bool IsUsed() const { return offset_ < 0; }
  bool IsPlaced() const { return offset_ > 0; }

  LiteralPool* GetLiteralPool() const { return literal_pool_; }

  ptrdiff_t GetOffset() const {
    VIXL_ASSERT(IsPlaced());
    return offset_ - 1;
  }
  VIXL_DEPRECATED("GetOffset", ptrdiff_t offset()) { return GetOffset(); }

 protected:
  void SetOffset(ptrdiff_t offset) {
    VIXL_ASSERT(offset >= 0);
    VIXL_ASSERT(IsWordAligned(offset));
    VIXL_ASSERT(!IsPlaced());
    offset_ = offset + 1;
  }
  VIXL_DEPRECATED("SetOffset", void set_offset(ptrdiff_t offset)) {
    SetOffset(offset);
  }

  ptrdiff_t GetLastUse() const {
    VIXL_ASSERT(IsUsed());
    return -offset_ - 1;
  }
  VIXL_DEPRECATED("GetLastUse", ptrdiff_t last_use()) { return GetLastUse(); }

  void SetLastUse(ptrdiff_t offset) {
    VIXL_ASSERT(offset >= 0);
    VIXL_ASSERT(IsWordAligned(offset));
    VIXL_ASSERT(!IsPlaced());
    offset_ = -offset - 1;
  }
  VIXL_DEPRECATED("SetLastUse", void set_last_use(ptrdiff_t offset)) {
    SetLastUse(offset);
  }

  size_t size_;
  ptrdiff_t offset_;
  uint64_t low64_;
  uint64_t high64_;

 private:
  LiteralPool* literal_pool_;
  DeletionPolicy deletion_policy_;

  friend class Assembler;
  friend class LiteralPool;
};


template <typename T>
class Literal : public RawLiteral {
 public:
  explicit Literal(T value,
                   LiteralPool* literal_pool = NULL,
                   RawLiteral::DeletionPolicy ownership = kManuallyDeleted)
      : RawLiteral(sizeof(value), literal_pool, ownership) {
    VIXL_STATIC_ASSERT(sizeof(value) <= kXRegSizeInBytes);
    UpdateValue(value);
  }

  Literal(T high64,
          T low64,
          LiteralPool* literal_pool = NULL,
          RawLiteral::DeletionPolicy ownership = kManuallyDeleted)
      : RawLiteral(kQRegSizeInBytes, literal_pool, ownership) {
    VIXL_STATIC_ASSERT(sizeof(low64) == (kQRegSizeInBytes / 2));
    UpdateValue(high64, low64);
  }

  virtual ~Literal() {}

  // Update the value of this literal, if necessary by rewriting the value in
  // the pool.
  // If the literal has already been placed in a literal pool, the address of
  // the start of the code buffer must be provided, as the literal only knows it
  // offset from there. This also allows patching the value after the code has
  // been moved in memory.
  void UpdateValue(T new_value, uint8_t* code_buffer = NULL) {
    VIXL_ASSERT(sizeof(new_value) == size_);
    memcpy(&low64_, &new_value, sizeof(new_value));
    if (IsPlaced()) {
      VIXL_ASSERT(code_buffer != NULL);
      RewriteValueInCode(code_buffer);
    }
  }

  void UpdateValue(T high64, T low64, uint8_t* code_buffer = NULL) {
    VIXL_ASSERT(sizeof(low64) == size_ / 2);
    memcpy(&low64_, &low64, sizeof(low64));
    memcpy(&high64_, &high64, sizeof(high64));
    if (IsPlaced()) {
      VIXL_ASSERT(code_buffer != NULL);
      RewriteValueInCode(code_buffer);
    }
  }

  void UpdateValue(T new_value, const Assembler* assembler);
  void UpdateValue(T high64, T low64, const Assembler* assembler);

 private:
  void RewriteValueInCode(uint8_t* code_buffer) {
    VIXL_ASSERT(IsPlaced());
    VIXL_STATIC_ASSERT(sizeof(T) <= kXRegSizeInBytes);
    switch (GetSize()) {
      case kSRegSizeInBytes:
        *reinterpret_cast<uint32_t*>(code_buffer + GetOffset()) =
            GetRawValue32();
        break;
      case kDRegSizeInBytes:
        *reinterpret_cast<uint64_t*>(code_buffer + GetOffset()) =
            GetRawValue64();
        break;
      default:
        VIXL_ASSERT(GetSize() == kQRegSizeInBytes);
        uint64_t* base_address =
            reinterpret_cast<uint64_t*>(code_buffer + GetOffset());
        *base_address = GetRawValue128Low64();
        *(base_address + 1) = GetRawValue128High64();
    }
  }
};


// Control whether or not position-independent code should be emitted.
enum PositionIndependentCodeOption {
  // All code generated will be position-independent; all branches and
  // references to labels generated with the Label class will use PC-relative
  // addressing.
  PositionIndependentCode,

  // Allow VIXL to generate code that refers to absolute addresses. With this
  // option, it will not be possible to copy the code buffer and run it from a
  // different address; code must be generated in its final location.
  PositionDependentCode,

  // Allow VIXL to assume that the bottom 12 bits of the address will be
  // constant, but that the top 48 bits may change. This allows `adrp` to
  // function in systems which copy code between pages, but otherwise maintain
  // 4KB page alignment.
  PageOffsetDependentCode
};


// Control how scaled- and unscaled-offset loads and stores are generated.
enum LoadStoreScalingOption {
  // Prefer scaled-immediate-offset instructions, but emit unscaled-offset,
  // register-offset, pre-index or post-index instructions if necessary.
  PreferScaledOffset,

  // Prefer unscaled-immediate-offset instructions, but emit scaled-offset,
  // register-offset, pre-index or post-index instructions if necessary.
  PreferUnscaledOffset,

  // Require scaled-immediate-offset instructions.
  RequireScaledOffset,

  // Require unscaled-immediate-offset instructions.
  RequireUnscaledOffset
};


// Assembler.
class Assembler : public vixl::internal::AssemblerBase {
 public:
  explicit Assembler(
      PositionIndependentCodeOption pic = PositionIndependentCode)
      : pic_(pic), cpu_features_(CPUFeatures::AArch64LegacyBaseline()) {}
  explicit Assembler(
      size_t capacity,
      PositionIndependentCodeOption pic = PositionIndependentCode)
      : AssemblerBase(capacity),
        pic_(pic),
        cpu_features_(CPUFeatures::AArch64LegacyBaseline()) {}
  Assembler(byte* buffer,
            size_t capacity,
            PositionIndependentCodeOption pic = PositionIndependentCode)
      : AssemblerBase(buffer, capacity),
        pic_(pic),
        cpu_features_(CPUFeatures::AArch64LegacyBaseline()) {}

  // Upon destruction, the code will assert that one of the following is true:
  //  * The Assembler object has not been used.
  //  * Nothing has been emitted since the last Reset() call.
  //  * Nothing has been emitted since the last FinalizeCode() call.
  ~Assembler() {}

  // System functions.

  // Start generating code from the beginning of the buffer, discarding any code
  // and data that has already been emitted into the buffer.
  void Reset();

  // Bind a label to the current PC.
  void bind(Label* label);

  // Bind a label to a specified offset from the start of the buffer.
  void BindToOffset(Label* label, ptrdiff_t offset);

  // Place a literal at the current PC.
  void place(RawLiteral* literal);

  VIXL_DEPRECATED("GetCursorOffset", ptrdiff_t CursorOffset() const) {
    return GetCursorOffset();
  }

  VIXL_DEPRECATED("GetBuffer().GetCapacity()",
                  ptrdiff_t GetBufferEndOffset() const) {
    return static_cast<ptrdiff_t>(GetBuffer().GetCapacity());
  }
  VIXL_DEPRECATED("GetBuffer().GetCapacity()",
                  ptrdiff_t BufferEndOffset() const) {
    return GetBuffer().GetCapacity();
  }

  // Return the address of a bound label.
  template <typename T>
  T GetLabelAddress(const Label* label) const {
    VIXL_ASSERT(label->IsBound());
    VIXL_STATIC_ASSERT(sizeof(T) >= sizeof(uintptr_t));
    return GetBuffer().GetOffsetAddress<T>(label->GetLocation());
  }

  Instruction* GetInstructionAt(ptrdiff_t instruction_offset) {
    return GetBuffer()->GetOffsetAddress<Instruction*>(instruction_offset);
  }
  VIXL_DEPRECATED("GetInstructionAt",
                  Instruction* InstructionAt(ptrdiff_t instruction_offset)) {
    return GetInstructionAt(instruction_offset);
  }

  ptrdiff_t GetInstructionOffset(Instruction* instruction) {
    VIXL_STATIC_ASSERT(sizeof(*instruction) == 1);
    ptrdiff_t offset =
        instruction - GetBuffer()->GetStartAddress<Instruction*>();
    VIXL_ASSERT((0 <= offset) &&
                (offset < static_cast<ptrdiff_t>(GetBuffer()->GetCapacity())));
    return offset;
  }
  VIXL_DEPRECATED("GetInstructionOffset",
                  ptrdiff_t InstructionOffset(Instruction* instruction)) {
    return GetInstructionOffset(instruction);
  }

  // Instruction set functions.

  // Branch / Jump instructions.

  // Branch to register.
  void br(const Register& xn);

  // Branch with link to register.
  void blr(const Register& xn);

  // Branch to register with return hint.
  void ret(const Register& xn = lr);

  // Branch to register, with pointer authentication. Using key A and a modifier
  // of zero [Armv8.3].
  void braaz(const Register& xn);

  // Branch to register, with pointer authentication. Using key B and a modifier
  // of zero [Armv8.3].
  void brabz(const Register& xn);

  // Branch with link to register, with pointer authentication. Using key A and
  // a modifier of zero [Armv8.3].
  void blraaz(const Register& xn);

  // Branch with link to register, with pointer authentication. Using key B and
  // a modifier of zero [Armv8.3].
  void blrabz(const Register& xn);

  // Return from subroutine, with pointer authentication. Using key A [Armv8.3].
  void retaa();

  // Return from subroutine, with pointer authentication. Using key B [Armv8.3].
  void retab();

  // Branch to register, with pointer authentication. Using key A [Armv8.3].
  void braa(const Register& xn, const Register& xm);

  // Branch to register, with pointer authentication. Using key B [Armv8.3].
  void brab(const Register& xn, const Register& xm);

  // Branch with link to register, with pointer authentication. Using key A
  // [Armv8.3].
  void blraa(const Register& xn, const Register& xm);

  // Branch with link to register, with pointer authentication. Using key B
  // [Armv8.3].
  void blrab(const Register& xn, const Register& xm);

  // Unconditional branch to label.
  void b(Label* label);

  // Conditional branch to label.
  void b(Label* label, Condition cond);

  // Unconditional branch to PC offset.
  void b(int64_t imm26);

  // Conditional branch to PC offset.
  void b(int64_t imm19, Condition cond);

  // Branch with link to label.
  void bl(Label* label);

  // Branch with link to PC offset.
  void bl(int64_t imm26);

  // Compare and branch to label if zero.
  void cbz(const Register& rt, Label* label);

  // Compare and branch to PC offset if zero.
  void cbz(const Register& rt, int64_t imm19);

  // Compare and branch to label if not zero.
  void cbnz(const Register& rt, Label* label);

  // Compare and branch to PC offset if not zero.
  void cbnz(const Register& rt, int64_t imm19);

  // Table lookup from one register.
  void tbl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Table lookup from two registers.
  void tbl(const VRegister& vd,
           const VRegister& vn,
           const VRegister& vn2,
           const VRegister& vm);

  // Table lookup from three registers.
  void tbl(const VRegister& vd,
           const VRegister& vn,
           const VRegister& vn2,
           const VRegister& vn3,
           const VRegister& vm);

  // Table lookup from four registers.
  void tbl(const VRegister& vd,
           const VRegister& vn,
           const VRegister& vn2,
           const VRegister& vn3,
           const VRegister& vn4,
           const VRegister& vm);

  // Table lookup extension from one register.
  void tbx(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Table lookup extension from two registers.
  void tbx(const VRegister& vd,
           const VRegister& vn,
           const VRegister& vn2,
           const VRegister& vm);

  // Table lookup extension from three registers.
  void tbx(const VRegister& vd,
           const VRegister& vn,
           const VRegister& vn2,
           const VRegister& vn3,
           const VRegister& vm);

  // Table lookup extension from four registers.
  void tbx(const VRegister& vd,
           const VRegister& vn,
           const VRegister& vn2,
           const VRegister& vn3,
           const VRegister& vn4,
           const VRegister& vm);

  // Test bit and branch to label if zero.
  void tbz(const Register& rt, unsigned bit_pos, Label* label);

  // Test bit and branch to PC offset if zero.
  void tbz(const Register& rt, unsigned bit_pos, int64_t imm14);

  // Test bit and branch to label if not zero.
  void tbnz(const Register& rt, unsigned bit_pos, Label* label);

  // Test bit and branch to PC offset if not zero.
  void tbnz(const Register& rt, unsigned bit_pos, int64_t imm14);

  // Address calculation instructions.
  // Calculate a PC-relative address. Unlike for branches the offset in adr is
  // unscaled (i.e. the result can be unaligned).

  // Calculate the address of a label.
  void adr(const Register& xd, Label* label);

  // Calculate the address of a PC offset.
  void adr(const Register& xd, int64_t imm21);

  // Calculate the page address of a label.
  void adrp(const Register& xd, Label* label);

  // Calculate the page address of a PC offset.
  void adrp(const Register& xd, int64_t imm21);

  // Data Processing instructions.

  // Add.
  void add(const Register& rd, const Register& rn, const Operand& operand);

  // Add and update status flags.
  void adds(const Register& rd, const Register& rn, const Operand& operand);

  // Compare negative.
  void cmn(const Register& rn, const Operand& operand);

  // Subtract.
  void sub(const Register& rd, const Register& rn, const Operand& operand);

  // Subtract and update status flags.
  void subs(const Register& rd, const Register& rn, const Operand& operand);

  // Compare.
  void cmp(const Register& rn, const Operand& operand);

  // Negate.
  void neg(const Register& rd, const Operand& operand);

  // Negate and update status flags.
  void negs(const Register& rd, const Operand& operand);

  // Add with carry bit.
  void adc(const Register& rd, const Register& rn, const Operand& operand);

  // Add with carry bit and update status flags.
  void adcs(const Register& rd, const Register& rn, const Operand& operand);

  // Subtract with carry bit.
  void sbc(const Register& rd, const Register& rn, const Operand& operand);

  // Subtract with carry bit and update status flags.
  void sbcs(const Register& rd, const Register& rn, const Operand& operand);

  // Rotate register right and insert into NZCV flags under the control of a
  // mask [Armv8.4].
  void rmif(const Register& xn, unsigned rotation, StatusFlags flags);

  // Set NZCV flags from register, treated as an 8-bit value [Armv8.4].
  void setf8(const Register& rn);

  // Set NZCV flags from register, treated as an 16-bit value [Armv8.4].
  void setf16(const Register& rn);

  // Negate with carry bit.
  void ngc(const Register& rd, const Operand& operand);

  // Negate with carry bit and update status flags.
  void ngcs(const Register& rd, const Operand& operand);

  // Logical instructions.

  // Bitwise and (A & B).
  void and_(const Register& rd, const Register& rn, const Operand& operand);

  // Bitwise and (A & B) and update status flags.
  void ands(const Register& rd, const Register& rn, const Operand& operand);

  // Bit test and set flags.
  void tst(const Register& rn, const Operand& operand);

  // Bit clear (A & ~B).
  void bic(const Register& rd, const Register& rn, const Operand& operand);

  // Bit clear (A & ~B) and update status flags.
  void bics(const Register& rd, const Register& rn, const Operand& operand);

  // Bitwise or (A | B).
  void orr(const Register& rd, const Register& rn, const Operand& operand);

  // Bitwise nor (A | ~B).
  void orn(const Register& rd, const Register& rn, const Operand& operand);

  // Bitwise eor/xor (A ^ B).
  void eor(const Register& rd, const Register& rn, const Operand& operand);

  // Bitwise enor/xnor (A ^ ~B).
  void eon(const Register& rd, const Register& rn, const Operand& operand);

  // Logical shift left by variable.
  void lslv(const Register& rd, const Register& rn, const Register& rm);

  // Logical shift right by variable.
  void lsrv(const Register& rd, const Register& rn, const Register& rm);

  // Arithmetic shift right by variable.
  void asrv(const Register& rd, const Register& rn, const Register& rm);

  // Rotate right by variable.
  void rorv(const Register& rd, const Register& rn, const Register& rm);

  // Bitfield instructions.

  // Bitfield move.
  void bfm(const Register& rd,
           const Register& rn,
           unsigned immr,
           unsigned imms);

  // Signed bitfield move.
  void sbfm(const Register& rd,
            const Register& rn,
            unsigned immr,
            unsigned imms);

  // Unsigned bitfield move.
  void ubfm(const Register& rd,
            const Register& rn,
            unsigned immr,
            unsigned imms);

  // Bfm aliases.

  // Bitfield insert.
  void bfi(const Register& rd,
           const Register& rn,
           unsigned lsb,
           unsigned width) {
    VIXL_ASSERT(width >= 1);
    VIXL_ASSERT(lsb + width <= static_cast<unsigned>(rn.GetSizeInBits()));
    bfm(rd,
        rn,
        (rd.GetSizeInBits() - lsb) & (rd.GetSizeInBits() - 1),
        width - 1);
  }

  // Bitfield extract and insert low.
  void bfxil(const Register& rd,
             const Register& rn,
             unsigned lsb,
             unsigned width) {
    VIXL_ASSERT(width >= 1);
    VIXL_ASSERT(lsb + width <= static_cast<unsigned>(rn.GetSizeInBits()));
    bfm(rd, rn, lsb, lsb + width - 1);
  }

  // Bitfield clear [Armv8.2].
  void bfc(const Register& rd, unsigned lsb, unsigned width) {
    bfi(rd, AppropriateZeroRegFor(rd), lsb, width);
  }

  // Sbfm aliases.

  // Arithmetic shift right.
  void asr(const Register& rd, const Register& rn, unsigned shift) {
    VIXL_ASSERT(shift < static_cast<unsigned>(rd.GetSizeInBits()));
    sbfm(rd, rn, shift, rd.GetSizeInBits() - 1);
  }

  // Signed bitfield insert with zero at right.
  void sbfiz(const Register& rd,
             const Register& rn,
             unsigned lsb,
             unsigned width) {
    VIXL_ASSERT(width >= 1);
    VIXL_ASSERT(lsb + width <= static_cast<unsigned>(rn.GetSizeInBits()));
    sbfm(rd,
         rn,
         (rd.GetSizeInBits() - lsb) & (rd.GetSizeInBits() - 1),
         width - 1);
  }

  // Signed bitfield extract.
  void sbfx(const Register& rd,
            const Register& rn,
            unsigned lsb,
            unsigned width) {
    VIXL_ASSERT(width >= 1);
    VIXL_ASSERT(lsb + width <= static_cast<unsigned>(rn.GetSizeInBits()));
    sbfm(rd, rn, lsb, lsb + width - 1);
  }

  // Signed extend byte.
  void sxtb(const Register& rd, const Register& rn) { sbfm(rd, rn, 0, 7); }

  // Signed extend halfword.
  void sxth(const Register& rd, const Register& rn) { sbfm(rd, rn, 0, 15); }

  // Signed extend word.
  void sxtw(const Register& rd, const Register& rn) { sbfm(rd, rn, 0, 31); }

  // Ubfm aliases.

  // Logical shift left.
  void lsl(const Register& rd, const Register& rn, unsigned shift) {
    unsigned reg_size = rd.GetSizeInBits();
    VIXL_ASSERT(shift < reg_size);
    ubfm(rd, rn, (reg_size - shift) % reg_size, reg_size - shift - 1);
  }

  // Logical shift right.
  void lsr(const Register& rd, const Register& rn, unsigned shift) {
    VIXL_ASSERT(shift < static_cast<unsigned>(rd.GetSizeInBits()));
    ubfm(rd, rn, shift, rd.GetSizeInBits() - 1);
  }

  // Unsigned bitfield insert with zero at right.
  void ubfiz(const Register& rd,
             const Register& rn,
             unsigned lsb,
             unsigned width) {
    VIXL_ASSERT(width >= 1);
    VIXL_ASSERT(lsb + width <= static_cast<unsigned>(rn.GetSizeInBits()));
    ubfm(rd,
         rn,
         (rd.GetSizeInBits() - lsb) & (rd.GetSizeInBits() - 1),
         width - 1);
  }

  // Unsigned bitfield extract.
  void ubfx(const Register& rd,
            const Register& rn,
            unsigned lsb,
            unsigned width) {
    VIXL_ASSERT(width >= 1);
    VIXL_ASSERT(lsb + width <= static_cast<unsigned>(rn.GetSizeInBits()));
    ubfm(rd, rn, lsb, lsb + width - 1);
  }

  // Unsigned extend byte.
  void uxtb(const Register& rd, const Register& rn) { ubfm(rd, rn, 0, 7); }

  // Unsigned extend halfword.
  void uxth(const Register& rd, const Register& rn) { ubfm(rd, rn, 0, 15); }

  // Unsigned extend word.
  void uxtw(const Register& rd, const Register& rn) { ubfm(rd, rn, 0, 31); }

  // Extract.
  void extr(const Register& rd,
            const Register& rn,
            const Register& rm,
            unsigned lsb);

  // Conditional select: rd = cond ? rn : rm.
  void csel(const Register& rd,
            const Register& rn,
            const Register& rm,
            Condition cond);

  // Conditional select increment: rd = cond ? rn : rm + 1.
  void csinc(const Register& rd,
             const Register& rn,
             const Register& rm,
             Condition cond);

  // Conditional select inversion: rd = cond ? rn : ~rm.
  void csinv(const Register& rd,
             const Register& rn,
             const Register& rm,
             Condition cond);

  // Conditional select negation: rd = cond ? rn : -rm.
  void csneg(const Register& rd,
             const Register& rn,
             const Register& rm,
             Condition cond);

  // Conditional set: rd = cond ? 1 : 0.
  void cset(const Register& rd, Condition cond);

  // Conditional set mask: rd = cond ? -1 : 0.
  void csetm(const Register& rd, Condition cond);

  // Conditional increment: rd = cond ? rn + 1 : rn.
  void cinc(const Register& rd, const Register& rn, Condition cond);

  // Conditional invert: rd = cond ? ~rn : rn.
  void cinv(const Register& rd, const Register& rn, Condition cond);

  // Conditional negate: rd = cond ? -rn : rn.
  void cneg(const Register& rd, const Register& rn, Condition cond);

  // Rotate right.
  void ror(const Register& rd, const Register& rs, unsigned shift) {
    extr(rd, rs, rs, shift);
  }

  // Conditional comparison.

  // Conditional compare negative.
  void ccmn(const Register& rn,
            const Operand& operand,
            StatusFlags nzcv,
            Condition cond);

  // Conditional compare.
  void ccmp(const Register& rn,
            const Operand& operand,
            StatusFlags nzcv,
            Condition cond);

  // CRC-32 checksum from byte.
  void crc32b(const Register& wd, const Register& wn, const Register& wm);

  // CRC-32 checksum from half-word.
  void crc32h(const Register& wd, const Register& wn, const Register& wm);

  // CRC-32 checksum from word.
  void crc32w(const Register& wd, const Register& wn, const Register& wm);

  // CRC-32 checksum from double word.
  void crc32x(const Register& wd, const Register& wn, const Register& xm);

  // CRC-32 C checksum from byte.
  void crc32cb(const Register& wd, const Register& wn, const Register& wm);

  // CRC-32 C checksum from half-word.
  void crc32ch(const Register& wd, const Register& wn, const Register& wm);

  // CRC-32 C checksum from word.
  void crc32cw(const Register& wd, const Register& wn, const Register& wm);

  // CRC-32C checksum from double word.
  void crc32cx(const Register& wd, const Register& wn, const Register& xm);

  // Multiply.
  void mul(const Register& rd, const Register& rn, const Register& rm);

  // Negated multiply.
  void mneg(const Register& rd, const Register& rn, const Register& rm);

  // Signed long multiply: 32 x 32 -> 64-bit.
  void smull(const Register& xd, const Register& wn, const Register& wm);

  // Signed multiply high: 64 x 64 -> 64-bit <127:64>.
  void smulh(const Register& xd, const Register& xn, const Register& xm);

  // Multiply and accumulate.
  void madd(const Register& rd,
            const Register& rn,
            const Register& rm,
            const Register& ra);

  // Multiply and subtract.
  void msub(const Register& rd,
            const Register& rn,
            const Register& rm,
            const Register& ra);

  // Signed long multiply and accumulate: 32 x 32 + 64 -> 64-bit.
  void smaddl(const Register& xd,
              const Register& wn,
              const Register& wm,
              const Register& xa);

  // Unsigned long multiply and accumulate: 32 x 32 + 64 -> 64-bit.
  void umaddl(const Register& xd,
              const Register& wn,
              const Register& wm,
              const Register& xa);

  // Unsigned long multiply: 32 x 32 -> 64-bit.
  void umull(const Register& xd, const Register& wn, const Register& wm) {
    umaddl(xd, wn, wm, xzr);
  }

  // Unsigned multiply high: 64 x 64 -> 64-bit <127:64>.
  void umulh(const Register& xd, const Register& xn, const Register& xm);

  // Signed long multiply and subtract: 64 - (32 x 32) -> 64-bit.
  void smsubl(const Register& xd,
              const Register& wn,
              const Register& wm,
              const Register& xa);

  // Unsigned long multiply and subtract: 64 - (32 x 32) -> 64-bit.
  void umsubl(const Register& xd,
              const Register& wn,
              const Register& wm,
              const Register& xa);

  // Signed integer divide.
  void sdiv(const Register& rd, const Register& rn, const Register& rm);

  // Unsigned integer divide.
  void udiv(const Register& rd, const Register& rn, const Register& rm);

  // Bit reverse.
  void rbit(const Register& rd, const Register& rn);

  // Reverse bytes in 16-bit half words.
  void rev16(const Register& rd, const Register& rn);

  // Reverse bytes in 32-bit words.
  void rev32(const Register& xd, const Register& xn);

  // Reverse bytes in 64-bit general purpose register, an alias for rev
  // [Armv8.2].
  void rev64(const Register& xd, const Register& xn) {
    VIXL_ASSERT(xd.Is64Bits() && xn.Is64Bits());
    rev(xd, xn);
  }

  // Reverse bytes.
  void rev(const Register& rd, const Register& rn);

  // Count leading zeroes.
  void clz(const Register& rd, const Register& rn);

  // Count leading sign bits.
  void cls(const Register& rd, const Register& rn);

  // Pointer Authentication Code for Instruction address, using key A [Armv8.3].
  void pacia(const Register& xd, const Register& rn);

  // Pointer Authentication Code for Instruction address, using key A and a
  // modifier of zero [Armv8.3].
  void paciza(const Register& xd);

  // Pointer Authentication Code for Instruction address, using key A, with
  // address in x17 and modifier in x16 [Armv8.3].
  void pacia1716();

  // Pointer Authentication Code for Instruction address, using key A, with
  // address in LR and modifier in SP [Armv8.3].
  void paciasp();

  // Pointer Authentication Code for Instruction address, using key A, with
  // address in LR and a modifier of zero [Armv8.3].
  void paciaz();

  // Pointer Authentication Code for Instruction address, using key B [Armv8.3].
  void pacib(const Register& xd, const Register& xn);

  // Pointer Authentication Code for Instruction address, using key B and a
  // modifier of zero [Armv8.3].
  void pacizb(const Register& xd);

  // Pointer Authentication Code for Instruction address, using key B, with
  // address in x17 and modifier in x16 [Armv8.3].
  void pacib1716();

  // Pointer Authentication Code for Instruction address, using key B, with
  // address in LR and modifier in SP [Armv8.3].
  void pacibsp();

  // Pointer Authentication Code for Instruction address, using key B, with
  // address in LR and a modifier of zero [Armv8.3].
  void pacibz();

  // Pointer Authentication Code for Data address, using key A [Armv8.3].
  void pacda(const Register& xd, const Register& xn);

  // Pointer Authentication Code for Data address, using key A and a modifier of
  // zero [Armv8.3].
  void pacdza(const Register& xd);

  // Pointer Authentication Code for Data address, using key B [Armv8.3].
  void pacdb(const Register& xd, const Register& xn);

  // Pointer Authentication Code for Data address, using key B and a modifier of
  // zero [Armv8.3].
  void pacdzb(const Register& xd);

  // Pointer Authentication Code, using Generic key [Armv8.3].
  void pacga(const Register& xd, const Register& xn, const Register& xm);

  // Authenticate Instruction address, using key A [Armv8.3].
  void autia(const Register& xd, const Register& xn);

  // Authenticate Instruction address, using key A and a modifier of zero
  // [Armv8.3].
  void autiza(const Register& xd);

  // Authenticate Instruction address, using key A, with address in x17 and
  // modifier in x16 [Armv8.3].
  void autia1716();

  // Authenticate Instruction address, using key A, with address in LR and
  // modifier in SP [Armv8.3].
  void autiasp();

  // Authenticate Instruction address, using key A, with address in LR and a
  // modifier of zero [Armv8.3].
  void autiaz();

  // Authenticate Instruction address, using key B [Armv8.3].
  void autib(const Register& xd, const Register& xn);

  // Authenticate Instruction address, using key B and a modifier of zero
  // [Armv8.3].
  void autizb(const Register& xd);

  // Authenticate Instruction address, using key B, with address in x17 and
  // modifier in x16 [Armv8.3].
  void autib1716();

  // Authenticate Instruction address, using key B, with address in LR and
  // modifier in SP [Armv8.3].
  void autibsp();

  // Authenticate Instruction address, using key B, with address in LR and a
  // modifier of zero [Armv8.3].
  void autibz();

  // Authenticate Data address, using key A [Armv8.3].
  void autda(const Register& xd, const Register& xn);

  // Authenticate Data address, using key A and a modifier of zero [Armv8.3].
  void autdza(const Register& xd);

  // Authenticate Data address, using key B [Armv8.3].
  void autdb(const Register& xd, const Register& xn);

  // Authenticate Data address, using key B and a modifier of zero [Armv8.3].
  void autdzb(const Register& xd);

  // Strip Pointer Authentication Code of Data address [Armv8.3].
  void xpacd(const Register& xd);

  // Strip Pointer Authentication Code of Instruction address [Armv8.3].
  void xpaci(const Register& xd);

  // Strip Pointer Authentication Code of Instruction address in LR [Armv8.3].
  void xpaclri();

  // Memory instructions.

  // Load integer or FP register.
  void ldr(const CPURegister& rt,
           const MemOperand& src,
           LoadStoreScalingOption option = PreferScaledOffset);

  // Store integer or FP register.
  void str(const CPURegister& rt,
           const MemOperand& dst,
           LoadStoreScalingOption option = PreferScaledOffset);

  // Load word with sign extension.
  void ldrsw(const Register& xt,
             const MemOperand& src,
             LoadStoreScalingOption option = PreferScaledOffset);

  // Load byte.
  void ldrb(const Register& rt,
            const MemOperand& src,
            LoadStoreScalingOption option = PreferScaledOffset);

  // Store byte.
  void strb(const Register& rt,
            const MemOperand& dst,
            LoadStoreScalingOption option = PreferScaledOffset);

  // Load byte with sign extension.
  void ldrsb(const Register& rt,
             const MemOperand& src,
             LoadStoreScalingOption option = PreferScaledOffset);

  // Load half-word.
  void ldrh(const Register& rt,
            const MemOperand& src,
            LoadStoreScalingOption option = PreferScaledOffset);

  // Store half-word.
  void strh(const Register& rt,
            const MemOperand& dst,
            LoadStoreScalingOption option = PreferScaledOffset);

  // Load half-word with sign extension.
  void ldrsh(const Register& rt,
             const MemOperand& src,
             LoadStoreScalingOption option = PreferScaledOffset);

  // Load integer or FP register (with unscaled offset).
  void ldur(const CPURegister& rt,
            const MemOperand& src,
            LoadStoreScalingOption option = PreferUnscaledOffset);

  // Store integer or FP register (with unscaled offset).
  void stur(const CPURegister& rt,
            const MemOperand& src,
            LoadStoreScalingOption option = PreferUnscaledOffset);

  // Load word with sign extension.
  void ldursw(const Register& xt,
              const MemOperand& src,
              LoadStoreScalingOption option = PreferUnscaledOffset);

  // Load byte (with unscaled offset).
  void ldurb(const Register& rt,
             const MemOperand& src,
             LoadStoreScalingOption option = PreferUnscaledOffset);

  // Store byte (with unscaled offset).
  void sturb(const Register& rt,
             const MemOperand& dst,
             LoadStoreScalingOption option = PreferUnscaledOffset);

  // Load byte with sign extension (and unscaled offset).
  void ldursb(const Register& rt,
              const MemOperand& src,
              LoadStoreScalingOption option = PreferUnscaledOffset);

  // Load half-word (with unscaled offset).
  void ldurh(const Register& rt,
             const MemOperand& src,
             LoadStoreScalingOption option = PreferUnscaledOffset);

  // Store half-word (with unscaled offset).
  void sturh(const Register& rt,
             const MemOperand& dst,
             LoadStoreScalingOption option = PreferUnscaledOffset);

  // Load half-word with sign extension (and unscaled offset).
  void ldursh(const Register& rt,
              const MemOperand& src,
              LoadStoreScalingOption option = PreferUnscaledOffset);

  // Load double-word with pointer authentication, using data key A and a
  // modifier of zero [Armv8.3].
  void ldraa(const Register& xt, const MemOperand& src);

  // Load double-word with pointer authentication, using data key B and a
  // modifier of zero [Armv8.3].
  void ldrab(const Register& xt, const MemOperand& src);

  // Load integer or FP register pair.
  void ldp(const CPURegister& rt,
           const CPURegister& rt2,
           const MemOperand& src);

  // Store integer or FP register pair.
  void stp(const CPURegister& rt,
           const CPURegister& rt2,
           const MemOperand& dst);

  // Load word pair with sign extension.
  void ldpsw(const Register& xt, const Register& xt2, const MemOperand& src);

  // Load integer or FP register pair, non-temporal.
  void ldnp(const CPURegister& rt,
            const CPURegister& rt2,
            const MemOperand& src);

  // Store integer or FP register pair, non-temporal.
  void stnp(const CPURegister& rt,
            const CPURegister& rt2,
            const MemOperand& dst);

  // Load integer or FP register from literal pool.
  void ldr(const CPURegister& rt, RawLiteral* literal);

  // Load word with sign extension from literal pool.
  void ldrsw(const Register& xt, RawLiteral* literal);

  // Load integer or FP register from pc + imm19 << 2.
  void ldr(const CPURegister& rt, int64_t imm19);

  // Load word with sign extension from pc + imm19 << 2.
  void ldrsw(const Register& xt, int64_t imm19);

  // Store exclusive byte.
  void stxrb(const Register& rs, const Register& rt, const MemOperand& dst);

  // Store exclusive half-word.
  void stxrh(const Register& rs, const Register& rt, const MemOperand& dst);

  // Store exclusive register.
  void stxr(const Register& rs, const Register& rt, const MemOperand& dst);

  // Load exclusive byte.
  void ldxrb(const Register& rt, const MemOperand& src);

  // Load exclusive half-word.
  void ldxrh(const Register& rt, const MemOperand& src);

  // Load exclusive register.
  void ldxr(const Register& rt, const MemOperand& src);

  // Store exclusive register pair.
  void stxp(const Register& rs,
            const Register& rt,
            const Register& rt2,
            const MemOperand& dst);

  // Load exclusive register pair.
  void ldxp(const Register& rt, const Register& rt2, const MemOperand& src);

  // Store-release exclusive byte.
  void stlxrb(const Register& rs, const Register& rt, const MemOperand& dst);

  // Store-release exclusive half-word.
  void stlxrh(const Register& rs, const Register& rt, const MemOperand& dst);

  // Store-release exclusive register.
  void stlxr(const Register& rs, const Register& rt, const MemOperand& dst);

  // Load-acquire exclusive byte.
  void ldaxrb(const Register& rt, const MemOperand& src);

  // Load-acquire exclusive half-word.
  void ldaxrh(const Register& rt, const MemOperand& src);

  // Load-acquire exclusive register.
  void ldaxr(const Register& rt, const MemOperand& src);

  // Store-release exclusive register pair.
  void stlxp(const Register& rs,
             const Register& rt,
             const Register& rt2,
             const MemOperand& dst);

  // Load-acquire exclusive register pair.
  void ldaxp(const Register& rt, const Register& rt2, const MemOperand& src);

  // Store-release byte.
  void stlrb(const Register& rt, const MemOperand& dst);

  // Store-release half-word.
  void stlrh(const Register& rt, const MemOperand& dst);

  // Store-release register.
  void stlr(const Register& rt, const MemOperand& dst);

  // Load-acquire byte.
  void ldarb(const Register& rt, const MemOperand& src);

  // Load-acquire half-word.
  void ldarh(const Register& rt, const MemOperand& src);

  // Load-acquire register.
  void ldar(const Register& rt, const MemOperand& src);

  // Store LORelease byte [Armv8.1].
  void stllrb(const Register& rt, const MemOperand& dst);

  // Store LORelease half-word [Armv8.1].
  void stllrh(const Register& rt, const MemOperand& dst);

  // Store LORelease register [Armv8.1].
  void stllr(const Register& rt, const MemOperand& dst);

  // Load LORelease byte [Armv8.1].
  void ldlarb(const Register& rt, const MemOperand& src);

  // Load LORelease half-word [Armv8.1].
  void ldlarh(const Register& rt, const MemOperand& src);

  // Load LORelease register [Armv8.1].
  void ldlar(const Register& rt, const MemOperand& src);

  // Compare and Swap word or doubleword in memory [Armv8.1].
  void cas(const Register& rs, const Register& rt, const MemOperand& src);

  // Compare and Swap word or doubleword in memory [Armv8.1].
  void casa(const Register& rs, const Register& rt, const MemOperand& src);

  // Compare and Swap word or doubleword in memory [Armv8.1].
  void casl(const Register& rs, const Register& rt, const MemOperand& src);

  // Compare and Swap word or doubleword in memory [Armv8.1].
  void casal(const Register& rs, const Register& rt, const MemOperand& src);

  // Compare and Swap byte in memory [Armv8.1].
  void casb(const Register& rs, const Register& rt, const MemOperand& src);

  // Compare and Swap byte in memory [Armv8.1].
  void casab(const Register& rs, const Register& rt, const MemOperand& src);

  // Compare and Swap byte in memory [Armv8.1].
  void caslb(const Register& rs, const Register& rt, const MemOperand& src);

  // Compare and Swap byte in memory [Armv8.1].
  void casalb(const Register& rs, const Register& rt, const MemOperand& src);

  // Compare and Swap halfword in memory [Armv8.1].
  void cash(const Register& rs, const Register& rt, const MemOperand& src);

  // Compare and Swap halfword in memory [Armv8.1].
  void casah(const Register& rs, const Register& rt, const MemOperand& src);

  // Compare and Swap halfword in memory [Armv8.1].
  void caslh(const Register& rs, const Register& rt, const MemOperand& src);

  // Compare and Swap halfword in memory [Armv8.1].
  void casalh(const Register& rs, const Register& rt, const MemOperand& src);

  // Compare and Swap Pair of words or doublewords in memory [Armv8.1].
  void casp(const Register& rs,
            const Register& rs2,
            const Register& rt,
            const Register& rt2,
            const MemOperand& src);

  // Compare and Swap Pair of words or doublewords in memory [Armv8.1].
  void caspa(const Register& rs,
             const Register& rs2,
             const Register& rt,
             const Register& rt2,
             const MemOperand& src);

  // Compare and Swap Pair of words or doublewords in memory [Armv8.1].
  void caspl(const Register& rs,
             const Register& rs2,
             const Register& rt,
             const Register& rt2,
             const MemOperand& src);

  // Compare and Swap Pair of words or doublewords in memory [Armv8.1].
  void caspal(const Register& rs,
              const Register& rs2,
              const Register& rt,
              const Register& rt2,
              const MemOperand& src);

  // Store-release byte (with unscaled offset) [Armv8.4].
  void stlurb(const Register& rt, const MemOperand& dst);

  // Load-acquire RCpc Register byte (with unscaled offset) [Armv8.4].
  void ldapurb(const Register& rt, const MemOperand& src);

  // Load-acquire RCpc Register signed byte (with unscaled offset) [Armv8.4].
  void ldapursb(const Register& rt, const MemOperand& src);

  // Store-release half-word (with unscaled offset) [Armv8.4].
  void stlurh(const Register& rt, const MemOperand& dst);

  // Load-acquire RCpc Register half-word (with unscaled offset) [Armv8.4].
  void ldapurh(const Register& rt, const MemOperand& src);

  // Load-acquire RCpc Register signed half-word (with unscaled offset)
  // [Armv8.4].
  void ldapursh(const Register& rt, const MemOperand& src);

  // Store-release word or double-word (with unscaled offset) [Armv8.4].
  void stlur(const Register& rt, const MemOperand& dst);

  // Load-acquire RCpc Register word or double-word (with unscaled offset)
  // [Armv8.4].
  void ldapur(const Register& rt, const MemOperand& src);

  // Load-acquire RCpc Register signed word (with unscaled offset) [Armv8.4].
  void ldapursw(const Register& xt, const MemOperand& src);

  // Atomic add on byte in memory [Armv8.1]
  void ldaddb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic add on byte in memory, with Load-acquire semantics [Armv8.1]
  void ldaddab(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic add on byte in memory, with Store-release semantics [Armv8.1]
  void ldaddlb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic add on byte in memory, with Load-acquire and Store-release semantics
  // [Armv8.1]
  void ldaddalb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic add on halfword in memory [Armv8.1]
  void ldaddh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic add on halfword in memory, with Load-acquire semantics [Armv8.1]
  void ldaddah(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic add on halfword in memory, with Store-release semantics [Armv8.1]
  void ldaddlh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic add on halfword in memory, with Load-acquire and Store-release
  // semantics [Armv8.1]
  void ldaddalh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic add on word or doubleword in memory [Armv8.1]
  void ldadd(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic add on word or doubleword in memory, with Load-acquire semantics
  // [Armv8.1]
  void ldadda(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic add on word or doubleword in memory, with Store-release semantics
  // [Armv8.1]
  void ldaddl(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic add on word or doubleword in memory, with Load-acquire and
  // Store-release semantics [Armv8.1]
  void ldaddal(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit clear on byte in memory [Armv8.1]
  void ldclrb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit clear on byte in memory, with Load-acquire semantics [Armv8.1]
  void ldclrab(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit clear on byte in memory, with Store-release semantics [Armv8.1]
  void ldclrlb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit clear on byte in memory, with Load-acquire and Store-release
  // semantics [Armv8.1]
  void ldclralb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit clear on halfword in memory [Armv8.1]
  void ldclrh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit clear on halfword in memory, with Load-acquire semantics
  // [Armv8.1]
  void ldclrah(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit clear on halfword in memory, with Store-release semantics
  // [Armv8.1]
  void ldclrlh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit clear on halfword in memory, with Load-acquire and Store-release
  // semantics [Armv8.1]
  void ldclralh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit clear on word or doubleword in memory [Armv8.1]
  void ldclr(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit clear on word or doubleword in memory, with Load-acquire
  // semantics [Armv8.1]
  void ldclra(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit clear on word or doubleword in memory, with Store-release
  // semantics [Armv8.1]
  void ldclrl(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit clear on word or doubleword in memory, with Load-acquire and
  // Store-release semantics [Armv8.1]
  void ldclral(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic exclusive OR on byte in memory [Armv8.1]
  void ldeorb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic exclusive OR on byte in memory, with Load-acquire semantics
  // [Armv8.1]
  void ldeorab(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic exclusive OR on byte in memory, with Store-release semantics
  // [Armv8.1]
  void ldeorlb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic exclusive OR on byte in memory, with Load-acquire and Store-release
  // semantics [Armv8.1]
  void ldeoralb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic exclusive OR on halfword in memory [Armv8.1]
  void ldeorh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic exclusive OR on halfword in memory, with Load-acquire semantics
  // [Armv8.1]
  void ldeorah(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic exclusive OR on halfword in memory, with Store-release semantics
  // [Armv8.1]
  void ldeorlh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic exclusive OR on halfword in memory, with Load-acquire and
  // Store-release semantics [Armv8.1]
  void ldeoralh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic exclusive OR on word or doubleword in memory [Armv8.1]
  void ldeor(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic exclusive OR on word or doubleword in memory, with Load-acquire
  // semantics [Armv8.1]
  void ldeora(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic exclusive OR on word or doubleword in memory, with Store-release
  // semantics [Armv8.1]
  void ldeorl(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic exclusive OR on word or doubleword in memory, with Load-acquire and
  // Store-release semantics [Armv8.1]
  void ldeoral(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit set on byte in memory [Armv8.1]
  void ldsetb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit set on byte in memory, with Load-acquire semantics [Armv8.1]
  void ldsetab(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit set on byte in memory, with Store-release semantics [Armv8.1]
  void ldsetlb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit set on byte in memory, with Load-acquire and Store-release
  // semantics [Armv8.1]
  void ldsetalb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit set on halfword in memory [Armv8.1]
  void ldseth(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit set on halfword in memory, with Load-acquire semantics [Armv8.1]
  void ldsetah(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit set on halfword in memory, with Store-release semantics
  // [Armv8.1]
  void ldsetlh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit set on halfword in memory, with Load-acquire and Store-release
  // semantics [Armv8.1]
  void ldsetalh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit set on word or doubleword in memory [Armv8.1]
  void ldset(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit set on word or doubleword in memory, with Load-acquire semantics
  // [Armv8.1]
  void ldseta(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit set on word or doubleword in memory, with Store-release
  // semantics [Armv8.1]
  void ldsetl(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic bit set on word or doubleword in memory, with Load-acquire and
  // Store-release semantics [Armv8.1]
  void ldsetal(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed maximum on byte in memory [Armv8.1]
  void ldsmaxb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed maximum on byte in memory, with Load-acquire semantics
  // [Armv8.1]
  void ldsmaxab(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed maximum on byte in memory, with Store-release semantics
  // [Armv8.1]
  void ldsmaxlb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed maximum on byte in memory, with Load-acquire and
  // Store-release semantics [Armv8.1]
  void ldsmaxalb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed maximum on halfword in memory [Armv8.1]
  void ldsmaxh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed maximum on halfword in memory, with Load-acquire semantics
  // [Armv8.1]
  void ldsmaxah(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed maximum on halfword in memory, with Store-release semantics
  // [Armv8.1]
  void ldsmaxlh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed maximum on halfword in memory, with Load-acquire and
  // Store-release semantics [Armv8.1]
  void ldsmaxalh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed maximum on word or doubleword in memory [Armv8.1]
  void ldsmax(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed maximum on word or doubleword in memory, with Load-acquire
  // semantics [Armv8.1]
  void ldsmaxa(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed maximum on word or doubleword in memory, with Store-release
  // semantics [Armv8.1]
  void ldsmaxl(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed maximum on word or doubleword in memory, with Load-acquire
  // and Store-release semantics [Armv8.1]
  void ldsmaxal(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed minimum on byte in memory [Armv8.1]
  void ldsminb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed minimum on byte in memory, with Load-acquire semantics
  // [Armv8.1]
  void ldsminab(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed minimum on byte in memory, with Store-release semantics
  // [Armv8.1]
  void ldsminlb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed minimum on byte in memory, with Load-acquire and
  // Store-release semantics [Armv8.1]
  void ldsminalb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed minimum on halfword in memory [Armv8.1]
  void ldsminh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed minimum on halfword in memory, with Load-acquire semantics
  // [Armv8.1]
  void ldsminah(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed minimum on halfword in memory, with Store-release semantics
  // [Armv8.1]
  void ldsminlh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed minimum on halfword in memory, with Load-acquire and
  // Store-release semantics [Armv8.1]
  void ldsminalh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed minimum on word or doubleword in memory [Armv8.1]
  void ldsmin(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed minimum on word or doubleword in memory, with Load-acquire
  // semantics [Armv8.1]
  void ldsmina(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed minimum on word or doubleword in memory, with Store-release
  // semantics [Armv8.1]
  void ldsminl(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic signed minimum on word or doubleword in memory, with Load-acquire
  // and Store-release semantics [Armv8.1]
  void ldsminal(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned maximum on byte in memory [Armv8.1]
  void ldumaxb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned maximum on byte in memory, with Load-acquire semantics
  // [Armv8.1]
  void ldumaxab(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned maximum on byte in memory, with Store-release semantics
  // [Armv8.1]
  void ldumaxlb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned maximum on byte in memory, with Load-acquire and
  // Store-release semantics [Armv8.1]
  void ldumaxalb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned maximum on halfword in memory [Armv8.1]
  void ldumaxh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned maximum on halfword in memory, with Load-acquire semantics
  // [Armv8.1]
  void ldumaxah(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned maximum on halfword in memory, with Store-release semantics
  // [Armv8.1]
  void ldumaxlh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned maximum on halfword in memory, with Load-acquire and
  // Store-release semantics [Armv8.1]
  void ldumaxalh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned maximum on word or doubleword in memory [Armv8.1]
  void ldumax(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned maximum on word or doubleword in memory, with Load-acquire
  // semantics [Armv8.1]
  void ldumaxa(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned maximum on word or doubleword in memory, with Store-release
  // semantics [Armv8.1]
  void ldumaxl(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned maximum on word or doubleword in memory, with Load-acquire
  // and Store-release semantics [Armv8.1]
  void ldumaxal(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned minimum on byte in memory [Armv8.1]
  void lduminb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned minimum on byte in memory, with Load-acquire semantics
  // [Armv8.1]
  void lduminab(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned minimum on byte in memory, with Store-release semantics
  // [Armv8.1]
  void lduminlb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned minimum on byte in memory, with Load-acquire and
  // Store-release semantics [Armv8.1]
  void lduminalb(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned minimum on halfword in memory [Armv8.1]
  void lduminh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned minimum on halfword in memory, with Load-acquire semantics
  // [Armv8.1]
  void lduminah(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned minimum on halfword in memory, with Store-release semantics
  // [Armv8.1]
  void lduminlh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned minimum on halfword in memory, with Load-acquire and
  // Store-release semantics [Armv8.1]
  void lduminalh(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned minimum on word or doubleword in memory [Armv8.1]
  void ldumin(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned minimum on word or doubleword in memory, with Load-acquire
  // semantics [Armv8.1]
  void ldumina(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned minimum on word or doubleword in memory, with Store-release
  // semantics [Armv8.1]
  void lduminl(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic unsigned minimum on word or doubleword in memory, with Load-acquire
  // and Store-release semantics [Armv8.1]
  void lduminal(const Register& rs, const Register& rt, const MemOperand& src);

  // Atomic add on byte in memory, without return. [Armv8.1]
  void staddb(const Register& rs, const MemOperand& src);

  // Atomic add on byte in memory, with Store-release semantics and without
  // return. [Armv8.1]
  void staddlb(const Register& rs, const MemOperand& src);

  // Atomic add on halfword in memory, without return. [Armv8.1]
  void staddh(const Register& rs, const MemOperand& src);

  // Atomic add on halfword in memory, with Store-release semantics and without
  // return. [Armv8.1]
  void staddlh(const Register& rs, const MemOperand& src);

  // Atomic add on word or doubleword in memory, without return. [Armv8.1]
  void stadd(const Register& rs, const MemOperand& src);

  // Atomic add on word or doubleword in memory, with Store-release semantics
  // and without return. [Armv8.1]
  void staddl(const Register& rs, const MemOperand& src);

  // Atomic bit clear on byte in memory, without return. [Armv8.1]
  void stclrb(const Register& rs, const MemOperand& src);

  // Atomic bit clear on byte in memory, with Store-release semantics and
  // without return. [Armv8.1]
  void stclrlb(const Register& rs, const MemOperand& src);

  // Atomic bit clear on halfword in memory, without return. [Armv8.1]
  void stclrh(const Register& rs, const MemOperand& src);

  // Atomic bit clear on halfword in memory, with Store-release semantics and
  // without return. [Armv8.1]
  void stclrlh(const Register& rs, const MemOperand& src);

  // Atomic bit clear on word or doubleword in memory, without return. [Armv8.1]
  void stclr(const Register& rs, const MemOperand& src);

  // Atomic bit clear on word or doubleword in memory, with Store-release
  // semantics and without return. [Armv8.1]
  void stclrl(const Register& rs, const MemOperand& src);

  // Atomic exclusive OR on byte in memory, without return. [Armv8.1]
  void steorb(const Register& rs, const MemOperand& src);

  // Atomic exclusive OR on byte in memory, with Store-release semantics and
  // without return. [Armv8.1]
  void steorlb(const Register& rs, const MemOperand& src);

  // Atomic exclusive OR on halfword in memory, without return. [Armv8.1]
  void steorh(const Register& rs, const MemOperand& src);

  // Atomic exclusive OR on halfword in memory, with Store-release semantics
  // and without return. [Armv8.1]
  void steorlh(const Register& rs, const MemOperand& src);

  // Atomic exclusive OR on word or doubleword in memory, without return.
  // [Armv8.1]
  void steor(const Register& rs, const MemOperand& src);

  // Atomic exclusive OR on word or doubleword in memory, with Store-release
  // semantics and without return. [Armv8.1]
  void steorl(const Register& rs, const MemOperand& src);

  // Atomic bit set on byte in memory, without return. [Armv8.1]
  void stsetb(const Register& rs, const MemOperand& src);

  // Atomic bit set on byte in memory, with Store-release semantics and without
  // return. [Armv8.1]
  void stsetlb(const Register& rs, const MemOperand& src);

  // Atomic bit set on halfword in memory, without return. [Armv8.1]
  void stseth(const Register& rs, const MemOperand& src);

  // Atomic bit set on halfword in memory, with Store-release semantics and
  // without return. [Armv8.1]
  void stsetlh(const Register& rs, const MemOperand& src);

  // Atomic bit set on word or doubleword in memory, without return. [Armv8.1]
  void stset(const Register& rs, const MemOperand& src);

  // Atomic bit set on word or doubleword in memory, with Store-release
  // semantics and without return. [Armv8.1]
  void stsetl(const Register& rs, const MemOperand& src);

  // Atomic signed maximum on byte in memory, without return. [Armv8.1]
  void stsmaxb(const Register& rs, const MemOperand& src);

  // Atomic signed maximum on byte in memory, with Store-release semantics and
  // without return. [Armv8.1]
  void stsmaxlb(const Register& rs, const MemOperand& src);

  // Atomic signed maximum on halfword in memory, without return. [Armv8.1]
  void stsmaxh(const Register& rs, const MemOperand& src);

  // Atomic signed maximum on halfword in memory, with Store-release semantics
  // and without return. [Armv8.1]
  void stsmaxlh(const Register& rs, const MemOperand& src);

  // Atomic signed maximum on word or doubleword in memory, without return.
  // [Armv8.1]
  void stsmax(const Register& rs, const MemOperand& src);

  // Atomic signed maximum on word or doubleword in memory, with Store-release
  // semantics and without return. [Armv8.1]
  void stsmaxl(const Register& rs, const MemOperand& src);

  // Atomic signed minimum on byte in memory, without return. [Armv8.1]
  void stsminb(const Register& rs, const MemOperand& src);

  // Atomic signed minimum on byte in memory, with Store-release semantics and
  // without return. [Armv8.1]
  void stsminlb(const Register& rs, const MemOperand& src);

  // Atomic signed minimum on halfword in memory, without return. [Armv8.1]
  void stsminh(const Register& rs, const MemOperand& src);

  // Atomic signed minimum on halfword in memory, with Store-release semantics
  // and without return. [Armv8.1]
  void stsminlh(const Register& rs, const MemOperand& src);

  // Atomic signed minimum on word or doubleword in memory, without return.
  // [Armv8.1]
  void stsmin(const Register& rs, const MemOperand& src);

  // Atomic signed minimum on word or doubleword in memory, with Store-release
  // semantics and without return. semantics [Armv8.1]
  void stsminl(const Register& rs, const MemOperand& src);

  // Atomic unsigned maximum on byte in memory, without return. [Armv8.1]
  void stumaxb(const Register& rs, const MemOperand& src);

  // Atomic unsigned maximum on byte in memory, with Store-release semantics and
  // without return. [Armv8.1]
  void stumaxlb(const Register& rs, const MemOperand& src);

  // Atomic unsigned maximum on halfword in memory, without return. [Armv8.1]
  void stumaxh(const Register& rs, const MemOperand& src);

  // Atomic unsigned maximum on halfword in memory, with Store-release semantics
  // and without return. [Armv8.1]
  void stumaxlh(const Register& rs, const MemOperand& src);

  // Atomic unsigned maximum on word or doubleword in memory, without return.
  // [Armv8.1]
  void stumax(const Register& rs, const MemOperand& src);

  // Atomic unsigned maximum on word or doubleword in memory, with Store-release
  // semantics and without return. [Armv8.1]
  void stumaxl(const Register& rs, const MemOperand& src);

  // Atomic unsigned minimum on byte in memory, without return. [Armv8.1]
  void stuminb(const Register& rs, const MemOperand& src);

  // Atomic unsigned minimum on byte in memory, with Store-release semantics and
  // without return. [Armv8.1]
  void stuminlb(const Register& rs, const MemOperand& src);

  // Atomic unsigned minimum on halfword in memory, without return. [Armv8.1]
  void stuminh(const Register& rs, const MemOperand& src);

  // Atomic unsigned minimum on halfword in memory, with Store-release semantics
  // and without return. [Armv8.1]
  void stuminlh(const Register& rs, const MemOperand& src);

  // Atomic unsigned minimum on word or doubleword in memory, without return.
  // [Armv8.1]
  void stumin(const Register& rs, const MemOperand& src);

  // Atomic unsigned minimum on word or doubleword in memory, with Store-release
  // semantics and without return. [Armv8.1]
  void stuminl(const Register& rs, const MemOperand& src);

  // Swap byte in memory [Armv8.1]
  void swpb(const Register& rs, const Register& rt, const MemOperand& src);

  // Swap byte in memory, with Load-acquire semantics [Armv8.1]
  void swpab(const Register& rs, const Register& rt, const MemOperand& src);

  // Swap byte in memory, with Store-release semantics [Armv8.1]
  void swplb(const Register& rs, const Register& rt, const MemOperand& src);

  // Swap byte in memory, with Load-acquire and Store-release semantics
  // [Armv8.1]
  void swpalb(const Register& rs, const Register& rt, const MemOperand& src);

  // Swap halfword in memory [Armv8.1]
  void swph(const Register& rs, const Register& rt, const MemOperand& src);

  // Swap halfword in memory, with Load-acquire semantics [Armv8.1]
  void swpah(const Register& rs, const Register& rt, const MemOperand& src);

  // Swap halfword in memory, with Store-release semantics [Armv8.1]
  void swplh(const Register& rs, const Register& rt, const MemOperand& src);

  // Swap halfword in memory, with Load-acquire and Store-release semantics
  // [Armv8.1]
  void swpalh(const Register& rs, const Register& rt, const MemOperand& src);

  // Swap word or doubleword in memory [Armv8.1]
  void swp(const Register& rs, const Register& rt, const MemOperand& src);

  // Swap word or doubleword in memory, with Load-acquire semantics [Armv8.1]
  void swpa(const Register& rs, const Register& rt, const MemOperand& src);

  // Swap word or doubleword in memory, with Store-release semantics [Armv8.1]
  void swpl(const Register& rs, const Register& rt, const MemOperand& src);

  // Swap word or doubleword in memory, with Load-acquire and Store-release
  // semantics [Armv8.1]
  void swpal(const Register& rs, const Register& rt, const MemOperand& src);

  // Load-Acquire RCpc Register byte [Armv8.3]
  void ldaprb(const Register& rt, const MemOperand& src);

  // Load-Acquire RCpc Register halfword [Armv8.3]
  void ldaprh(const Register& rt, const MemOperand& src);

  // Load-Acquire RCpc Register word or doubleword [Armv8.3]
  void ldapr(const Register& rt, const MemOperand& src);

  // Prefetch memory.
  void prfm(PrefetchOperation op,
            const MemOperand& addr,
            LoadStoreScalingOption option = PreferScaledOffset);

  // Prefetch memory (with unscaled offset).
  void prfum(PrefetchOperation op,
             const MemOperand& addr,
             LoadStoreScalingOption option = PreferUnscaledOffset);

  // Prefetch memory in the literal pool.
  void prfm(PrefetchOperation op, RawLiteral* literal);

  // Prefetch from pc + imm19 << 2.
  void prfm(PrefetchOperation op, int64_t imm19);

  // Prefetch memory (allowing unallocated hints).
  void prfm(int op,
            const MemOperand& addr,
            LoadStoreScalingOption option = PreferScaledOffset);

  // Prefetch memory (with unscaled offset, allowing unallocated hints).
  void prfum(int op,
             const MemOperand& addr,
             LoadStoreScalingOption option = PreferUnscaledOffset);

  // Prefetch memory in the literal pool (allowing unallocated hints).
  void prfm(int op, RawLiteral* literal);

  // Prefetch from pc + imm19 << 2 (allowing unallocated hints).
  void prfm(int op, int64_t imm19);

  // Move instructions. The default shift of -1 indicates that the move
  // instruction will calculate an appropriate 16-bit immediate and left shift
  // that is equal to the 64-bit immediate argument. If an explicit left shift
  // is specified (0, 16, 32 or 48), the immediate must be a 16-bit value.
  //
  // For movk, an explicit shift can be used to indicate which half word should
  // be overwritten, eg. movk(x0, 0, 0) will overwrite the least-significant
  // half word with zero, whereas movk(x0, 0, 48) will overwrite the
  // most-significant.

  // Move immediate and keep.
  void movk(const Register& rd, uint64_t imm, int shift = -1) {
    MoveWide(rd, imm, shift, MOVK);
  }

  // Move inverted immediate.
  void movn(const Register& rd, uint64_t imm, int shift = -1) {
    MoveWide(rd, imm, shift, MOVN);
  }

  // Move immediate.
  void movz(const Register& rd, uint64_t imm, int shift = -1) {
    MoveWide(rd, imm, shift, MOVZ);
  }

  // Move immediate, aliases for movz, movn, orr.
  void mov(const Register& rd, uint64_t imm) {
    if (!OneInstrMoveImmediateHelper(this, rd, imm)) {
      VIXL_UNIMPLEMENTED();
    }
  }

  // Misc instructions.

  // Monitor debug-mode breakpoint.
  void brk(int code);

  // Halting debug-mode breakpoint.
  void hlt(int code);

  // Generate exception targeting EL1.
  void svc(int code);

  // Generate undefined instruction exception.
  void udf(int code);

  // Move register to register.
  void mov(const Register& rd, const Register& rn);

  // Move inverted operand to register.
  void mvn(const Register& rd, const Operand& operand);

  // System instructions.

  // Move to register from system register.
  void mrs(const Register& xt, SystemRegister sysreg);

  // Move from register to system register.
  void msr(SystemRegister sysreg, const Register& xt);

  // Invert carry flag [Armv8.4].
  void cfinv();

  // Convert floating-point condition flags from alternative format to Arm
  // format [Armv8.5].
  void xaflag();

  // Convert floating-point condition flags from Arm format to alternative
  // format [Armv8.5].
  void axflag();

  // System instruction.
  void sys(int op1, int crn, int crm, int op2, const Register& xt = xzr);

  // System instruction with pre-encoded op (op1:crn:crm:op2).
  void sys(int op, const Register& xt = xzr);

  // System instruction with result.
  void sysl(int op, const Register& xt = xzr);

  // System data cache operation.
  void dc(DataCacheOp op, const Register& rt);

  // System instruction cache operation.
  void ic(InstructionCacheOp op, const Register& rt);

  // System hint (named type).
  void hint(SystemHint code);

  // System hint (numbered type).
  void hint(int imm7);

  // Clear exclusive monitor.
  void clrex(int imm4 = 0xf);

  // Data memory barrier.
  void dmb(BarrierDomain domain, BarrierType type);

  // Data synchronization barrier.
  void dsb(BarrierDomain domain, BarrierType type);

  // Instruction synchronization barrier.
  void isb();

  // Error synchronization barrier.
  void esb();

  // Conditional speculation dependency barrier.
  void csdb();

  // No-op.
  void nop() { hint(NOP); }

  // Branch target identification.
  void bti(BranchTargetIdentifier id);

  // FP and NEON instructions.

  // Move double precision immediate to FP register.
  void fmov(const VRegister& vd, double imm);

  // Move single precision immediate to FP register.
  void fmov(const VRegister& vd, float imm);

  // Move half precision immediate to FP register [Armv8.2].
  void fmov(const VRegister& vd, Float16 imm);

  // Move FP register to register.
  void fmov(const Register& rd, const VRegister& fn);

  // Move register to FP register.
  void fmov(const VRegister& vd, const Register& rn);

  // Move FP register to FP register.
  void fmov(const VRegister& vd, const VRegister& fn);

  // Move 64-bit register to top half of 128-bit FP register.
  void fmov(const VRegister& vd, int index, const Register& rn);

  // Move top half of 128-bit FP register to 64-bit register.
  void fmov(const Register& rd, const VRegister& vn, int index);

  // FP add.
  void fadd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP subtract.
  void fsub(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP multiply.
  void fmul(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP fused multiply-add.
  void fmadd(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             const VRegister& va);

  // FP fused multiply-subtract.
  void fmsub(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             const VRegister& va);

  // FP fused multiply-add and negate.
  void fnmadd(const VRegister& vd,
              const VRegister& vn,
              const VRegister& vm,
              const VRegister& va);

  // FP fused multiply-subtract and negate.
  void fnmsub(const VRegister& vd,
              const VRegister& vn,
              const VRegister& vm,
              const VRegister& va);

  // FP multiply-negate scalar.
  void fnmul(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP reciprocal exponent scalar.
  void frecpx(const VRegister& vd, const VRegister& vn);

  // FP divide.
  void fdiv(const VRegister& vd, const VRegister& fn, const VRegister& vm);

  // FP maximum.
  void fmax(const VRegister& vd, const VRegister& fn, const VRegister& vm);

  // FP minimum.
  void fmin(const VRegister& vd, const VRegister& fn, const VRegister& vm);

  // FP maximum number.
  void fmaxnm(const VRegister& vd, const VRegister& fn, const VRegister& vm);

  // FP minimum number.
  void fminnm(const VRegister& vd, const VRegister& fn, const VRegister& vm);

  // FP absolute.
  void fabs(const VRegister& vd, const VRegister& vn);

  // FP negate.
  void fneg(const VRegister& vd, const VRegister& vn);

  // FP square root.
  void fsqrt(const VRegister& vd, const VRegister& vn);

  // FP round to integer, nearest with ties to away.
  void frinta(const VRegister& vd, const VRegister& vn);

  // FP round to integer, implicit rounding.
  void frinti(const VRegister& vd, const VRegister& vn);

  // FP round to integer, toward minus infinity.
  void frintm(const VRegister& vd, const VRegister& vn);

  // FP round to integer, nearest with ties to even.
  void frintn(const VRegister& vd, const VRegister& vn);

  // FP round to integer, toward plus infinity.
  void frintp(const VRegister& vd, const VRegister& vn);

  // FP round to integer, exact, implicit rounding.
  void frintx(const VRegister& vd, const VRegister& vn);

  // FP round to integer, towards zero.
  void frintz(const VRegister& vd, const VRegister& vn);

  // FP round to 32-bit integer, exact, implicit rounding [Armv8.5].
  void frint32x(const VRegister& vd, const VRegister& vn);

  // FP round to 32-bit integer, towards zero [Armv8.5].
  void frint32z(const VRegister& vd, const VRegister& vn);

  // FP round to 64-bit integer, exact, implicit rounding [Armv8.5].
  void frint64x(const VRegister& vd, const VRegister& vn);

  // FP round to 64-bit integer, towards zero [Armv8.5].
  void frint64z(const VRegister& vd, const VRegister& vn);

  void FPCompareMacro(const VRegister& vn, double value, FPTrapFlags trap);

  void FPCompareMacro(const VRegister& vn,
                      const VRegister& vm,
                      FPTrapFlags trap);

  // FP compare registers.
  void fcmp(const VRegister& vn, const VRegister& vm);

  // FP compare immediate.
  void fcmp(const VRegister& vn, double value);

  void FPCCompareMacro(const VRegister& vn,
                       const VRegister& vm,
                       StatusFlags nzcv,
                       Condition cond,
                       FPTrapFlags trap);

  // FP conditional compare.
  void fccmp(const VRegister& vn,
             const VRegister& vm,
             StatusFlags nzcv,
             Condition cond);

  // FP signaling compare registers.
  void fcmpe(const VRegister& vn, const VRegister& vm);

  // FP signaling compare immediate.
  void fcmpe(const VRegister& vn, double value);

  // FP conditional signaling compare.
  void fccmpe(const VRegister& vn,
              const VRegister& vm,
              StatusFlags nzcv,
              Condition cond);

  // FP conditional select.
  void fcsel(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             Condition cond);

  // Common FP Convert functions.
  void NEONFPConvertToInt(const Register& rd, const VRegister& vn, Instr op);
  void NEONFPConvertToInt(const VRegister& vd, const VRegister& vn, Instr op);
  void NEONFP16ConvertToInt(const VRegister& vd, const VRegister& vn, Instr op);

  // FP convert between precisions.
  void fcvt(const VRegister& vd, const VRegister& vn);

  // FP convert to higher precision.
  void fcvtl(const VRegister& vd, const VRegister& vn);

  // FP convert to higher precision (second part).
  void fcvtl2(const VRegister& vd, const VRegister& vn);

  // FP convert to lower precision.
  void fcvtn(const VRegister& vd, const VRegister& vn);

  // FP convert to lower prevision (second part).
  void fcvtn2(const VRegister& vd, const VRegister& vn);

  // FP convert to lower precision, rounding to odd.
  void fcvtxn(const VRegister& vd, const VRegister& vn);

  // FP convert to lower precision, rounding to odd (second part).
  void fcvtxn2(const VRegister& vd, const VRegister& vn);

  // FP convert to signed integer, nearest with ties to away.
  void fcvtas(const Register& rd, const VRegister& vn);

  // FP convert to unsigned integer, nearest with ties to away.
  void fcvtau(const Register& rd, const VRegister& vn);

  // FP convert to signed integer, nearest with ties to away.
  void fcvtas(const VRegister& vd, const VRegister& vn);

  // FP convert to unsigned integer, nearest with ties to away.
  void fcvtau(const VRegister& vd, const VRegister& vn);

  // FP convert to signed integer, round towards -infinity.
  void fcvtms(const Register& rd, const VRegister& vn);

  // FP convert to unsigned integer, round towards -infinity.
  void fcvtmu(const Register& rd, const VRegister& vn);

  // FP convert to signed integer, round towards -infinity.
  void fcvtms(const VRegister& vd, const VRegister& vn);

  // FP convert to unsigned integer, round towards -infinity.
  void fcvtmu(const VRegister& vd, const VRegister& vn);

  // FP convert to signed integer, nearest with ties to even.
  void fcvtns(const Register& rd, const VRegister& vn);

  // FP JavaScript convert to signed integer, rounding toward zero [Armv8.3].
  void fjcvtzs(const Register& rd, const VRegister& vn);

  // FP convert to unsigned integer, nearest with ties to even.
  void fcvtnu(const Register& rd, const VRegister& vn);

  // FP convert to signed integer, nearest with ties to even.
  void fcvtns(const VRegister& rd, const VRegister& vn);

  // FP convert to unsigned integer, nearest with ties to even.
  void fcvtnu(const VRegister& rd, const VRegister& vn);

  // FP convert to signed integer or fixed-point, round towards zero.
  void fcvtzs(const Register& rd, const VRegister& vn, int fbits = 0);

  // FP convert to unsigned integer or fixed-point, round towards zero.
  void fcvtzu(const Register& rd, const VRegister& vn, int fbits = 0);

  // FP convert to signed integer or fixed-point, round towards zero.
  void fcvtzs(const VRegister& vd, const VRegister& vn, int fbits = 0);

  // FP convert to unsigned integer or fixed-point, round towards zero.
  void fcvtzu(const VRegister& vd, const VRegister& vn, int fbits = 0);

  // FP convert to signed integer, round towards +infinity.
  void fcvtps(const Register& rd, const VRegister& vn);

  // FP convert to unsigned integer, round towards +infinity.
  void fcvtpu(const Register& rd, const VRegister& vn);

  // FP convert to signed integer, round towards +infinity.
  void fcvtps(const VRegister& vd, const VRegister& vn);

  // FP convert to unsigned integer, round towards +infinity.
  void fcvtpu(const VRegister& vd, const VRegister& vn);

  // Convert signed integer or fixed point to FP.
  void scvtf(const VRegister& fd, const Register& rn, int fbits = 0);

  // Convert unsigned integer or fixed point to FP.
  void ucvtf(const VRegister& fd, const Register& rn, int fbits = 0);

  // Convert signed integer or fixed-point to FP.
  void scvtf(const VRegister& fd, const VRegister& vn, int fbits = 0);

  // Convert unsigned integer or fixed-point to FP.
  void ucvtf(const VRegister& fd, const VRegister& vn, int fbits = 0);

  // Unsigned absolute difference.
  void uabd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed absolute difference.
  void sabd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned absolute difference and accumulate.
  void uaba(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed absolute difference and accumulate.
  void saba(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Add.
  void add(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Subtract.
  void sub(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned halving add.
  void uhadd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed halving add.
  void shadd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned rounding halving add.
  void urhadd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed rounding halving add.
  void srhadd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned halving sub.
  void uhsub(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed halving sub.
  void shsub(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned saturating add.
  void uqadd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed saturating add.
  void sqadd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned saturating subtract.
  void uqsub(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed saturating subtract.
  void sqsub(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Add pairwise.
  void addp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Add pair of elements scalar.
  void addp(const VRegister& vd, const VRegister& vn);

  // Multiply-add to accumulator.
  void mla(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Multiply-subtract to accumulator.
  void mls(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Multiply.
  void mul(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Multiply by scalar element.
  void mul(const VRegister& vd,
           const VRegister& vn,
           const VRegister& vm,
           int vm_index);

  // Multiply-add by scalar element.
  void mla(const VRegister& vd,
           const VRegister& vn,
           const VRegister& vm,
           int vm_index);

  // Multiply-subtract by scalar element.
  void mls(const VRegister& vd,
           const VRegister& vn,
           const VRegister& vm,
           int vm_index);

  // Signed long multiply-add by scalar element.
  void smlal(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             int vm_index);

  // Signed long multiply-add by scalar element (second part).
  void smlal2(const VRegister& vd,
              const VRegister& vn,
              const VRegister& vm,
              int vm_index);

  // Unsigned long multiply-add by scalar element.
  void umlal(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             int vm_index);

  // Unsigned long multiply-add by scalar element (second part).
  void umlal2(const VRegister& vd,
              const VRegister& vn,
              const VRegister& vm,
              int vm_index);

  // Signed long multiply-sub by scalar element.
  void smlsl(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             int vm_index);

  // Signed long multiply-sub by scalar element (second part).
  void smlsl2(const VRegister& vd,
              const VRegister& vn,
              const VRegister& vm,
              int vm_index);

  // Unsigned long multiply-sub by scalar element.
  void umlsl(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             int vm_index);

  // Unsigned long multiply-sub by scalar element (second part).
  void umlsl2(const VRegister& vd,
              const VRegister& vn,
              const VRegister& vm,
              int vm_index);

  // Signed long multiply by scalar element.
  void smull(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             int vm_index);

  // Signed long multiply by scalar element (second part).
  void smull2(const VRegister& vd,
              const VRegister& vn,
              const VRegister& vm,
              int vm_index);

  // Unsigned long multiply by scalar element.
  void umull(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             int vm_index);

  // Unsigned long multiply by scalar element (second part).
  void umull2(const VRegister& vd,
              const VRegister& vn,
              const VRegister& vm,
              int vm_index);

  // Signed saturating double long multiply by element.
  void sqdmull(const VRegister& vd,
               const VRegister& vn,
               const VRegister& vm,
               int vm_index);

  // Signed saturating double long multiply by element (second part).
  void sqdmull2(const VRegister& vd,
                const VRegister& vn,
                const VRegister& vm,
                int vm_index);

  // Signed saturating doubling long multiply-add by element.
  void sqdmlal(const VRegister& vd,
               const VRegister& vn,
               const VRegister& vm,
               int vm_index);

  // Signed saturating doubling long multiply-add by element (second part).
  void sqdmlal2(const VRegister& vd,
                const VRegister& vn,
                const VRegister& vm,
                int vm_index);

  // Signed saturating doubling long multiply-sub by element.
  void sqdmlsl(const VRegister& vd,
               const VRegister& vn,
               const VRegister& vm,
               int vm_index);

  // Signed saturating doubling long multiply-sub by element (second part).
  void sqdmlsl2(const VRegister& vd,
                const VRegister& vn,
                const VRegister& vm,
                int vm_index);

  // Compare equal.
  void cmeq(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Compare signed greater than or equal.
  void cmge(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Compare signed greater than.
  void cmgt(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Compare unsigned higher.
  void cmhi(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Compare unsigned higher or same.
  void cmhs(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Compare bitwise test bits nonzero.
  void cmtst(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Compare bitwise to zero.
  void cmeq(const VRegister& vd, const VRegister& vn, int value);

  // Compare signed greater than or equal to zero.
  void cmge(const VRegister& vd, const VRegister& vn, int value);

  // Compare signed greater than zero.
  void cmgt(const VRegister& vd, const VRegister& vn, int value);

  // Compare signed less than or equal to zero.
  void cmle(const VRegister& vd, const VRegister& vn, int value);

  // Compare signed less than zero.
  void cmlt(const VRegister& vd, const VRegister& vn, int value);

  // Signed shift left by register.
  void sshl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned shift left by register.
  void ushl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed saturating shift left by register.
  void sqshl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned saturating shift left by register.
  void uqshl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed rounding shift left by register.
  void srshl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned rounding shift left by register.
  void urshl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed saturating rounding shift left by register.
  void sqrshl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned saturating rounding shift left by register.
  void uqrshl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Bitwise and.
  void and_(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Bitwise or.
  void orr(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Bitwise or immediate.
  void orr(const VRegister& vd, const int imm8, const int left_shift = 0);

  // Move register to register.
  void mov(const VRegister& vd, const VRegister& vn);

  // Bitwise orn.
  void orn(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Bitwise eor.
  void eor(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Bit clear immediate.
  void bic(const VRegister& vd, const int imm8, const int left_shift = 0);

  // Bit clear.
  void bic(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Bitwise insert if false.
  void bif(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Bitwise insert if true.
  void bit(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Bitwise select.
  void bsl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Polynomial multiply.
  void pmul(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Vector move immediate.
  void movi(const VRegister& vd,
            const uint64_t imm,
            Shift shift = LSL,
            const int shift_amount = 0);

  // Bitwise not.
  void mvn(const VRegister& vd, const VRegister& vn);

  // Vector move inverted immediate.
  void mvni(const VRegister& vd,
            const int imm8,
            Shift shift = LSL,
            const int shift_amount = 0);

  // Signed saturating accumulate of unsigned value.
  void suqadd(const VRegister& vd, const VRegister& vn);

  // Unsigned saturating accumulate of signed value.
  void usqadd(const VRegister& vd, const VRegister& vn);

  // Absolute value.
  void abs(const VRegister& vd, const VRegister& vn);

  // Signed saturating absolute value.
  void sqabs(const VRegister& vd, const VRegister& vn);

  // Negate.
  void neg(const VRegister& vd, const VRegister& vn);

  // Signed saturating negate.
  void sqneg(const VRegister& vd, const VRegister& vn);

  // Bitwise not.
  void not_(const VRegister& vd, const VRegister& vn);

  // Extract narrow.
  void xtn(const VRegister& vd, const VRegister& vn);

  // Extract narrow (second part).
  void xtn2(const VRegister& vd, const VRegister& vn);

  // Signed saturating extract narrow.
  void sqxtn(const VRegister& vd, const VRegister& vn);

  // Signed saturating extract narrow (second part).
  void sqxtn2(const VRegister& vd, const VRegister& vn);

  // Unsigned saturating extract narrow.
  void uqxtn(const VRegister& vd, const VRegister& vn);

  // Unsigned saturating extract narrow (second part).
  void uqxtn2(const VRegister& vd, const VRegister& vn);

  // Signed saturating extract unsigned narrow.
  void sqxtun(const VRegister& vd, const VRegister& vn);

  // Signed saturating extract unsigned narrow (second part).
  void sqxtun2(const VRegister& vd, const VRegister& vn);

  // Extract vector from pair of vectors.
  void ext(const VRegister& vd,
           const VRegister& vn,
           const VRegister& vm,
           int index);

  // Duplicate vector element to vector or scalar.
  void dup(const VRegister& vd, const VRegister& vn, int vn_index);

  // Move vector element to scalar.
  void mov(const VRegister& vd, const VRegister& vn, int vn_index);

  // Duplicate general-purpose register to vector.
  void dup(const VRegister& vd, const Register& rn);

  // Insert vector element from another vector element.
  void ins(const VRegister& vd,
           int vd_index,
           const VRegister& vn,
           int vn_index);

  // Move vector element to another vector element.
  void mov(const VRegister& vd,
           int vd_index,
           const VRegister& vn,
           int vn_index);

  // Insert vector element from general-purpose register.
  void ins(const VRegister& vd, int vd_index, const Register& rn);

  // Move general-purpose register to a vector element.
  void mov(const VRegister& vd, int vd_index, const Register& rn);

  // Unsigned move vector element to general-purpose register.
  void umov(const Register& rd, const VRegister& vn, int vn_index);

  // Move vector element to general-purpose register.
  void mov(const Register& rd, const VRegister& vn, int vn_index);

  // Signed move vector element to general-purpose register.
  void smov(const Register& rd, const VRegister& vn, int vn_index);

  // One-element structure load to one register.
  void ld1(const VRegister& vt, const MemOperand& src);

  // One-element structure load to two registers.
  void ld1(const VRegister& vt, const VRegister& vt2, const MemOperand& src);

  // One-element structure load to three registers.
  void ld1(const VRegister& vt,
           const VRegister& vt2,
           const VRegister& vt3,
           const MemOperand& src);

  // One-element structure load to four registers.
  void ld1(const VRegister& vt,
           const VRegister& vt2,
           const VRegister& vt3,
           const VRegister& vt4,
           const MemOperand& src);

  // One-element single structure load to one lane.
  void ld1(const VRegister& vt, int lane, const MemOperand& src);

  // One-element single structure load to all lanes.
  void ld1r(const VRegister& vt, const MemOperand& src);

  // Two-element structure load.
  void ld2(const VRegister& vt, const VRegister& vt2, const MemOperand& src);

  // Two-element single structure load to one lane.
  void ld2(const VRegister& vt,
           const VRegister& vt2,
           int lane,
           const MemOperand& src);

  // Two-element single structure load to all lanes.
  void ld2r(const VRegister& vt, const VRegister& vt2, const MemOperand& src);

  // Three-element structure load.
  void ld3(const VRegister& vt,
           const VRegister& vt2,
           const VRegister& vt3,
           const MemOperand& src);

  // Three-element single structure load to one lane.
  void ld3(const VRegister& vt,
           const VRegister& vt2,
           const VRegister& vt3,
           int lane,
           const MemOperand& src);

  // Three-element single structure load to all lanes.
  void ld3r(const VRegister& vt,
            const VRegister& vt2,
            const VRegister& vt3,
            const MemOperand& src);

  // Four-element structure load.
  void ld4(const VRegister& vt,
           const VRegister& vt2,
           const VRegister& vt3,
           const VRegister& vt4,
           const MemOperand& src);

  // Four-element single structure load to one lane.
  void ld4(const VRegister& vt,
           const VRegister& vt2,
           const VRegister& vt3,
           const VRegister& vt4,
           int lane,
           const MemOperand& src);

  // Four-element single structure load to all lanes.
  void ld4r(const VRegister& vt,
            const VRegister& vt2,
            const VRegister& vt3,
            const VRegister& vt4,
            const MemOperand& src);

  // Count leading sign bits.
  void cls(const VRegister& vd, const VRegister& vn);

  // Count leading zero bits (vector).
  void clz(const VRegister& vd, const VRegister& vn);

  // Population count per byte.
  void cnt(const VRegister& vd, const VRegister& vn);

  // Reverse bit order.
  void rbit(const VRegister& vd, const VRegister& vn);

  // Reverse elements in 16-bit halfwords.
  void rev16(const VRegister& vd, const VRegister& vn);

  // Reverse elements in 32-bit words.
  void rev32(const VRegister& vd, const VRegister& vn);

  // Reverse elements in 64-bit doublewords.
  void rev64(const VRegister& vd, const VRegister& vn);

  // Unsigned reciprocal square root estimate.
  void ursqrte(const VRegister& vd, const VRegister& vn);

  // Unsigned reciprocal estimate.
  void urecpe(const VRegister& vd, const VRegister& vn);

  // Signed pairwise long add.
  void saddlp(const VRegister& vd, const VRegister& vn);

  // Unsigned pairwise long add.
  void uaddlp(const VRegister& vd, const VRegister& vn);

  // Signed pairwise long add and accumulate.
  void sadalp(const VRegister& vd, const VRegister& vn);

  // Unsigned pairwise long add and accumulate.
  void uadalp(const VRegister& vd, const VRegister& vn);

  // Shift left by immediate.
  void shl(const VRegister& vd, const VRegister& vn, int shift);

  // Signed saturating shift left by immediate.
  void sqshl(const VRegister& vd, const VRegister& vn, int shift);

  // Signed saturating shift left unsigned by immediate.
  void sqshlu(const VRegister& vd, const VRegister& vn, int shift);

  // Unsigned saturating shift left by immediate.
  void uqshl(const VRegister& vd, const VRegister& vn, int shift);

  // Signed shift left long by immediate.
  void sshll(const VRegister& vd, const VRegister& vn, int shift);

  // Signed shift left long by immediate (second part).
  void sshll2(const VRegister& vd, const VRegister& vn, int shift);

  // Signed extend long.
  void sxtl(const VRegister& vd, const VRegister& vn);

  // Signed extend long (second part).
  void sxtl2(const VRegister& vd, const VRegister& vn);

  // Unsigned shift left long by immediate.
  void ushll(const VRegister& vd, const VRegister& vn, int shift);

  // Unsigned shift left long by immediate (second part).
  void ushll2(const VRegister& vd, const VRegister& vn, int shift);

  // Shift left long by element size.
  void shll(const VRegister& vd, const VRegister& vn, int shift);

  // Shift left long by element size (second part).
  void shll2(const VRegister& vd, const VRegister& vn, int shift);

  // Unsigned extend long.
  void uxtl(const VRegister& vd, const VRegister& vn);

  // Unsigned extend long (second part).
  void uxtl2(const VRegister& vd, const VRegister& vn);

  // Shift left by immediate and insert.
  void sli(const VRegister& vd, const VRegister& vn, int shift);

  // Shift right by immediate and insert.
  void sri(const VRegister& vd, const VRegister& vn, int shift);

  // Signed maximum.
  void smax(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed pairwise maximum.
  void smaxp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Add across vector.
  void addv(const VRegister& vd, const VRegister& vn);

  // Signed add long across vector.
  void saddlv(const VRegister& vd, const VRegister& vn);

  // Unsigned add long across vector.
  void uaddlv(const VRegister& vd, const VRegister& vn);

  // FP maximum number across vector.
  void fmaxnmv(const VRegister& vd, const VRegister& vn);

  // FP maximum across vector.
  void fmaxv(const VRegister& vd, const VRegister& vn);

  // FP minimum number across vector.
  void fminnmv(const VRegister& vd, const VRegister& vn);

  // FP minimum across vector.
  void fminv(const VRegister& vd, const VRegister& vn);

  // Signed maximum across vector.
  void smaxv(const VRegister& vd, const VRegister& vn);

  // Signed minimum.
  void smin(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed minimum pairwise.
  void sminp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed minimum across vector.
  void sminv(const VRegister& vd, const VRegister& vn);

  // One-element structure store from one register.
  void st1(const VRegister& vt, const MemOperand& src);

  // One-element structure store from two registers.
  void st1(const VRegister& vt, const VRegister& vt2, const MemOperand& src);

  // One-element structure store from three registers.
  void st1(const VRegister& vt,
           const VRegister& vt2,
           const VRegister& vt3,
           const MemOperand& src);

  // One-element structure store from four registers.
  void st1(const VRegister& vt,
           const VRegister& vt2,
           const VRegister& vt3,
           const VRegister& vt4,
           const MemOperand& src);

  // One-element single structure store from one lane.
  void st1(const VRegister& vt, int lane, const MemOperand& src);

  // Two-element structure store from two registers.
  void st2(const VRegister& vt, const VRegister& vt2, const MemOperand& src);

  // Two-element single structure store from two lanes.
  void st2(const VRegister& vt,
           const VRegister& vt2,
           int lane,
           const MemOperand& src);

  // Three-element structure store from three registers.
  void st3(const VRegister& vt,
           const VRegister& vt2,
           const VRegister& vt3,
           const MemOperand& src);

  // Three-element single structure store from three lanes.
  void st3(const VRegister& vt,
           const VRegister& vt2,
           const VRegister& vt3,
           int lane,
           const MemOperand& src);

  // Four-element structure store from four registers.
  void st4(const VRegister& vt,
           const VRegister& vt2,
           const VRegister& vt3,
           const VRegister& vt4,
           const MemOperand& src);

  // Four-element single structure store from four lanes.
  void st4(const VRegister& vt,
           const VRegister& vt2,
           const VRegister& vt3,
           const VRegister& vt4,
           int lane,
           const MemOperand& src);

  // Unsigned add long.
  void uaddl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned add long (second part).
  void uaddl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned add wide.
  void uaddw(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned add wide (second part).
  void uaddw2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed add long.
  void saddl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed add long (second part).
  void saddl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed add wide.
  void saddw(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed add wide (second part).
  void saddw2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned subtract long.
  void usubl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned subtract long (second part).
  void usubl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned subtract wide.
  void usubw(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned subtract wide (second part).
  void usubw2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed subtract long.
  void ssubl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed subtract long (second part).
  void ssubl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed integer subtract wide.
  void ssubw(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed integer subtract wide (second part).
  void ssubw2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned maximum.
  void umax(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned pairwise maximum.
  void umaxp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned maximum across vector.
  void umaxv(const VRegister& vd, const VRegister& vn);

  // Unsigned minimum.
  void umin(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned pairwise minimum.
  void uminp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned minimum across vector.
  void uminv(const VRegister& vd, const VRegister& vn);

  // Transpose vectors (primary).
  void trn1(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Transpose vectors (secondary).
  void trn2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unzip vectors (primary).
  void uzp1(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unzip vectors (secondary).
  void uzp2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Zip vectors (primary).
  void zip1(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Zip vectors (secondary).
  void zip2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed shift right by immediate.
  void sshr(const VRegister& vd, const VRegister& vn, int shift);

  // Unsigned shift right by immediate.
  void ushr(const VRegister& vd, const VRegister& vn, int shift);

  // Signed rounding shift right by immediate.
  void srshr(const VRegister& vd, const VRegister& vn, int shift);

  // Unsigned rounding shift right by immediate.
  void urshr(const VRegister& vd, const VRegister& vn, int shift);

  // Signed shift right by immediate and accumulate.
  void ssra(const VRegister& vd, const VRegister& vn, int shift);

  // Unsigned shift right by immediate and accumulate.
  void usra(const VRegister& vd, const VRegister& vn, int shift);

  // Signed rounding shift right by immediate and accumulate.
  void srsra(const VRegister& vd, const VRegister& vn, int shift);

  // Unsigned rounding shift right by immediate and accumulate.
  void ursra(const VRegister& vd, const VRegister& vn, int shift);

  // Shift right narrow by immediate.
  void shrn(const VRegister& vd, const VRegister& vn, int shift);

  // Shift right narrow by immediate (second part).
  void shrn2(const VRegister& vd, const VRegister& vn, int shift);

  // Rounding shift right narrow by immediate.
  void rshrn(const VRegister& vd, const VRegister& vn, int shift);

  // Rounding shift right narrow by immediate (second part).
  void rshrn2(const VRegister& vd, const VRegister& vn, int shift);

  // Unsigned saturating shift right narrow by immediate.
  void uqshrn(const VRegister& vd, const VRegister& vn, int shift);

  // Unsigned saturating shift right narrow by immediate (second part).
  void uqshrn2(const VRegister& vd, const VRegister& vn, int shift);

  // Unsigned saturating rounding shift right narrow by immediate.
  void uqrshrn(const VRegister& vd, const VRegister& vn, int shift);

  // Unsigned saturating rounding shift right narrow by immediate (second part).
  void uqrshrn2(const VRegister& vd, const VRegister& vn, int shift);

  // Signed saturating shift right narrow by immediate.
  void sqshrn(const VRegister& vd, const VRegister& vn, int shift);

  // Signed saturating shift right narrow by immediate (second part).
  void sqshrn2(const VRegister& vd, const VRegister& vn, int shift);

  // Signed saturating rounded shift right narrow by immediate.
  void sqrshrn(const VRegister& vd, const VRegister& vn, int shift);

  // Signed saturating rounded shift right narrow by immediate (second part).
  void sqrshrn2(const VRegister& vd, const VRegister& vn, int shift);

  // Signed saturating shift right unsigned narrow by immediate.
  void sqshrun(const VRegister& vd, const VRegister& vn, int shift);

  // Signed saturating shift right unsigned narrow by immediate (second part).
  void sqshrun2(const VRegister& vd, const VRegister& vn, int shift);

  // Signed sat rounded shift right unsigned narrow by immediate.
  void sqrshrun(const VRegister& vd, const VRegister& vn, int shift);

  // Signed sat rounded shift right unsigned narrow by immediate (second part).
  void sqrshrun2(const VRegister& vd, const VRegister& vn, int shift);

  // FP reciprocal step.
  void frecps(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP reciprocal estimate.
  void frecpe(const VRegister& vd, const VRegister& vn);

  // FP reciprocal square root estimate.
  void frsqrte(const VRegister& vd, const VRegister& vn);

  // FP reciprocal square root step.
  void frsqrts(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed absolute difference and accumulate long.
  void sabal(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed absolute difference and accumulate long (second part).
  void sabal2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned absolute difference and accumulate long.
  void uabal(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned absolute difference and accumulate long (second part).
  void uabal2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed absolute difference long.
  void sabdl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed absolute difference long (second part).
  void sabdl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned absolute difference long.
  void uabdl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned absolute difference long (second part).
  void uabdl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Polynomial multiply long.
  void pmull(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Polynomial multiply long (second part).
  void pmull2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed long multiply-add.
  void smlal(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed long multiply-add (second part).
  void smlal2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned long multiply-add.
  void umlal(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned long multiply-add (second part).
  void umlal2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed long multiply-sub.
  void smlsl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed long multiply-sub (second part).
  void smlsl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned long multiply-sub.
  void umlsl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned long multiply-sub (second part).
  void umlsl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed long multiply.
  void smull(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed long multiply (second part).
  void smull2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed saturating doubling long multiply-add.
  void sqdmlal(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed saturating doubling long multiply-add (second part).
  void sqdmlal2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed saturating doubling long multiply-subtract.
  void sqdmlsl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed saturating doubling long multiply-subtract (second part).
  void sqdmlsl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed saturating doubling long multiply.
  void sqdmull(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed saturating doubling long multiply (second part).
  void sqdmull2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed saturating doubling multiply returning high half.
  void sqdmulh(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed saturating rounding doubling multiply returning high half.
  void sqrdmulh(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed dot product [Armv8.2].
  void sdot(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed saturating rounding doubling multiply accumulate returning high
  // half [Armv8.1].
  void sqrdmlah(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned dot product [Armv8.2].
  void udot(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Dot Product with unsigned and signed integers (vector).
  void usdot(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Dot product with signed and unsigned integers (vector, by element).
  void sudot(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             int vm_index);

  // Dot product with unsigned and signed integers (vector, by element).
  void usdot(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             int vm_index);

  // Signed saturating rounding doubling multiply subtract returning high half
  // [Armv8.1].
  void sqrdmlsh(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Signed saturating doubling multiply element returning high half.
  void sqdmulh(const VRegister& vd,
               const VRegister& vn,
               const VRegister& vm,
               int vm_index);

  // Signed saturating rounding doubling multiply element returning high half.
  void sqrdmulh(const VRegister& vd,
                const VRegister& vn,
                const VRegister& vm,
                int vm_index);

  // Signed dot product by element [Armv8.2].
  void sdot(const VRegister& vd,
            const VRegister& vn,
            const VRegister& vm,
            int vm_index);

  // Signed saturating rounding doubling multiply accumulate element returning
  // high half [Armv8.1].
  void sqrdmlah(const VRegister& vd,
                const VRegister& vn,
                const VRegister& vm,
                int vm_index);

  // Unsigned dot product by element [Armv8.2].
  void udot(const VRegister& vd,
            const VRegister& vn,
            const VRegister& vm,
            int vm_index);

  // Signed saturating rounding doubling multiply subtract element returning
  // high half [Armv8.1].
  void sqrdmlsh(const VRegister& vd,
                const VRegister& vn,
                const VRegister& vm,
                int vm_index);

  // Unsigned long multiply long.
  void umull(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned long multiply (second part).
  void umull2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Add narrow returning high half.
  void addhn(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Add narrow returning high half (second part).
  void addhn2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Rounding add narrow returning high half.
  void raddhn(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Rounding add narrow returning high half (second part).
  void raddhn2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Subtract narrow returning high half.
  void subhn(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Subtract narrow returning high half (second part).
  void subhn2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Rounding subtract narrow returning high half.
  void rsubhn(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Rounding subtract narrow returning high half (second part).
  void rsubhn2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP vector multiply accumulate.
  void fmla(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP fused multiply-add long to accumulator.
  void fmlal(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP fused multiply-add long to accumulator (second part).
  void fmlal2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP fused multiply-add long to accumulator by element.
  void fmlal(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             int vm_index);

  // FP fused multiply-add long to accumulator by element (second part).
  void fmlal2(const VRegister& vd,
              const VRegister& vn,
              const VRegister& vm,
              int vm_index);

  // FP vector multiply subtract.
  void fmls(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP fused multiply-subtract long to accumulator.
  void fmlsl(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP fused multiply-subtract long to accumulator (second part).
  void fmlsl2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP fused multiply-subtract long to accumulator by element.
  void fmlsl(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             int vm_index);

  // FP fused multiply-subtract long to accumulator by element (second part).
  void fmlsl2(const VRegister& vd,
              const VRegister& vn,
              const VRegister& vm,
              int vm_index);

  // FP vector multiply extended.
  void fmulx(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP absolute greater than or equal.
  void facge(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP absolute greater than.
  void facgt(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP multiply by element.
  void fmul(const VRegister& vd,
            const VRegister& vn,
            const VRegister& vm,
            int vm_index);

  // FP fused multiply-add to accumulator by element.
  void fmla(const VRegister& vd,
            const VRegister& vn,
            const VRegister& vm,
            int vm_index);

  // FP fused multiply-sub from accumulator by element.
  void fmls(const VRegister& vd,
            const VRegister& vn,
            const VRegister& vm,
            int vm_index);

  // FP multiply extended by element.
  void fmulx(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             int vm_index);

  // FP compare equal.
  void fcmeq(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP greater than.
  void fcmgt(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP greater than or equal.
  void fcmge(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP compare equal to zero.
  void fcmeq(const VRegister& vd, const VRegister& vn, double imm);

  // FP greater than zero.
  void fcmgt(const VRegister& vd, const VRegister& vn, double imm);

  // FP greater than or equal to zero.
  void fcmge(const VRegister& vd, const VRegister& vn, double imm);

  // FP less than or equal to zero.
  void fcmle(const VRegister& vd, const VRegister& vn, double imm);

  // FP less than to zero.
  void fcmlt(const VRegister& vd, const VRegister& vn, double imm);

  // FP absolute difference.
  void fabd(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP pairwise add vector.
  void faddp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP pairwise add scalar.
  void faddp(const VRegister& vd, const VRegister& vn);

  // FP pairwise maximum vector.
  void fmaxp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP pairwise maximum scalar.
  void fmaxp(const VRegister& vd, const VRegister& vn);

  // FP pairwise minimum vector.
  void fminp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP pairwise minimum scalar.
  void fminp(const VRegister& vd, const VRegister& vn);

  // FP pairwise maximum number vector.
  void fmaxnmp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP pairwise maximum number scalar.
  void fmaxnmp(const VRegister& vd, const VRegister& vn);

  // FP pairwise minimum number vector.
  void fminnmp(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // FP pairwise minimum number scalar.
  void fminnmp(const VRegister& vd, const VRegister& vn);

  // v8.3 complex numbers - note that these are only partial/helper functions
  // and must be used in series in order to perform full CN operations.

  // FP complex multiply accumulate (by element) [Armv8.3].
  void fcmla(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             int vm_index,
             int rot);

  // FP complex multiply accumulate [Armv8.3].
  void fcmla(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             int rot);

  // FP complex add [Armv8.3].
  void fcadd(const VRegister& vd,
             const VRegister& vn,
             const VRegister& vm,
             int rot);

  // Signed 8-bit integer matrix multiply-accumulate (vector).
  void smmla(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned and signed 8-bit integer matrix multiply-accumulate (vector).
  void usmmla(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Unsigned 8-bit integer matrix multiply-accumulate (vector).
  void ummla(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Bit Clear and exclusive-OR.
  void bcax(const VRegister& vd,
            const VRegister& vn,
            const VRegister& vm,
            const VRegister& va);

  // Three-way Exclusive-OR.
  void eor3(const VRegister& vd,
            const VRegister& vn,
            const VRegister& vm,
            const VRegister& va);

  // Exclusive-OR and Rotate.
  void xar(const VRegister& vd,
           const VRegister& vn,
           const VRegister& vm,
           int rotate);

  // Rotate and Exclusive-OR
  void rax1(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // SHA1 hash update (choose).
  void sha1c(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // SHA1 fixed rotate.
  void sha1h(const VRegister& sd, const VRegister& sn);

  // SHA1 hash update (majority).
  void sha1m(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // SHA1 hash update (parity).
  void sha1p(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // SHA1 schedule update 0.
  void sha1su0(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // SHA1 schedule update 1.
  void sha1su1(const VRegister& vd, const VRegister& vn);

  // SHA256 hash update (part 1).
  void sha256h(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // SHA256 hash update (part 2).
  void sha256h2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // SHA256 schedule update 0.
  void sha256su0(const VRegister& vd, const VRegister& vn);

  // SHA256 schedule update 1.
  void sha256su1(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // SHA512 hash update part 1.
  void sha512h(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // SHA512 hash update part 2.
  void sha512h2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // SHA512 schedule Update 0.
  void sha512su0(const VRegister& vd, const VRegister& vn);

  // SHA512 schedule Update 1.
  void sha512su1(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // AES single round decryption.
  void aesd(const VRegister& vd, const VRegister& vn);

  // AES single round encryption.
  void aese(const VRegister& vd, const VRegister& vn);

  // AES inverse mix columns.
  void aesimc(const VRegister& vd, const VRegister& vn);

  // AES mix columns.
  void aesmc(const VRegister& vd, const VRegister& vn);

  // SM3PARTW1.
  void sm3partw1(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // SM3PARTW2.
  void sm3partw2(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // SM3SS1.
  void sm3ss1(const VRegister& vd,
              const VRegister& vn,
              const VRegister& vm,
              const VRegister& va);

  // SM3TT1A.
  void sm3tt1a(const VRegister& vd,
               const VRegister& vn,
               const VRegister& vm,
               int index);

  // SM3TT1B.
  void sm3tt1b(const VRegister& vd,
               const VRegister& vn,
               const VRegister& vm,
               int index);

  // SM3TT2A.
  void sm3tt2a(const VRegister& vd,
               const VRegister& vn,
               const VRegister& vm,
               int index);

  // SM3TT2B.
  void sm3tt2b(const VRegister& vd,
               const VRegister& vn,
               const VRegister& vm,
               int index);

  // SM4 Encode.
  void sm4e(const VRegister& vd, const VRegister& vn);

  // SM4 Key.
  void sm4ekey(const VRegister& vd, const VRegister& vn, const VRegister& vm);

  // Scalable Vector Extensions.

  // Absolute value (predicated).
  void abs(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Add vectors (predicated).
  void add(const ZRegister& zd,
           const PRegisterM& pg,
           const ZRegister& zn,
           const ZRegister& zm);

  // Add vectors (unpredicated).
  void add(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Add immediate (unpredicated).
  void add(const ZRegister& zd, const ZRegister& zn, int imm8, int shift = -1);

  // Add multiple of predicate register size to scalar register.
  void addpl(const Register& xd, const Register& xn, int imm6);

  // Add multiple of vector register size to scalar register.
  void addvl(const Register& xd, const Register& xn, int imm6);

  // Compute vector address.
  void adr(const ZRegister& zd, const SVEMemOperand& addr);

  // Bitwise AND predicates.
  void and_(const PRegisterWithLaneSize& pd,
            const PRegisterZ& pg,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Bitwise AND vectors (predicated).
  void and_(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Bitwise AND with immediate (unpredicated).
  void and_(const ZRegister& zd, const ZRegister& zn, uint64_t imm);

  // Bitwise AND vectors (unpredicated).
  void and_(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Bitwise AND predicates.
  void ands(const PRegisterWithLaneSize& pd,
            const PRegisterZ& pg,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Bitwise AND reduction to scalar.
  void andv(const VRegister& vd, const PRegister& pg, const ZRegister& zn);

  // Arithmetic shift right by immediate (predicated).
  void asr(const ZRegister& zd,
           const PRegisterM& pg,
           const ZRegister& zn,
           int shift);

  // Arithmetic shift right by 64-bit wide elements (predicated).
  void asr(const ZRegister& zd,
           const PRegisterM& pg,
           const ZRegister& zn,
           const ZRegister& zm);

  // Arithmetic shift right by immediate (unpredicated).
  void asr(const ZRegister& zd, const ZRegister& zn, int shift);

  // Arithmetic shift right by 64-bit wide elements (unpredicated).
  void asr(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Arithmetic shift right for divide by immediate (predicated).
  void asrd(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            int shift);

  // Reversed arithmetic shift right by vector (predicated).
  void asrr(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Bitwise clear predicates.
  void bic(const PRegisterWithLaneSize& pd,
           const PRegisterZ& pg,
           const PRegisterWithLaneSize& pn,
           const PRegisterWithLaneSize& pm);

  // Bitwise clear vectors (predicated).
  void bic(const ZRegister& zd,
           const PRegisterM& pg,
           const ZRegister& zn,
           const ZRegister& zm);

  // Bitwise clear bits using immediate (unpredicated).
  void bic(const ZRegister& zd, const ZRegister& zn, uint64_t imm);

  // Bitwise clear vectors (unpredicated).
  void bic(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Bitwise clear predicates.
  void bics(const PRegisterWithLaneSize& pd,
            const PRegisterZ& pg,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Break after first true condition.
  void brka(const PRegisterWithLaneSize& pd,
            const PRegister& pg,
            const PRegisterWithLaneSize& pn);

  // Break after first true condition.
  void brkas(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const PRegisterWithLaneSize& pn);

  // Break before first true condition.
  void brkb(const PRegisterWithLaneSize& pd,
            const PRegister& pg,
            const PRegisterWithLaneSize& pn);

  // Break before first true condition.
  void brkbs(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const PRegisterWithLaneSize& pn);

  // Propagate break to next partition.
  void brkn(const PRegisterWithLaneSize& pd,
            const PRegisterZ& pg,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Propagate break to next partition.
  void brkns(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const PRegisterWithLaneSize& pn,
             const PRegisterWithLaneSize& pm);

  // Break after first true condition, propagating from previous partition.
  void brkpa(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const PRegisterWithLaneSize& pn,
             const PRegisterWithLaneSize& pm);

  // Break after first true condition, propagating from previous partition.
  void brkpas(const PRegisterWithLaneSize& pd,
              const PRegisterZ& pg,
              const PRegisterWithLaneSize& pn,
              const PRegisterWithLaneSize& pm);

  // Break before first true condition, propagating from previous partition.
  void brkpb(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const PRegisterWithLaneSize& pn,
             const PRegisterWithLaneSize& pm);

  // Break before first true condition, propagating from previous partition.
  void brkpbs(const PRegisterWithLaneSize& pd,
              const PRegisterZ& pg,
              const PRegisterWithLaneSize& pn,
              const PRegisterWithLaneSize& pm);

  // Conditionally extract element after last to general-purpose register.
  void clasta(const Register& rd,
              const PRegister& pg,
              const Register& rn,
              const ZRegister& zm);

  // Conditionally extract element after last to SIMD&FP scalar register.
  void clasta(const VRegister& vd,
              const PRegister& pg,
              const VRegister& vn,
              const ZRegister& zm);

  // Conditionally extract element after last to vector register.
  void clasta(const ZRegister& zd,
              const PRegister& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Conditionally extract last element to general-purpose register.
  void clastb(const Register& rd,
              const PRegister& pg,
              const Register& rn,
              const ZRegister& zm);

  // Conditionally extract last element to SIMD&FP scalar register.
  void clastb(const VRegister& vd,
              const PRegister& pg,
              const VRegister& vn,
              const ZRegister& zm);

  // Conditionally extract last element to vector register.
  void clastb(const ZRegister& zd,
              const PRegister& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Count leading sign bits (predicated).
  void cls(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Count leading zero bits (predicated).
  void clz(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  void cmp(Condition cond,
           const PRegisterWithLaneSize& pd,
           const PRegisterZ& pg,
           const ZRegister& zn,
           const ZRegister& zm);

  // Compare vector to 64-bit wide elements.
  void cmpeq(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Compare vector to immediate.
  void cmpeq(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             int imm5);

  // Compare vector to 64-bit wide elements.
  void cmpge(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Compare vector to immediate.
  void cmpge(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             int imm5);

  // Compare vector to 64-bit wide elements.
  void cmpgt(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Compare vector to immediate.
  void cmpgt(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             int imm5);

  // Compare vector to 64-bit wide elements.
  void cmphi(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Compare vector to immediate.
  void cmphi(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             unsigned imm7);

  // Compare vector to 64-bit wide elements.
  void cmphs(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Compare vector to immediate.
  void cmphs(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             unsigned imm7);

  // Compare vector to 64-bit wide elements.
  void cmple(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Compare vector to immediate.
  void cmple(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             int imm5);

  // Compare vector to 64-bit wide elements.
  void cmplo(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Compare vector to immediate.
  void cmplo(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             unsigned imm7);

  // Compare vector to 64-bit wide elements.
  void cmpls(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Compare vector to immediate.
  void cmpls(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             unsigned imm7);

  // Compare vector to 64-bit wide elements.
  void cmplt(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Compare vector to immediate.
  void cmplt(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             int imm5);

  // Compare vector to 64-bit wide elements.
  void cmpne(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Compare vector to immediate.
  void cmpne(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             int imm5);

  // Logically invert boolean condition in vector (predicated).
  void cnot(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Count non-zero bits (predicated).
  void cnt(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Set scalar to multiple of predicate constraint element count.
  void cntb(const Register& rd, int pattern = SVE_ALL, int multiplier = 1);

  // Set scalar to multiple of predicate constraint element count.
  void cntd(const Register& rd, int pattern = SVE_ALL, int multiplier = 1);

  // Set scalar to multiple of predicate constraint element count.
  void cnth(const Register& rd, int pattern = SVE_ALL, int multiplier = 1);

  // Set scalar to active predicate element count.
  void cntp(const Register& xd,
            const PRegister& pg,
            const PRegisterWithLaneSize& pn);

  // Set scalar to multiple of predicate constraint element count.
  void cntw(const Register& rd, int pattern = SVE_ALL, int multiplier = 1);

  // Shuffle active elements of vector to the right and fill with zero.
  void compact(const ZRegister& zd, const PRegister& pg, const ZRegister& zn);

  // Copy signed integer immediate to vector elements (predicated).
  void cpy(const ZRegister& zd, const PRegister& pg, int imm8, int shift = -1);

  // Copy general-purpose register to vector elements (predicated).
  void cpy(const ZRegister& zd, const PRegisterM& pg, const Register& rn);

  // Copy SIMD&FP scalar register to vector elements (predicated).
  void cpy(const ZRegister& zd, const PRegisterM& pg, const VRegister& vn);

  // Compare and terminate loop.
  void ctermeq(const Register& rn, const Register& rm);

  // Compare and terminate loop.
  void ctermne(const Register& rn, const Register& rm);

  // Decrement scalar by multiple of predicate constraint element count.
  void decb(const Register& xdn, int pattern = SVE_ALL, int multiplier = 1);

  // Decrement scalar by multiple of predicate constraint element count.
  void decd(const Register& xdn, int pattern = SVE_ALL, int multiplier = 1);

  // Decrement vector by multiple of predicate constraint element count.
  void decd(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Decrement scalar by multiple of predicate constraint element count.
  void dech(const Register& xdn, int pattern = SVE_ALL, int multiplier = 1);

  // Decrement vector by multiple of predicate constraint element count.
  void dech(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Decrement scalar by active predicate element count.
  void decp(const Register& rdn, const PRegisterWithLaneSize& pg);

  // Decrement vector by active predicate element count.
  void decp(const ZRegister& zdn, const PRegister& pg);

  // Decrement scalar by multiple of predicate constraint element count.
  void decw(const Register& xdn, int pattern = SVE_ALL, int multiplier = 1);

  // Decrement vector by multiple of predicate constraint element count.
  void decw(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Broadcast general-purpose register to vector elements (unpredicated).
  void dup(const ZRegister& zd, const Register& xn);

  // Broadcast indexed element to vector (unpredicated).
  void dup(const ZRegister& zd, const ZRegister& zn, unsigned index);

  // As for movz/movk/movn, if the default shift of -1 is specified to dup, the
  // assembler will pick an appropriate immediate and left shift that is
  // equivalent to the immediate argument. If an explicit left shift is
  // specified (0 or 8), the immediate must be a signed 8-bit integer.

  // Broadcast signed immediate to vector elements (unpredicated).
  void dup(const ZRegister& zd, int imm8, int shift = -1);

  // Broadcast logical bitmask immediate to vector (unpredicated).
  void dupm(const ZRegister& zd, uint64_t imm);

  // Bitwise exclusive OR with inverted immediate (unpredicated).
  void eon(const ZRegister& zd, const ZRegister& zn, uint64_t imm);

  // Bitwise exclusive OR predicates.
  void eor(const PRegisterWithLaneSize& pd,
           const PRegisterZ& pg,
           const PRegisterWithLaneSize& pn,
           const PRegisterWithLaneSize& pm);

  // Bitwise exclusive OR vectors (predicated).
  void eor(const ZRegister& zd,
           const PRegisterM& pg,
           const ZRegister& zn,
           const ZRegister& zm);

  // Bitwise exclusive OR with immediate (unpredicated).
  void eor(const ZRegister& zd, const ZRegister& zn, uint64_t imm);

  // Bitwise exclusive OR vectors (unpredicated).
  void eor(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Bitwise exclusive OR predicates.
  void eors(const PRegisterWithLaneSize& pd,
            const PRegisterZ& pg,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Bitwise XOR reduction to scalar.
  void eorv(const VRegister& vd, const PRegister& pg, const ZRegister& zn);

  // Extract vector from pair of vectors.
  void ext(const ZRegister& zd,
           const ZRegister& zn,
           const ZRegister& zm,
           unsigned offset);

  // Floating-point absolute difference (predicated).
  void fabd(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Floating-point absolute value (predicated).
  void fabs(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point absolute compare vectors.
  void facge(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Floating-point absolute compare vectors.
  void facgt(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Floating-point add immediate (predicated).
  void fadd(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            double imm);

  // Floating-point add vector (predicated).
  void fadd(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Floating-point add vector (unpredicated).
  void fadd(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Floating-point add strictly-ordered reduction, accumulating in scalar.
  void fadda(const VRegister& vd,
             const PRegister& pg,
             const VRegister& vn,
             const ZRegister& zm);

  // Floating-point add recursive reduction to scalar.
  void faddv(const VRegister& vd, const PRegister& pg, const ZRegister& zn);

  // Floating-point complex add with rotate (predicated).
  void fcadd(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm,
             int rot);

  // Floating-point compare vector with zero.
  void fcmeq(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             double zero);

  // Floating-point compare vectors.
  void fcmeq(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Floating-point compare vector with zero.
  void fcmge(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             double zero);

  // Floating-point compare vectors.
  void fcmge(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Floating-point compare vector with zero.
  void fcmgt(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             double zero);

  // Floating-point compare vectors.
  void fcmgt(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Floating-point complex multiply-add with rotate (predicated).
  void fcmla(const ZRegister& zda,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm,
             int rot);

  // Floating-point complex multiply-add by indexed values with rotate.
  void fcmla(const ZRegister& zda,
             const ZRegister& zn,
             const ZRegister& zm,
             int index,
             int rot);

  // Floating-point compare vector with zero.
  void fcmle(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             double zero);

  // Floating-point compare vector with zero.
  void fcmlt(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             double zero);

  // Floating-point compare vector with zero.
  void fcmne(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             double zero);

  // Floating-point compare vectors.
  void fcmne(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Floating-point compare vectors.
  void fcmuo(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Copy floating-point immediate to vector elements (predicated).
  void fcpy(const ZRegister& zd, const PRegisterM& pg, double imm);

  // Copy half-precision floating-point immediate to vector elements
  // (predicated).
  void fcpy(const ZRegister& zd, const PRegisterM& pg, Float16 imm) {
    fcpy(zd, pg, FPToDouble(imm, kIgnoreDefaultNaN));
  }

  // Floating-point convert precision (predicated).
  void fcvt(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point convert to signed integer, rounding toward zero
  // (predicated).
  void fcvtzs(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point convert to unsigned integer, rounding toward zero
  // (predicated).
  void fcvtzu(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point divide by vector (predicated).
  void fdiv(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Floating-point reversed divide by vector (predicated).
  void fdivr(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Broadcast floating-point immediate to vector elements.
  void fdup(const ZRegister& zd, double imm);

  // Broadcast half-precision floating-point immediate to vector elements.
  void fdup(const ZRegister& zd, Float16 imm) {
    fdup(zd, FPToDouble(imm, kIgnoreDefaultNaN));
  }

  // Floating-point exponential accelerator.
  void fexpa(const ZRegister& zd, const ZRegister& zn);

  // Floating-point fused multiply-add vectors (predicated), writing
  // multiplicand [Zdn = Za + Zdn * Zm].
  void fmad(const ZRegister& zdn,
            const PRegisterM& pg,
            const ZRegister& zm,
            const ZRegister& za);

  // Floating-point maximum with immediate (predicated).
  void fmax(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            double imm);

  // Floating-point maximum (predicated).
  void fmax(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Floating-point maximum number with immediate (predicated).
  void fmaxnm(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              double imm);

  // Floating-point maximum number (predicated).
  void fmaxnm(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Floating-point maximum number recursive reduction to scalar.
  void fmaxnmv(const VRegister& vd, const PRegister& pg, const ZRegister& zn);

  // Floating-point maximum recursive reduction to scalar.
  void fmaxv(const VRegister& vd, const PRegister& pg, const ZRegister& zn);

  // Floating-point minimum with immediate (predicated).
  void fmin(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            double imm);

  // Floating-point minimum (predicated).
  void fmin(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Floating-point minimum number with immediate (predicated).
  void fminnm(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              double imm);

  // Floating-point minimum number (predicated).
  void fminnm(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Floating-point minimum number recursive reduction to scalar.
  void fminnmv(const VRegister& vd, const PRegister& pg, const ZRegister& zn);

  // Floating-point minimum recursive reduction to scalar.
  void fminv(const VRegister& vd, const PRegister& pg, const ZRegister& zn);

  // Floating-point fused multiply-add vectors (predicated), writing addend
  // [Zda = Zda + Zn * Zm].
  void fmla(const ZRegister& zda,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Floating-point fused multiply-add by indexed elements
  // (Zda = Zda + Zn * Zm[indexed]).
  void fmla(const ZRegister& zda,
            const ZRegister& zn,
            const ZRegister& zm,
            int index);

  // Floating-point fused multiply-subtract vectors (predicated), writing
  // addend [Zda = Zda + -Zn * Zm].
  void fmls(const ZRegister& zda,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Floating-point fused multiply-subtract by indexed elements
  // (Zda = Zda + -Zn * Zm[indexed]).
  void fmls(const ZRegister& zda,
            const ZRegister& zn,
            const ZRegister& zm,
            int index);

  // Move 8-bit floating-point immediate to vector elements (unpredicated).
  void fmov(const ZRegister& zd, double imm);

  // Move 8-bit floating-point immediate to vector elements (predicated).
  void fmov(const ZRegister& zd, const PRegisterM& pg, double imm);

  // Floating-point fused multiply-subtract vectors (predicated), writing
  // multiplicand [Zdn = Za + -Zdn * Zm].
  void fmsb(const ZRegister& zdn,
            const PRegisterM& pg,
            const ZRegister& zm,
            const ZRegister& za);

  // Floating-point multiply by immediate (predicated).
  void fmul(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            double imm);

  // Floating-point multiply vectors (predicated).
  void fmul(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Floating-point multiply by indexed elements.
  void fmul(const ZRegister& zd,
            const ZRegister& zn,
            const ZRegister& zm,
            unsigned index);

  // Floating-point multiply vectors (unpredicated).
  void fmul(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Floating-point multiply-extended vectors (predicated).
  void fmulx(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Floating-point negate (predicated).
  void fneg(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point negated fused multiply-add vectors (predicated), writing
  // multiplicand [Zdn = -Za + -Zdn * Zm].
  void fnmad(const ZRegister& zdn,
             const PRegisterM& pg,
             const ZRegister& zm,
             const ZRegister& za);

  // Floating-point negated fused multiply-add vectors (predicated), writing
  // addend [Zda = -Zda + -Zn * Zm].
  void fnmla(const ZRegister& zda,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Floating-point negated fused multiply-subtract vectors (predicated),
  // writing addend [Zda = -Zda + Zn * Zm].
  void fnmls(const ZRegister& zda,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Floating-point negated fused multiply-subtract vectors (predicated),
  // writing multiplicand [Zdn = -Za + Zdn * Zm].
  void fnmsb(const ZRegister& zdn,
             const PRegisterM& pg,
             const ZRegister& zm,
             const ZRegister& za);

  // Floating-point reciprocal estimate (unpredicated).
  void frecpe(const ZRegister& zd, const ZRegister& zn);

  // Floating-point reciprocal step (unpredicated).
  void frecps(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Floating-point reciprocal exponent (predicated).
  void frecpx(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point round to integral value (predicated).
  void frinta(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point round to integral value (predicated).
  void frinti(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point round to integral value (predicated).
  void frintm(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point round to integral value (predicated).
  void frintn(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point round to integral value (predicated).
  void frintp(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point round to integral value (predicated).
  void frintx(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point round to integral value (predicated).
  void frintz(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point reciprocal square root estimate (unpredicated).
  void frsqrte(const ZRegister& zd, const ZRegister& zn);

  // Floating-point reciprocal square root step (unpredicated).
  void frsqrts(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Floating-point adjust exponent by vector (predicated).
  void fscale(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Floating-point square root (predicated).
  void fsqrt(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point subtract immediate (predicated).
  void fsub(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            double imm);

  // Floating-point subtract vectors (predicated).
  void fsub(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Floating-point subtract vectors (unpredicated).
  void fsub(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Floating-point reversed subtract from immediate (predicated).
  void fsubr(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             double imm);

  // Floating-point reversed subtract vectors (predicated).
  void fsubr(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Floating-point trigonometric multiply-add coefficient.
  void ftmad(const ZRegister& zd,
             const ZRegister& zn,
             const ZRegister& zm,
             int imm3);

  // Floating-point trigonometric starting value.
  void ftsmul(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Floating-point trigonometric select coefficient.
  void ftssel(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Increment scalar by multiple of predicate constraint element count.
  void incb(const Register& xdn, int pattern = SVE_ALL, int multiplier = 1);

  // Increment scalar by multiple of predicate constraint element count.
  void incd(const Register& xdn, int pattern = SVE_ALL, int multiplier = 1);

  // Increment vector by multiple of predicate constraint element count.
  void incd(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Increment scalar by multiple of predicate constraint element count.
  void inch(const Register& xdn, int pattern = SVE_ALL, int multiplier = 1);

  // Increment vector by multiple of predicate constraint element count.
  void inch(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Increment scalar by active predicate element count.
  void incp(const Register& rdn, const PRegisterWithLaneSize& pg);

  // Increment vector by active predicate element count.
  void incp(const ZRegister& zdn, const PRegister& pg);

  // Increment scalar by multiple of predicate constraint element count.
  void incw(const Register& xdn, int pattern = SVE_ALL, int multiplier = 1);

  // Increment vector by multiple of predicate constraint element count.
  void incw(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Create index starting from and incremented by immediate.
  void index(const ZRegister& zd, int start, int step);

  // Create index starting from and incremented by general-purpose register.
  void index(const ZRegister& zd, const Register& rn, const Register& rm);

  // Create index starting from general-purpose register and incremented by
  // immediate.
  void index(const ZRegister& zd, const Register& rn, int imm5);

  // Create index starting from immediate and incremented by general-purpose
  // register.
  void index(const ZRegister& zd, int imm5, const Register& rm);

  // Insert general-purpose register in shifted vector.
  void insr(const ZRegister& zdn, const Register& rm);

  // Insert SIMD&FP scalar register in shifted vector.
  void insr(const ZRegister& zdn, const VRegister& vm);

  // Extract element after last to general-purpose register.
  void lasta(const Register& rd, const PRegister& pg, const ZRegister& zn);

  // Extract element after last to SIMD&FP scalar register.
  void lasta(const VRegister& vd, const PRegister& pg, const ZRegister& zn);

  // Extract last element to general-purpose register.
  void lastb(const Register& rd, const PRegister& pg, const ZRegister& zn);

  // Extract last element to SIMD&FP scalar register.
  void lastb(const VRegister& vd, const PRegister& pg, const ZRegister& zn);

  // Contiguous/gather load bytes to vector.
  void ld1b(const ZRegister& zt,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous/gather load halfwords to vector.
  void ld1h(const ZRegister& zt,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous/gather load words to vector.
  void ld1w(const ZRegister& zt,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous/gather load doublewords to vector.
  void ld1d(const ZRegister& zt,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // TODO: Merge other loads into the SVEMemOperand versions.

  // Load and broadcast unsigned byte to vector.
  void ld1rb(const ZRegister& zt,
             const PRegisterZ& pg,
             const SVEMemOperand& addr);

  // Load and broadcast unsigned halfword to vector.
  void ld1rh(const ZRegister& zt,
             const PRegisterZ& pg,
             const SVEMemOperand& addr);

  // Load and broadcast unsigned word to vector.
  void ld1rw(const ZRegister& zt,
             const PRegisterZ& pg,
             const SVEMemOperand& addr);

  // Load and broadcast doubleword to vector.
  void ld1rd(const ZRegister& zt,
             const PRegisterZ& pg,
             const SVEMemOperand& addr);

  // Contiguous load and replicate sixteen bytes.
  void ld1rqb(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load and replicate eight halfwords.
  void ld1rqh(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load and replicate four words.
  void ld1rqw(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load and replicate two doublewords.
  void ld1rqd(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load and replicate thirty-two bytes.
  void ld1rob(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load and replicate sixteen halfwords.
  void ld1roh(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load and replicate eight words.
  void ld1row(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load and replicate four doublewords.
  void ld1rod(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Load and broadcast signed byte to vector.
  void ld1rsb(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Load and broadcast signed halfword to vector.
  void ld1rsh(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Load and broadcast signed word to vector.
  void ld1rsw(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous/gather load signed bytes to vector.
  void ld1sb(const ZRegister& zt,
             const PRegisterZ& pg,
             const SVEMemOperand& addr);

  // Contiguous/gather load signed halfwords to vector.
  void ld1sh(const ZRegister& zt,
             const PRegisterZ& pg,
             const SVEMemOperand& addr);

  // Contiguous/gather load signed words to vector.
  void ld1sw(const ZRegister& zt,
             const PRegisterZ& pg,
             const SVEMemOperand& addr);

  // TODO: Merge other loads into the SVEMemOperand versions.

  // Contiguous load two-byte structures to two vectors.
  void ld2b(const ZRegister& zt1,
            const ZRegister& zt2,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous load two-halfword structures to two vectors.
  void ld2h(const ZRegister& zt1,
            const ZRegister& zt2,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous load two-word structures to two vectors.
  void ld2w(const ZRegister& zt1,
            const ZRegister& zt2,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous load two-doubleword structures to two vectors.
  void ld2d(const ZRegister& zt1,
            const ZRegister& zt2,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous load three-byte structures to three vectors.
  void ld3b(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous load three-halfword structures to three vectors.
  void ld3h(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous load three-word structures to three vectors.
  void ld3w(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous load three-doubleword structures to three vectors.
  void ld3d(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous load four-byte structures to four vectors.
  void ld4b(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const ZRegister& zt4,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous load four-halfword structures to four vectors.
  void ld4h(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const ZRegister& zt4,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous load four-word structures to four vectors.
  void ld4w(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const ZRegister& zt4,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous load four-doubleword structures to four vectors.
  void ld4d(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const ZRegister& zt4,
            const PRegisterZ& pg,
            const SVEMemOperand& addr);

  // Contiguous load first-fault unsigned bytes to vector.
  void ldff1b(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load first-fault unsigned halfwords to vector.
  void ldff1h(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load first-fault unsigned words to vector.
  void ldff1w(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load first-fault doublewords to vector.
  void ldff1d(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load first-fault signed bytes to vector.
  void ldff1sb(const ZRegister& zt,
               const PRegisterZ& pg,
               const SVEMemOperand& addr);

  // Contiguous load first-fault signed halfwords to vector.
  void ldff1sh(const ZRegister& zt,
               const PRegisterZ& pg,
               const SVEMemOperand& addr);

  // Contiguous load first-fault signed words to vector.
  void ldff1sw(const ZRegister& zt,
               const PRegisterZ& pg,
               const SVEMemOperand& addr);

  // Gather load first-fault unsigned bytes to vector.
  void ldff1b(const ZRegister& zt,
              const PRegisterZ& pg,
              const Register& xn,
              const ZRegister& zm);

  // Gather load first-fault unsigned bytes to vector (immediate index).
  void ldff1b(const ZRegister& zt,
              const PRegisterZ& pg,
              const ZRegister& zn,
              int imm5);

  // Gather load first-fault doublewords to vector (vector index).
  void ldff1d(const ZRegister& zt,
              const PRegisterZ& pg,
              const Register& xn,
              const ZRegister& zm);

  // Gather load first-fault doublewords to vector (immediate index).
  void ldff1d(const ZRegister& zt,
              const PRegisterZ& pg,
              const ZRegister& zn,
              int imm5);

  // Gather load first-fault unsigned halfwords to vector (vector index).
  void ldff1h(const ZRegister& zt,
              const PRegisterZ& pg,
              const Register& xn,
              const ZRegister& zm);

  // Gather load first-fault unsigned halfwords to vector (immediate index).
  void ldff1h(const ZRegister& zt,
              const PRegisterZ& pg,
              const ZRegister& zn,
              int imm5);

  // Gather load first-fault signed bytes to vector (vector index).
  void ldff1sb(const ZRegister& zt,
               const PRegisterZ& pg,
               const Register& xn,
               const ZRegister& zm);

  // Gather load first-fault signed bytes to vector (immediate index).
  void ldff1sb(const ZRegister& zt,
               const PRegisterZ& pg,
               const ZRegister& zn,
               int imm5);

  // Gather load first-fault signed halfwords to vector (vector index).
  void ldff1sh(const ZRegister& zt,
               const PRegisterZ& pg,
               const Register& xn,
               const ZRegister& zm);

  // Gather load first-fault signed halfwords to vector (immediate index).
  void ldff1sh(const ZRegister& zt,
               const PRegisterZ& pg,
               const ZRegister& zn,
               int imm5);

  // Gather load first-fault signed words to vector (vector index).
  void ldff1sw(const ZRegister& zt,
               const PRegisterZ& pg,
               const Register& xn,
               const ZRegister& zm);

  // Gather load first-fault signed words to vector (immediate index).
  void ldff1sw(const ZRegister& zt,
               const PRegisterZ& pg,
               const ZRegister& zn,
               int imm5);

  // Gather load first-fault unsigned words to vector (vector index).
  void ldff1w(const ZRegister& zt,
              const PRegisterZ& pg,
              const Register& xn,
              const ZRegister& zm);

  // Gather load first-fault unsigned words to vector (immediate index).
  void ldff1w(const ZRegister& zt,
              const PRegisterZ& pg,
              const ZRegister& zn,
              int imm5);

  // Contiguous load non-fault unsigned bytes to vector (immediate index).
  void ldnf1b(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load non-fault doublewords to vector (immediate index).
  void ldnf1d(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load non-fault unsigned halfwords to vector (immediate
  // index).
  void ldnf1h(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load non-fault signed bytes to vector (immediate index).
  void ldnf1sb(const ZRegister& zt,
               const PRegisterZ& pg,
               const SVEMemOperand& addr);

  // Contiguous load non-fault signed halfwords to vector (immediate index).
  void ldnf1sh(const ZRegister& zt,
               const PRegisterZ& pg,
               const SVEMemOperand& addr);

  // Contiguous load non-fault signed words to vector (immediate index).
  void ldnf1sw(const ZRegister& zt,
               const PRegisterZ& pg,
               const SVEMemOperand& addr);

  // Contiguous load non-fault unsigned words to vector (immediate index).
  void ldnf1w(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load non-temporal bytes to vector.
  void ldnt1b(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load non-temporal halfwords to vector.
  void ldnt1h(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load non-temporal words to vector.
  void ldnt1w(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Contiguous load non-temporal doublewords to vector.
  void ldnt1d(const ZRegister& zt,
              const PRegisterZ& pg,
              const SVEMemOperand& addr);

  // Load SVE predicate/vector register.
  void ldr(const CPURegister& rt, const SVEMemOperand& addr);

  // Logical shift left by immediate (predicated).
  void lsl(const ZRegister& zd,
           const PRegisterM& pg,
           const ZRegister& zn,
           int shift);

  // Logical shift left by 64-bit wide elements (predicated).
  void lsl(const ZRegister& zd,
           const PRegisterM& pg,
           const ZRegister& zn,
           const ZRegister& zm);

  // Logical shift left by immediate (unpredicated).
  void lsl(const ZRegister& zd, const ZRegister& zn, int shift);

  // Logical shift left by 64-bit wide elements (unpredicated).
  void lsl(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Reversed logical shift left by vector (predicated).
  void lslr(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Logical shift right by immediate (predicated).
  void lsr(const ZRegister& zd,
           const PRegisterM& pg,
           const ZRegister& zn,
           int shift);

  // Logical shift right by 64-bit wide elements (predicated).
  void lsr(const ZRegister& zd,
           const PRegisterM& pg,
           const ZRegister& zn,
           const ZRegister& zm);

  // Logical shift right by immediate (unpredicated).
  void lsr(const ZRegister& zd, const ZRegister& zn, int shift);

  // Logical shift right by 64-bit wide elements (unpredicated).
  void lsr(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Reversed logical shift right by vector (predicated).
  void lsrr(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Bitwise invert predicate.
  void not_(const PRegisterWithLaneSize& pd,
            const PRegisterZ& pg,
            const PRegisterWithLaneSize& pn);

  // Bitwise invert predicate, setting the condition flags.
  void nots(const PRegisterWithLaneSize& pd,
            const PRegisterZ& pg,
            const PRegisterWithLaneSize& pn);

  // Multiply-add vectors (predicated), writing multiplicand
  // [Zdn = Za + Zdn * Zm].
  void mad(const ZRegister& zdn,
           const PRegisterM& pg,
           const ZRegister& zm,
           const ZRegister& za);

  // Multiply-add vectors (predicated), writing addend
  // [Zda = Zda + Zn * Zm].
  void mla(const ZRegister& zda,
           const PRegisterM& pg,
           const ZRegister& zn,
           const ZRegister& zm);

  // Multiply-subtract vectors (predicated), writing addend
  // [Zda = Zda - Zn * Zm].
  void mls(const ZRegister& zda,
           const PRegisterM& pg,
           const ZRegister& zn,
           const ZRegister& zm);

  // Move predicates (unpredicated)
  void mov(const PRegister& pd, const PRegister& pn);

  // Move predicates (merging)
  void mov(const PRegisterWithLaneSize& pd,
           const PRegisterM& pg,
           const PRegisterWithLaneSize& pn);

  // Move predicates (zeroing)
  void mov(const PRegisterWithLaneSize& pd,
           const PRegisterZ& pg,
           const PRegisterWithLaneSize& pn);

  // Move general-purpose register to vector elements (unpredicated)
  void mov(const ZRegister& zd, const Register& xn);

  // Move SIMD&FP scalar register to vector elements (unpredicated)
  void mov(const ZRegister& zd, const VRegister& vn);

  // Move vector register (unpredicated)
  void mov(const ZRegister& zd, const ZRegister& zn);

  // Move indexed element to vector elements (unpredicated)
  void mov(const ZRegister& zd, const ZRegister& zn, unsigned index);

  // Move general-purpose register to vector elements (predicated)
  void mov(const ZRegister& zd, const PRegisterM& pg, const Register& rn);

  // Move SIMD&FP scalar register to vector elements (predicated)
  void mov(const ZRegister& zd, const PRegisterM& pg, const VRegister& vn);

  // Move vector elements (predicated)
  void mov(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Move signed integer immediate to vector elements (predicated)
  void mov(const ZRegister& zd, const PRegister& pg, int imm8, int shift = -1);

  // Move signed immediate to vector elements (unpredicated).
  void mov(const ZRegister& zd, int imm8, int shift);

  // Move logical bitmask immediate to vector (unpredicated).
  void mov(const ZRegister& zd, uint64_t imm);

  // Move predicate (unpredicated), setting the condition flags
  void movs(const PRegister& pd, const PRegister& pn);

  // Move predicates (zeroing), setting the condition flags
  void movs(const PRegisterWithLaneSize& pd,
            const PRegisterZ& pg,
            const PRegisterWithLaneSize& pn);

  // Move prefix (predicated).
  void movprfx(const ZRegister& zd, const PRegister& pg, const ZRegister& zn);

  // Move prefix (unpredicated).
  void movprfx(const ZRegister& zd, const ZRegister& zn);

  // Multiply-subtract vectors (predicated), writing multiplicand
  // [Zdn = Za - Zdn * Zm].
  void msb(const ZRegister& zdn,
           const PRegisterM& pg,
           const ZRegister& zm,
           const ZRegister& za);

  // Multiply vectors (predicated).
  void mul(const ZRegister& zd,
           const PRegisterM& pg,
           const ZRegister& zn,
           const ZRegister& zm);

  // Multiply by immediate (unpredicated).
  void mul(const ZRegister& zd, const ZRegister& zn, int imm8);

  // Bitwise NAND predicates.
  void nand(const PRegisterWithLaneSize& pd,
            const PRegisterZ& pg,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Bitwise NAND predicates.
  void nands(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const PRegisterWithLaneSize& pn,
             const PRegisterWithLaneSize& pm);

  // Negate (predicated).
  void neg(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Bitwise NOR predicates.
  void nor(const PRegisterWithLaneSize& pd,
           const PRegisterZ& pg,
           const PRegisterWithLaneSize& pn,
           const PRegisterWithLaneSize& pm);

  // Bitwise NOR predicates.
  void nors(const PRegisterWithLaneSize& pd,
            const PRegisterZ& pg,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Bitwise invert vector (predicated).
  void not_(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Bitwise OR inverted predicate.
  void orn(const PRegisterWithLaneSize& pd,
           const PRegisterZ& pg,
           const PRegisterWithLaneSize& pn,
           const PRegisterWithLaneSize& pm);

  // Bitwise OR inverted predicate.
  void orns(const PRegisterWithLaneSize& pd,
            const PRegisterZ& pg,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Bitwise OR with inverted immediate (unpredicated).
  void orn(const ZRegister& zd, const ZRegister& zn, uint64_t imm);

  // Bitwise OR predicate.
  void orr(const PRegisterWithLaneSize& pd,
           const PRegisterZ& pg,
           const PRegisterWithLaneSize& pn,
           const PRegisterWithLaneSize& pm);

  // Bitwise OR vectors (predicated).
  void orr(const ZRegister& zd,
           const PRegisterM& pg,
           const ZRegister& zn,
           const ZRegister& zm);

  // Bitwise OR with immediate (unpredicated).
  void orr(const ZRegister& zd, const ZRegister& zn, uint64_t imm);

  // Bitwise OR vectors (unpredicated).
  void orr(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Bitwise OR predicate.
  void orrs(const PRegisterWithLaneSize& pd,
            const PRegisterZ& pg,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Bitwise OR reduction to scalar.
  void orv(const VRegister& vd, const PRegister& pg, const ZRegister& zn);

  // Set all predicate elements to false.
  void pfalse(const PRegisterWithLaneSize& pd);

  // Set the first active predicate element to true.
  void pfirst(const PRegisterWithLaneSize& pd,
              const PRegister& pg,
              const PRegisterWithLaneSize& pn);

  // Find next active predicate.
  void pnext(const PRegisterWithLaneSize& pd,
             const PRegister& pg,
             const PRegisterWithLaneSize& pn);

  // Prefetch bytes.
  void prfb(PrefetchOperation prfop,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Prefetch halfwords.
  void prfh(PrefetchOperation prfop,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Prefetch words.
  void prfw(PrefetchOperation prfop,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Prefetch doublewords.
  void prfd(PrefetchOperation prfop,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Set condition flags for predicate.
  void ptest(const PRegister& pg, const PRegisterWithLaneSize& pn);

  // Initialise predicate from named constraint.
  void ptrue(const PRegisterWithLaneSize& pd, int pattern = SVE_ALL);

  // Initialise predicate from named constraint.
  void ptrues(const PRegisterWithLaneSize& pd, int pattern = SVE_ALL);

  // Unpack and widen half of predicate.
  void punpkhi(const PRegisterWithLaneSize& pd,
               const PRegisterWithLaneSize& pn);

  // Unpack and widen half of predicate.
  void punpklo(const PRegisterWithLaneSize& pd,
               const PRegisterWithLaneSize& pn);

  // Reverse bits (predicated).
  void rbit(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Read the first-fault register.
  void rdffr(const PRegisterWithLaneSize& pd);

  // Return predicate of succesfully loaded elements.
  void rdffr(const PRegisterWithLaneSize& pd, const PRegisterZ& pg);

  // Return predicate of succesfully loaded elements.
  void rdffrs(const PRegisterWithLaneSize& pd, const PRegisterZ& pg);

  // Read multiple of vector register size to scalar register.
  void rdvl(const Register& xd, int imm6);

  // Reverse all elements in a predicate.
  void rev(const PRegisterWithLaneSize& pd, const PRegisterWithLaneSize& pn);

  // Reverse all elements in a vector (unpredicated).
  void rev(const ZRegister& zd, const ZRegister& zn);

  // Reverse bytes / halfwords / words within elements (predicated).
  void revb(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Reverse bytes / halfwords / words within elements (predicated).
  void revh(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Reverse bytes / halfwords / words within elements (predicated).
  void revw(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Signed absolute difference (predicated).
  void sabd(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Signed add reduction to scalar.
  void saddv(const VRegister& dd, const PRegister& pg, const ZRegister& zn);

  // Signed integer convert to floating-point (predicated).
  void scvtf(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Signed divide (predicated).
  void sdiv(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Signed reversed divide (predicated).
  void sdivr(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Signed dot product by indexed quadtuplet.
  void sdot(const ZRegister& zda,
            const ZRegister& zn,
            const ZRegister& zm,
            int index);

  // Signed dot product.
  void sdot(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Conditionally select elements from two predicates.
  void sel(const PRegisterWithLaneSize& pd,
           const PRegister& pg,
           const PRegisterWithLaneSize& pn,
           const PRegisterWithLaneSize& pm);

  // Conditionally select elements from two vectors.
  void sel(const ZRegister& zd,
           const PRegister& pg,
           const ZRegister& zn,
           const ZRegister& zm);

  // Initialise the first-fault register to all true.
  void setffr();

  // Signed maximum vectors (predicated).
  void smax(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Signed maximum with immediate (unpredicated).
  void smax(const ZRegister& zd, const ZRegister& zn, int imm8);

  // Signed maximum reduction to scalar.
  void smaxv(const VRegister& vd, const PRegister& pg, const ZRegister& zn);

  // Signed minimum vectors (predicated).
  void smin(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Signed minimum with immediate (unpredicated).
  void smin(const ZRegister& zd, const ZRegister& zn, int imm8);

  // Signed minimum reduction to scalar.
  void sminv(const VRegister& vd, const PRegister& pg, const ZRegister& zn);

  // Signed multiply returning high half (predicated).
  void smulh(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Splice two vectors under predicate control.
  void splice(const ZRegister& zd,
              const PRegister& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Splice two vectors under predicate control (constructive).
  void splice_con(const ZRegister& zd,
                  const PRegister& pg,
                  const ZRegister& zn,
                  const ZRegister& zm);

  // Signed saturating add vectors (unpredicated).
  void sqadd(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed saturating add immediate (unpredicated).
  void sqadd(const ZRegister& zd,
             const ZRegister& zn,
             int imm8,
             int shift = -1);

  // Signed saturating decrement scalar by multiple of 8-bit predicate
  // constraint element count.
  void sqdecb(const Register& xd,
              const Register& wn,
              int pattern,
              int multiplier);

  // Signed saturating decrement scalar by multiple of 8-bit predicate
  // constraint element count.
  void sqdecb(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Signed saturating decrement scalar by multiple of 64-bit predicate
  // constraint element count.
  void sqdecd(const Register& xd,
              const Register& wn,
              int pattern = SVE_ALL,
              int multiplier = 1);

  // Signed saturating decrement scalar by multiple of 64-bit predicate
  // constraint element count.
  void sqdecd(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Signed saturating decrement vector by multiple of 64-bit predicate
  // constraint element count.
  void sqdecd(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Signed saturating decrement scalar by multiple of 16-bit predicate
  // constraint element count.
  void sqdech(const Register& xd,
              const Register& wn,
              int pattern = SVE_ALL,
              int multiplier = 1);

  // Signed saturating decrement scalar by multiple of 16-bit predicate
  // constraint element count.
  void sqdech(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Signed saturating decrement vector by multiple of 16-bit predicate
  // constraint element count.
  void sqdech(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Signed saturating decrement scalar by active predicate element count.
  void sqdecp(const Register& xd,
              const PRegisterWithLaneSize& pg,
              const Register& wn);

  // Signed saturating decrement scalar by active predicate element count.
  void sqdecp(const Register& xdn, const PRegisterWithLaneSize& pg);

  // Signed saturating decrement vector by active predicate element count.
  void sqdecp(const ZRegister& zdn, const PRegister& pg);

  // Signed saturating decrement scalar by multiple of 32-bit predicate
  // constraint element count.
  void sqdecw(const Register& xd,
              const Register& wn,
              int pattern = SVE_ALL,
              int multiplier = 1);

  // Signed saturating decrement scalar by multiple of 32-bit predicate
  // constraint element count.
  void sqdecw(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Signed saturating decrement vector by multiple of 32-bit predicate
  // constraint element count.
  void sqdecw(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Signed saturating increment scalar by multiple of 8-bit predicate
  // constraint element count.
  void sqincb(const Register& xd,
              const Register& wn,
              int pattern = SVE_ALL,
              int multiplier = 1);

  // Signed saturating increment scalar by multiple of 8-bit predicate
  // constraint element count.
  void sqincb(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Signed saturating increment scalar by multiple of 64-bit predicate
  // constraint element count.
  void sqincd(const Register& xd,
              const Register& wn,
              int pattern,
              int multiplier);

  // Signed saturating increment scalar by multiple of 64-bit predicate
  // constraint element count.
  void sqincd(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Signed saturating increment vector by multiple of 64-bit predicate
  // constraint element count.
  void sqincd(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Signed saturating increment scalar by multiple of 16-bit predicate
  // constraint element count.
  void sqinch(const Register& xd,
              const Register& wn,
              int pattern = SVE_ALL,
              int multiplier = 1);

  // Signed saturating increment scalar by multiple of 16-bit predicate
  // constraint element count.
  void sqinch(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Signed saturating increment vector by multiple of 16-bit predicate
  // constraint element count.
  void sqinch(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Signed saturating increment scalar by active predicate element count.
  void sqincp(const Register& xd,
              const PRegisterWithLaneSize& pg,
              const Register& wn);

  // Signed saturating increment scalar by active predicate element count.
  void sqincp(const Register& xdn, const PRegisterWithLaneSize& pg);

  // Signed saturating increment vector by active predicate element count.
  void sqincp(const ZRegister& zdn, const PRegister& pg);

  // Signed saturating increment scalar by multiple of 32-bit predicate
  // constraint element count.
  void sqincw(const Register& xd,
              const Register& wn,
              int pattern = SVE_ALL,
              int multiplier = 1);

  // Signed saturating increment scalar by multiple of 32-bit predicate
  // constraint element count.
  void sqincw(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Signed saturating increment vector by multiple of 32-bit predicate
  // constraint element count.
  void sqincw(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Signed saturating subtract vectors (unpredicated).
  void sqsub(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed saturating subtract immediate (unpredicated).
  void sqsub(const ZRegister& zd,
             const ZRegister& zn,
             int imm8,
             int shift = -1);

  // Contiguous/scatter store bytes from vector.
  void st1b(const ZRegister& zt,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous/scatter store halfwords from vector.
  void st1h(const ZRegister& zt,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous/scatter store words from vector.
  void st1w(const ZRegister& zt,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous/scatter store doublewords from vector.
  void st1d(const ZRegister& zt,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous store two-byte structures from two vectors.
  void st2b(const ZRegister& zt1,
            const ZRegister& zt2,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous store two-halfword structures from two vectors.
  void st2h(const ZRegister& zt1,
            const ZRegister& zt2,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous store two-word structures from two vectors.
  void st2w(const ZRegister& zt1,
            const ZRegister& zt2,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous store two-doubleword structures from two vectors,
  void st2d(const ZRegister& zt1,
            const ZRegister& zt2,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous store three-byte structures from three vectors.
  void st3b(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous store three-halfword structures from three vectors.
  void st3h(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous store three-word structures from three vectors.
  void st3w(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous store three-doubleword structures from three vectors.
  void st3d(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous store four-byte structures from four vectors.
  void st4b(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const ZRegister& zt4,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous store four-halfword structures from four vectors.
  void st4h(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const ZRegister& zt4,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous store four-word structures from four vectors.
  void st4w(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const ZRegister& zt4,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous store four-doubleword structures from four vectors.
  void st4d(const ZRegister& zt1,
            const ZRegister& zt2,
            const ZRegister& zt3,
            const ZRegister& zt4,
            const PRegister& pg,
            const SVEMemOperand& addr);

  // Contiguous store non-temporal bytes from vector.
  void stnt1b(const ZRegister& zt,
              const PRegister& pg,
              const SVEMemOperand& addr);

  // Contiguous store non-temporal halfwords from vector.
  void stnt1h(const ZRegister& zt,
              const PRegister& pg,
              const SVEMemOperand& addr);

  // Contiguous store non-temporal words from vector.
  void stnt1w(const ZRegister& zt,
              const PRegister& pg,
              const SVEMemOperand& addr);

  // Contiguous store non-temporal doublewords from vector.
  void stnt1d(const ZRegister& zt,
              const PRegister& pg,
              const SVEMemOperand& addr);

  // Store SVE predicate/vector register.
  void str(const CPURegister& rt, const SVEMemOperand& addr);

  // Subtract vectors (predicated).
  void sub(const ZRegister& zd,
           const PRegisterM& pg,
           const ZRegister& zn,
           const ZRegister& zm);

  // Subtract vectors (unpredicated).
  void sub(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Subtract immediate (unpredicated).
  void sub(const ZRegister& zd, const ZRegister& zn, int imm8, int shift = -1);

  // Reversed subtract vectors (predicated).
  void subr(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Reversed subtract from immediate (unpredicated).
  void subr(const ZRegister& zd, const ZRegister& zn, int imm8, int shift = -1);

  // Signed unpack and extend half of vector.
  void sunpkhi(const ZRegister& zd, const ZRegister& zn);

  // Signed unpack and extend half of vector.
  void sunpklo(const ZRegister& zd, const ZRegister& zn);

  // Signed byte extend (predicated).
  void sxtb(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Signed halfword extend (predicated).
  void sxth(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Signed word extend (predicated).
  void sxtw(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Programmable table lookup/permute using vector of indices into a
  // vector.
  void tbl(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Interleave even or odd elements from two predicates.
  void trn1(const PRegisterWithLaneSize& pd,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Interleave even or odd elements from two vectors.
  void trn1(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Interleave even or odd elements from two predicates.
  void trn2(const PRegisterWithLaneSize& pd,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Interleave even or odd elements from two vectors.
  void trn2(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned absolute difference (predicated).
  void uabd(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Unsigned add reduction to scalar.
  void uaddv(const VRegister& dd, const PRegister& pg, const ZRegister& zn);

  // Unsigned integer convert to floating-point (predicated).
  void ucvtf(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Unsigned divide (predicated).
  void udiv(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Unsigned reversed divide (predicated).
  void udivr(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Unsigned dot product by indexed quadtuplet.
  void udot(const ZRegister& zda,
            const ZRegister& zn,
            const ZRegister& zm,
            int index);

  // Unsigned dot product.
  void udot(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Unsigned maximum vectors (predicated).
  void umax(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Unsigned maximum with immediate (unpredicated).
  void umax(const ZRegister& zd, const ZRegister& zn, int imm8);

  // Unsigned maximum reduction to scalar.
  void umaxv(const VRegister& vd, const PRegister& pg, const ZRegister& zn);

  // Unsigned minimum vectors (predicated).
  void umin(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Unsigned minimum with immediate (unpredicated).
  void umin(const ZRegister& zd, const ZRegister& zn, int imm8);

  // Unsigned minimum reduction to scalar.
  void uminv(const VRegister& vd, const PRegister& pg, const ZRegister& zn);

  // Unsigned multiply returning high half (predicated).
  void umulh(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Unsigned saturating add vectors (unpredicated).
  void uqadd(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned saturating add immediate (unpredicated).
  void uqadd(const ZRegister& zd,
             const ZRegister& zn,
             int imm8,
             int shift = -1);

  // Unsigned saturating decrement scalar by multiple of 8-bit predicate
  // constraint element count.
  void uqdecb(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Unsigned saturating decrement scalar by multiple of 64-bit predicate
  // constraint element count.
  void uqdecd(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Unsigned saturating decrement vector by multiple of 64-bit predicate
  // constraint element count.
  void uqdecd(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Unsigned saturating decrement scalar by multiple of 16-bit predicate
  // constraint element count.
  void uqdech(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Unsigned saturating decrement vector by multiple of 16-bit predicate
  // constraint element count.
  void uqdech(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Unsigned saturating decrement scalar by active predicate element count.
  void uqdecp(const Register& rdn, const PRegisterWithLaneSize& pg);

  // Unsigned saturating decrement vector by active predicate element count.
  void uqdecp(const ZRegister& zdn, const PRegister& pg);

  // Unsigned saturating decrement scalar by multiple of 32-bit predicate
  // constraint element count.
  void uqdecw(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Unsigned saturating decrement vector by multiple of 32-bit predicate
  // constraint element count.
  void uqdecw(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Unsigned saturating increment scalar by multiple of 8-bit predicate
  // constraint element count.
  void uqincb(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Unsigned saturating increment scalar by multiple of 64-bit predicate
  // constraint element count.
  void uqincd(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Unsigned saturating increment vector by multiple of 64-bit predicate
  // constraint element count.
  void uqincd(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Unsigned saturating increment scalar by multiple of 16-bit predicate
  // constraint element count.
  void uqinch(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Unsigned saturating increment vector by multiple of 16-bit predicate
  // constraint element count.
  void uqinch(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Unsigned saturating increment scalar by active predicate element count.
  void uqincp(const Register& rdn, const PRegisterWithLaneSize& pg);

  // Unsigned saturating increment vector by active predicate element count.
  void uqincp(const ZRegister& zdn, const PRegister& pg);

  // Unsigned saturating increment scalar by multiple of 32-bit predicate
  // constraint element count.
  void uqincw(const Register& rdn, int pattern = SVE_ALL, int multiplier = 1);

  // Unsigned saturating increment vector by multiple of 32-bit predicate
  // constraint element count.
  void uqincw(const ZRegister& zdn, int pattern = SVE_ALL, int multiplier = 1);

  // Unsigned saturating subtract vectors (unpredicated).
  void uqsub(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned saturating subtract immediate (unpredicated).
  void uqsub(const ZRegister& zd,
             const ZRegister& zn,
             int imm8,
             int shift = -1);

  // Unsigned unpack and extend half of vector.
  void uunpkhi(const ZRegister& zd, const ZRegister& zn);

  // Unsigned unpack and extend half of vector.
  void uunpklo(const ZRegister& zd, const ZRegister& zn);

  // Unsigned byte extend (predicated).
  void uxtb(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Unsigned halfword extend (predicated).
  void uxth(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Unsigned word extend (predicated).
  void uxtw(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Concatenate even or odd elements from two predicates.
  void uzp1(const PRegisterWithLaneSize& pd,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Concatenate even or odd elements from two vectors.
  void uzp1(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Concatenate even or odd elements from two predicates.
  void uzp2(const PRegisterWithLaneSize& pd,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Concatenate even or odd elements from two vectors.
  void uzp2(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // While incrementing signed scalar less than or equal to scalar.
  void whilele(const PRegisterWithLaneSize& pd,
               const Register& rn,
               const Register& rm);

  // While incrementing unsigned scalar lower than scalar.
  void whilelo(const PRegisterWithLaneSize& pd,
               const Register& rn,
               const Register& rm);

  // While incrementing unsigned scalar lower or same as scalar.
  void whilels(const PRegisterWithLaneSize& pd,
               const Register& rn,
               const Register& rm);

  // While incrementing signed scalar less than scalar.
  void whilelt(const PRegisterWithLaneSize& pd,
               const Register& rn,
               const Register& rm);

  // Write the first-fault register.
  void wrffr(const PRegisterWithLaneSize& pn);

  // Interleave elements from two half predicates.
  void zip1(const PRegisterWithLaneSize& pd,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Interleave elements from two half vectors.
  void zip1(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Interleave elements from two half predicates.
  void zip2(const PRegisterWithLaneSize& pd,
            const PRegisterWithLaneSize& pn,
            const PRegisterWithLaneSize& pm);

  // Interleave elements from two half vectors.
  void zip2(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Add with carry long (bottom).
  void adclb(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Add with carry long (top).
  void adclt(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Add narrow high part (bottom).
  void addhnb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Add narrow high part (top).
  void addhnt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Add pairwise.
  void addp(const ZRegister& zd,
            const PRegisterM& pg,
            const ZRegister& zn,
            const ZRegister& zm);

  // Bitwise clear and exclusive OR.
  void bcax(const ZRegister& zd,
            const ZRegister& zn,
            const ZRegister& zm,
            const ZRegister& zk);

  // Scatter lower bits into positions selected by bitmask.
  void bdep(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Gather lower bits from positions selected by bitmask.
  void bext(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Group bits to right or left as selected by bitmask.
  void bgrp(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Bitwise select.
  void bsl(const ZRegister& zd,
           const ZRegister& zn,
           const ZRegister& zm,
           const ZRegister& zk);

  // Bitwise select with first input inverted.
  void bsl1n(const ZRegister& zd,
             const ZRegister& zn,
             const ZRegister& zm,
             const ZRegister& zk);

  // Bitwise select with second input inverted.
  void bsl2n(const ZRegister& zd,
             const ZRegister& zn,
             const ZRegister& zm,
             const ZRegister& zk);

  // Complex integer add with rotate.
  void cadd(const ZRegister& zd,
            const ZRegister& zn,
            const ZRegister& zm,
            int rot);

  // Complex integer dot product (indexed).
  void cdot(const ZRegister& zda,
            const ZRegister& zn,
            const ZRegister& zm,
            int index,
            int rot);

  // Complex integer dot product.
  void cdot(const ZRegister& zda,
            const ZRegister& zn,
            const ZRegister& zm,
            int rot);

  // Complex integer multiply-add with rotate (indexed).
  void cmla(const ZRegister& zda,
            const ZRegister& zn,
            const ZRegister& zm,
            int index,
            int rot);

  // Complex integer multiply-add with rotate.
  void cmla(const ZRegister& zda,
            const ZRegister& zn,
            const ZRegister& zm,
            int rot);

  // Bitwise exclusive OR of three vectors.
  void eor3(const ZRegister& zd,
            const ZRegister& zn,
            const ZRegister& zm,
            const ZRegister& zk);

  // Interleaving exclusive OR (bottom, top).
  void eorbt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Interleaving exclusive OR (top, bottom).
  void eortb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Floating-point add pairwise.
  void faddp(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Floating-point up convert long (top, predicated).
  void fcvtlt(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point down convert and narrow (top, predicated).
  void fcvtnt(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point down convert, rounding to odd (predicated).
  void fcvtx(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point down convert, rounding to odd (top, predicated).
  void fcvtxnt(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point base 2 logarithm as integer.
  void flogb(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Floating-point maximum number pairwise.
  void fmaxnmp(const ZRegister& zd,
               const PRegisterM& pg,
               const ZRegister& zn,
               const ZRegister& zm);

  // Floating-point maximum pairwise.
  void fmaxp(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Floating-point minimum number pairwise.
  void fminnmp(const ZRegister& zd,
               const PRegisterM& pg,
               const ZRegister& zn,
               const ZRegister& zm);

  // Floating-point minimum pairwise.
  void fminp(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Half-precision floating-point multiply-add long to single-precision
  // (bottom).
  void fmlalb(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Half-precision floating-point multiply-add long to single-precision
  // (top).
  void fmlalt(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Half-precision floating-point multiply-subtract long from
  // single-precision (bottom).
  void fmlslb(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Half-precision floating-point multiply-subtract long from
  // single-precision (top, indexed).
  void fmlslt(const ZRegister& zda,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Half-precision floating-point multiply-add long to single-precision
  // (bottom, indexed).
  void fmlalb(const ZRegister& zda,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Half-precision floating-point multiply-add long to single-precision
  // (top, indexed).
  void fmlalt(const ZRegister& zda,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Half-precision floating-point multiply-subtract long from
  // single-precision (bottom, indexed).
  void fmlslb(const ZRegister& zda,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Half-precision floating-point multiply-subtract long from
  // single-precision (top).
  void fmlslt(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Count matching elements in vector.
  void histcnt(const ZRegister& zd,
               const PRegisterZ& pg,
               const ZRegister& zn,
               const ZRegister& zm);

  // Count matching elements in vector segments.
  void histseg(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Gather load non-temporal signed bytes.
  void ldnt1sb(const ZRegister& zt,
               const PRegisterZ& pg,
               const SVEMemOperand& addr);

  // Gather load non-temporal signed halfwords.
  void ldnt1sh(const ZRegister& zt,
               const PRegisterZ& pg,
               const SVEMemOperand& addr);

  // Gather load non-temporal signed words.
  void ldnt1sw(const ZRegister& zt,
               const PRegisterZ& pg,
               const SVEMemOperand& addr);

  // Detect any matching elements, setting the condition flags.
  void match(const PRegisterWithLaneSize& pd,
             const PRegisterZ& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Multiply-add to accumulator (indexed).
  void mla(const ZRegister& zda,
           const ZRegister& zn,
           const ZRegister& zm,
           int index);

  // Multiply-subtract from accumulator (indexed).
  void mls(const ZRegister& zda,
           const ZRegister& zn,
           const ZRegister& zm,
           int index);

  // Multiply (indexed).
  void mul(const ZRegister& zd,
           const ZRegister& zn,
           const ZRegister& zm,
           int index);

  // Multiply vectors (unpredicated).
  void mul(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Bitwise inverted select.
  void nbsl(const ZRegister& zd,
            const ZRegister& zn,
            const ZRegister& zm,
            const ZRegister& zk);

  // Detect no matching elements, setting the condition flags.
  void nmatch(const PRegisterWithLaneSize& pd,
              const PRegisterZ& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Polynomial multiply vectors (unpredicated).
  void pmul(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Polynomial multiply long (bottom).
  void pmullb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Polynomial multiply long (top).
  void pmullt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Rounding add narrow high part (bottom).
  void raddhnb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Rounding add narrow high part (top).
  void raddhnt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Rounding shift right narrow by immediate (bottom).
  void rshrnb(const ZRegister& zd, const ZRegister& zn, int shift);

  // Rounding shift right narrow by immediate (top).
  void rshrnt(const ZRegister& zd, const ZRegister& zn, int shift);

  // Rounding subtract narrow high part (bottom).
  void rsubhnb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Rounding subtract narrow high part (top).
  void rsubhnt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed absolute difference and accumulate.
  void saba(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed absolute difference and accumulate long (bottom).
  void sabalb(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed absolute difference and accumulate long (top).
  void sabalt(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed absolute difference long (bottom).
  void sabdlb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed absolute difference long (top).
  void sabdlt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed add and accumulate long pairwise.
  void sadalp(const ZRegister& zda, const PRegisterM& pg, const ZRegister& zn);

  // Signed add long (bottom).
  void saddlb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed add long (bottom + top).
  void saddlbt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed add long (top).
  void saddlt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed add wide (bottom).
  void saddwb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed add wide (top).
  void saddwt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Subtract with carry long (bottom).
  void sbclb(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Subtract with carry long (top).
  void sbclt(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed halving addition.
  void shadd(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Shift right narrow by immediate (bottom).
  void shrnb(const ZRegister& zd, const ZRegister& zn, int shift);

  // Shift right narrow by immediate (top).
  void shrnt(const ZRegister& zd, const ZRegister& zn, int shift);

  // Signed halving subtract.
  void shsub(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Signed halving subtract reversed vectors.
  void shsubr(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Shift left and insert (immediate).
  void sli(const ZRegister& zd, const ZRegister& zn, int shift);

  // Signed maximum pairwise.
  void smaxp(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Signed minimum pairwise.
  void sminp(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Signed multiply-add long to accumulator (bottom, indexed).
  void smlalb(const ZRegister& zda,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Signed multiply-add long to accumulator (bottom).
  void smlalb(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed multiply-add long to accumulator (top, indexed).
  void smlalt(const ZRegister& zda,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Signed multiply-add long to accumulator (top).
  void smlalt(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed multiply-subtract long from accumulator (bottom, indexed).
  void smlslb(const ZRegister& zda,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Signed multiply-subtract long from accumulator (bottom).
  void smlslb(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed multiply-subtract long from accumulator (top, indexed).
  void smlslt(const ZRegister& zda,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Signed multiply-subtract long from accumulator (top).
  void smlslt(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed multiply returning high half (unpredicated).
  void smulh(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed multiply long (bottom, indexed).
  void smullb(const ZRegister& zd,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Signed multiply long (bottom).
  void smullb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed multiply long (top, indexed).
  void smullt(const ZRegister& zd,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Signed multiply long (top).
  void smullt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed saturating absolute value.
  void sqabs(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Signed saturating addition (predicated).
  void sqadd(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Saturating complex integer add with rotate.
  void sqcadd(const ZRegister& zd,
              const ZRegister& zn,
              const ZRegister& zm,
              int rot);

  // Signed saturating doubling multiply-add long to accumulator (bottom,
  // indexed).
  void sqdmlalb(const ZRegister& zda,
                const ZRegister& zn,
                const ZRegister& zm,
                int index);

  // Signed saturating doubling multiply-add long to accumulator (bottom).
  void sqdmlalb(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed saturating doubling multiply-add long to accumulator (bottom x
  // top).
  void sqdmlalbt(const ZRegister& zda,
                 const ZRegister& zn,
                 const ZRegister& zm);

  // Signed saturating doubling multiply-add long to accumulator (top,
  // indexed).
  void sqdmlalt(const ZRegister& zda,
                const ZRegister& zn,
                const ZRegister& zm,
                int index);

  // Signed saturating doubling multiply-add long to accumulator (top).
  void sqdmlalt(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed saturating doubling multiply-subtract long from accumulator
  // (bottom, indexed).
  void sqdmlslb(const ZRegister& zda,
                const ZRegister& zn,
                const ZRegister& zm,
                int index);

  // Signed saturating doubling multiply-subtract long from accumulator
  // (bottom).
  void sqdmlslb(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed saturating doubling multiply-subtract long from accumulator
  // (bottom x top).
  void sqdmlslbt(const ZRegister& zda,
                 const ZRegister& zn,
                 const ZRegister& zm);

  // Signed saturating doubling multiply-subtract long from accumulator
  // (top, indexed).
  void sqdmlslt(const ZRegister& zda,
                const ZRegister& zn,
                const ZRegister& zm,
                int index);

  // Signed saturating doubling multiply-subtract long from accumulator
  // (top).
  void sqdmlslt(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed saturating doubling multiply high (indexed).
  void sqdmulh(const ZRegister& zd,
               const ZRegister& zn,
               const ZRegister& zm,
               int index);

  // Signed saturating doubling multiply high (unpredicated).
  void sqdmulh(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed saturating doubling multiply long (bottom, indexed).
  void sqdmullb(const ZRegister& zd,
                const ZRegister& zn,
                const ZRegister& zm,
                int index);

  // Signed saturating doubling multiply long (bottom).
  void sqdmullb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed saturating doubling multiply long (top, indexed).
  void sqdmullt(const ZRegister& zd,
                const ZRegister& zn,
                const ZRegister& zm,
                int index);

  // Signed saturating doubling multiply long (top).
  void sqdmullt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed saturating negate.
  void sqneg(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Saturating rounding doubling complex integer multiply-add high with
  // rotate (indexed).
  void sqrdcmlah(const ZRegister& zda,
                 const ZRegister& zn,
                 const ZRegister& zm,
                 int index,
                 int rot);

  // Saturating rounding doubling complex integer multiply-add high with
  // rotate.
  void sqrdcmlah(const ZRegister& zda,
                 const ZRegister& zn,
                 const ZRegister& zm,
                 int rot);

  // Signed saturating rounding doubling multiply-add high to accumulator
  // (indexed).
  void sqrdmlah(const ZRegister& zda,
                const ZRegister& zn,
                const ZRegister& zm,
                int index);

  // Signed saturating rounding doubling multiply-add high to accumulator
  // (unpredicated).
  void sqrdmlah(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed saturating rounding doubling multiply-subtract high from
  // accumulator (indexed).
  void sqrdmlsh(const ZRegister& zda,
                const ZRegister& zn,
                const ZRegister& zm,
                int index);

  // Signed saturating rounding doubling multiply-subtract high from
  // accumulator (unpredicated).
  void sqrdmlsh(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed saturating rounding doubling multiply high (indexed).
  void sqrdmulh(const ZRegister& zd,
                const ZRegister& zn,
                const ZRegister& zm,
                int index);

  // Signed saturating rounding doubling multiply high (unpredicated).
  void sqrdmulh(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed saturating rounding shift left by vector (predicated).
  void sqrshl(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Signed saturating rounding shift left reversed vectors (predicated).
  void sqrshlr(const ZRegister& zd,
               const PRegisterM& pg,
               const ZRegister& zn,
               const ZRegister& zm);

  // Signed saturating rounding shift right narrow by immediate (bottom).
  void sqrshrnb(const ZRegister& zd, const ZRegister& zn, int shift);

  // Signed saturating rounding shift right narrow by immediate (top).
  void sqrshrnt(const ZRegister& zd, const ZRegister& zn, int shift);

  // Signed saturating rounding shift right unsigned narrow by immediate
  // (bottom).
  void sqrshrunb(const ZRegister& zd, const ZRegister& zn, int shift);

  // Signed saturating rounding shift right unsigned narrow by immediate
  // (top).
  void sqrshrunt(const ZRegister& zd, const ZRegister& zn, int shift);

  // Signed saturating shift left by immediate.
  void sqshl(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             int shift);

  // Signed saturating shift left by vector (predicated).
  void sqshl(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Signed saturating shift left reversed vectors (predicated).
  void sqshlr(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Signed saturating shift left unsigned by immediate.
  void sqshlu(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              int shift);

  // Signed saturating shift right narrow by immediate (bottom).
  void sqshrnb(const ZRegister& zd, const ZRegister& zn, int shift);

  // Signed saturating shift right narrow by immediate (top).
  void sqshrnt(const ZRegister& zd, const ZRegister& zn, int shift);

  // Signed saturating shift right unsigned narrow by immediate (bottom).
  void sqshrunb(const ZRegister& zd, const ZRegister& zn, int shift);

  // Signed saturating shift right unsigned narrow by immediate (top).
  void sqshrunt(const ZRegister& zd, const ZRegister& zn, int shift);

  // Signed saturating subtraction (predicated).
  void sqsub(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Signed saturating subtraction reversed vectors (predicated).
  void sqsubr(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Signed saturating extract narrow (bottom).
  void sqxtnb(const ZRegister& zd, const ZRegister& zn);

  // Signed saturating extract narrow (top).
  void sqxtnt(const ZRegister& zd, const ZRegister& zn);

  // Signed saturating unsigned extract narrow (bottom).
  void sqxtunb(const ZRegister& zd, const ZRegister& zn);

  // Signed saturating unsigned extract narrow (top).
  void sqxtunt(const ZRegister& zd, const ZRegister& zn);

  // Signed rounding halving addition.
  void srhadd(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Shift right and insert (immediate).
  void sri(const ZRegister& zd, const ZRegister& zn, int shift);

  // Signed rounding shift left by vector (predicated).
  void srshl(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Signed rounding shift left reversed vectors (predicated).
  void srshlr(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Signed rounding shift right by immediate.
  void srshr(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             int shift);

  // Signed rounding shift right and accumulate (immediate).
  void srsra(const ZRegister& zda, const ZRegister& zn, int shift);

  // Signed shift left long by immediate (bottom).
  void sshllb(const ZRegister& zd, const ZRegister& zn, int shift);

  // Signed shift left long by immediate (top).
  void sshllt(const ZRegister& zd, const ZRegister& zn, int shift);

  // Signed shift right and accumulate (immediate).
  void ssra(const ZRegister& zda, const ZRegister& zn, int shift);

  // Signed subtract long (bottom).
  void ssublb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed subtract long (bottom - top).
  void ssublbt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed subtract long (top).
  void ssublt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed subtract long (top - bottom).
  void ssubltb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed subtract wide (bottom).
  void ssubwb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed subtract wide (top).
  void ssubwt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Subtract narrow high part (bottom).
  void subhnb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Subtract narrow high part (top).
  void subhnt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Signed saturating addition of unsigned value.
  void suqadd(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Programmable table lookup in one or two vector table (zeroing).
  void tbl(const ZRegister& zd,
           const ZRegister& zn1,
           const ZRegister& zn2,
           const ZRegister& zm);

  // Programmable table lookup in single vector table (merging).
  void tbx(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned absolute difference and accumulate.
  void uaba(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Unsigned absolute difference and accumulate long (bottom).
  void uabalb(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Unsigned absolute difference and accumulate long (top).
  void uabalt(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Unsigned absolute difference long (bottom).
  void uabdlb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned absolute difference long (top).
  void uabdlt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned add and accumulate long pairwise.
  void uadalp(const ZRegister& zda, const PRegisterM& pg, const ZRegister& zn);

  // Unsigned add long (bottom).
  void uaddlb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned add long (top).
  void uaddlt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned add wide (bottom).
  void uaddwb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned add wide (top).
  void uaddwt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned halving addition.
  void uhadd(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Unsigned halving subtract.
  void uhsub(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Unsigned halving subtract reversed vectors.
  void uhsubr(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Unsigned maximum pairwise.
  void umaxp(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Unsigned minimum pairwise.
  void uminp(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Unsigned multiply-add long to accumulator (bottom, indexed).
  void umlalb(const ZRegister& zda,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Unsigned multiply-add long to accumulator (bottom).
  void umlalb(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Unsigned multiply-add long to accumulator (top, indexed).
  void umlalt(const ZRegister& zda,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Unsigned multiply-add long to accumulator (top).
  void umlalt(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Unsigned multiply-subtract long from accumulator (bottom, indexed).
  void umlslb(const ZRegister& zda,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Unsigned multiply-subtract long from accumulator (bottom).
  void umlslb(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Unsigned multiply-subtract long from accumulator (top, indexed).
  void umlslt(const ZRegister& zda,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Unsigned multiply-subtract long from accumulator (top).
  void umlslt(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Unsigned multiply returning high half (unpredicated).
  void umulh(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned multiply long (bottom, indexed).
  void umullb(const ZRegister& zd,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Unsigned multiply long (bottom).
  void umullb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned multiply long (top, indexed).
  void umullt(const ZRegister& zd,
              const ZRegister& zn,
              const ZRegister& zm,
              int index);

  // Unsigned multiply long (top).
  void umullt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned saturating addition (predicated).
  void uqadd(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Unsigned saturating rounding shift left by vector (predicated).
  void uqrshl(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Unsigned saturating rounding shift left reversed vectors (predicated).
  void uqrshlr(const ZRegister& zd,
               const PRegisterM& pg,
               const ZRegister& zn,
               const ZRegister& zm);

  // Unsigned saturating rounding shift right narrow by immediate (bottom).
  void uqrshrnb(const ZRegister& zd, const ZRegister& zn, int shift);

  // Unsigned saturating rounding shift right narrow by immediate (top).
  void uqrshrnt(const ZRegister& zd, const ZRegister& zn, int shift);

  // Unsigned saturating shift left by immediate.
  void uqshl(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             int shift);

  // Unsigned saturating shift left by vector (predicated).
  void uqshl(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Unsigned saturating shift left reversed vectors (predicated).
  void uqshlr(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Unsigned saturating shift right narrow by immediate (bottom).
  void uqshrnb(const ZRegister& zd, const ZRegister& zn, int shift);

  // Unsigned saturating shift right narrow by immediate (top).
  void uqshrnt(const ZRegister& zd, const ZRegister& zn, int shift);

  // Unsigned saturating subtraction (predicated).
  void uqsub(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Unsigned saturating subtraction reversed vectors (predicated).
  void uqsubr(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Unsigned saturating extract narrow (bottom).
  void uqxtnb(const ZRegister& zd, const ZRegister& zn);

  // Unsigned saturating extract narrow (top).
  void uqxtnt(const ZRegister& zd, const ZRegister& zn);

  // Unsigned reciprocal estimate (predicated).
  void urecpe(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Unsigned rounding halving addition.
  void urhadd(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Unsigned rounding shift left by vector (predicated).
  void urshl(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             const ZRegister& zm);

  // Unsigned rounding shift left reversed vectors (predicated).
  void urshlr(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Unsigned rounding shift right by immediate.
  void urshr(const ZRegister& zd,
             const PRegisterM& pg,
             const ZRegister& zn,
             int shift);

  // Unsigned reciprocal square root estimate (predicated).
  void ursqrte(const ZRegister& zd, const PRegisterM& pg, const ZRegister& zn);

  // Unsigned rounding shift right and accumulate (immediate).
  void ursra(const ZRegister& zda, const ZRegister& zn, int shift);

  // Unsigned shift left long by immediate (bottom).
  void ushllb(const ZRegister& zd, const ZRegister& zn, int shift);

  // Unsigned shift left long by immediate (top).
  void ushllt(const ZRegister& zd, const ZRegister& zn, int shift);

  // Unsigned saturating addition of signed value.
  void usqadd(const ZRegister& zd,
              const PRegisterM& pg,
              const ZRegister& zn,
              const ZRegister& zm);

  // Unsigned shift right and accumulate (immediate).
  void usra(const ZRegister& zda, const ZRegister& zn, int shift);

  // Unsigned subtract long (bottom).
  void usublb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned subtract long (top).
  void usublt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned subtract wide (bottom).
  void usubwb(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // Unsigned subtract wide (top).
  void usubwt(const ZRegister& zd, const ZRegister& zn, const ZRegister& zm);

  // While decrementing signed scalar greater than or equal to scalar.
  void whilege(const PRegisterWithLaneSize& pd,
               const Register& rn,
               const Register& rm);

  // While decrementing signed scalar greater than scalar.
  void whilegt(const PRegisterWithLaneSize& pd,
               const Register& rn,
               const Register& rm);

  // While decrementing unsigned scalar higher than scalar.
  void whilehi(const PRegisterWithLaneSize& pd,
               const Register& rn,
               const Register& rm);

  // While decrementing unsigned scalar higher or same as scalar.
  void whilehs(const PRegisterWithLaneSize& pd,
               const Register& rn,
               const Register& rm);

  // While free of read-after-write conflicts.
  void whilerw(const PRegisterWithLaneSize& pd,
               const Register& rn,
               const Register& rm);

  // While free of write-after-read/write conflicts.
  void whilewr(const PRegisterWithLaneSize& pd,
               const Register& rn,
               const Register& rm);

  // Bitwise exclusive OR and rotate right by immediate.
  void xar(const ZRegister& zd,
           const ZRegister& zn,
           const ZRegister& zm,
           int shift);

  // Floating-point matrix multiply-accumulate.
  void fmmla(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Signed integer matrix multiply-accumulate.
  void smmla(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Unsigned by signed integer matrix multiply-accumulate.
  void usmmla(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Unsigned integer matrix multiply-accumulate.
  void ummla(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Unsigned by signed integer dot product.
  void usdot(const ZRegister& zda, const ZRegister& zn, const ZRegister& zm);

  // Unsigned by signed integer indexed dot product.
  void usdot(const ZRegister& zda,
             const ZRegister& zn,
             const ZRegister& zm,
             int index);

  // Signed by unsigned integer indexed dot product.
  void sudot(const ZRegister& zda,
             const ZRegister& zn,
             const ZRegister& zm,
             int index);

  // Add with Tag.
  void addg(const Register& xd, const Register& xn, int offset, int tag_offset);

  // Tag Mask Insert.
  void gmi(const Register& xd, const Register& xn, const Register& xm);

  // Insert Random Tag.
  void irg(const Register& xd, const Register& xn, const Register& xm = xzr);

  // Load Allocation Tag.
  void ldg(const Register& xt, const MemOperand& addr);

  void StoreTagHelper(const Register& xt, const MemOperand& addr, Instr op);

  // Store Allocation Tags.
  void st2g(const Register& xt, const MemOperand& addr);

  // Store Allocation Tag.
  void stg(const Register& xt, const MemOperand& addr);

  // Store Allocation Tag and Pair of registers.
  void stgp(const Register& xt1, const Register& xt2, const MemOperand& addr);

  // Store Allocation Tags, Zeroing.
  void stz2g(const Register& xt, const MemOperand& addr);

  // Store Allocation Tag, Zeroing.
  void stzg(const Register& xt, const MemOperand& addr);

  // Subtract with Tag.
  void subg(const Register& xd, const Register& xn, int offset, int tag_offset);

  // Subtract Pointer.
  void subp(const Register& xd, const Register& xn, const Register& xm);

  // Subtract Pointer, setting Flags.
  void subps(const Register& xd, const Register& xn, const Register& xm);

  // Compare with Tag.
  void cmpp(const Register& xn, const Register& xm) { subps(xzr, xn, xm); }

  // Memory Copy.
  void cpye(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy, reads and writes non-temporal.
  void cpyen(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy, reads non-temporal.
  void cpyern(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy, writes non-temporal.
  void cpyewn(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy Forward-only.
  void cpyfe(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy Forward-only, reads and writes non-temporal.
  void cpyfen(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy Forward-only, reads non-temporal.
  void cpyfern(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy Forward-only, writes non-temporal.
  void cpyfewn(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy Forward-only.
  void cpyfm(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy Forward-only, reads and writes non-temporal.
  void cpyfmn(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy Forward-only, reads non-temporal.
  void cpyfmrn(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy Forward-only, writes non-temporal.
  void cpyfmwn(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy Forward-only.
  void cpyfp(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy Forward-only, reads and writes non-temporal.
  void cpyfpn(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy Forward-only, reads non-temporal.
  void cpyfprn(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy Forward-only, writes non-temporal.
  void cpyfpwn(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy.
  void cpym(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy, reads and writes non-temporal.
  void cpymn(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy, reads non-temporal.
  void cpymrn(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy, writes non-temporal.
  void cpymwn(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy.
  void cpyp(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy, reads and writes non-temporal.
  void cpypn(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy, reads non-temporal.
  void cpyprn(const Register& rd, const Register& rs, const Register& rn);

  // Memory Copy, writes non-temporal.
  void cpypwn(const Register& rd, const Register& rs, const Register& rn);

  // Memory Set.
  void sete(const Register& rd, const Register& rn, const Register& rs);

  // Memory Set, non-temporal.
  void seten(const Register& rd, const Register& rn, const Register& rs);

  // Memory Set with tag setting.
  void setge(const Register& rd, const Register& rn, const Register& rs);

  // Memory Set with tag setting, non-temporal.
  void setgen(const Register& rd, const Register& rn, const Register& rs);

  // Memory Set with tag setting.
  void setgm(const Register& rd, const Register& rn, const Register& rs);

  // Memory Set with tag setting, non-temporal.
  void setgmn(const Register& rd, const Register& rn, const Register& rs);

  // Memory Set with tag setting.
  void setgp(const Register& rd, const Register& rn, const Register& rs);

  // Memory Set with tag setting, non-temporal.
  void setgpn(const Register& rd, const Register& rn, const Register& rs);

  // Memory Set.
  void setm(const Register& rd, const Register& rn, const Register& rs);

  // Memory Set, non-temporal.
  void setmn(const Register& rd, const Register& rn, const Register& rs);

  // Memory Set.
  void setp(const Register& rd, const Register& rn, const Register& rs);

  // Memory Set, non-temporal.
  void setpn(const Register& rd, const Register& rn, const Register& rs);

  // Absolute value.
  void abs(const Register& rd, const Register& rn);

  // Count bits.
  void cnt(const Register& rd, const Register& rn);

  // Count Trailing Zeros.
  void ctz(const Register& rd, const Register& rn);

  // Signed Maximum.
  void smax(const Register& rd, const Register& rn, const Operand& op);

  // Signed Minimum.
  void smin(const Register& rd, const Register& rn, const Operand& op);

  // Unsigned Maximum.
  void umax(const Register& rd, const Register& rn, const Operand& op);

  // Unsigned Minimum.
  void umin(const Register& rd, const Register& rn, const Operand& op);

  // Check feature status.
  void chkfeat(const Register& rd);

  // Guarded Control Stack Push.
  void gcspushm(const Register& rt);

  // Guarded Control Stack Pop.
  void gcspopm(const Register& rt);

  // Guarded Control Stack Switch Stack 1.
  void gcsss1(const Register& rt);

  // Guarded Control Stack Switch Stack 2.
  void gcsss2(const Register& rt);

  // Emit generic instructions.

  // Emit raw instructions into the instruction stream.
  void dci(Instr raw_inst) { Emit(raw_inst); }

  // Emit 32 bits of data into the instruction stream.
  void dc32(uint32_t data) { dc(data); }

  // Emit 64 bits of data into the instruction stream.
  void dc64(uint64_t data) { dc(data); }

  // Emit data in the instruction stream.
  template <typename T>
  void dc(T data) {
    VIXL_ASSERT(AllowAssembler());
    GetBuffer()->Emit<T>(data);
  }

  // Copy a string into the instruction stream, including the terminating NULL
  // character. The instruction pointer is then aligned correctly for
  // subsequent instructions.
  void EmitString(const char* string) {
    VIXL_ASSERT(string != NULL);
    VIXL_ASSERT(AllowAssembler());

    GetBuffer()->EmitString(string);
    GetBuffer()->Align();
  }

  // Code generation helpers.
  static bool OneInstrMoveImmediateHelper(Assembler* assm,
                                          const Register& dst,
                                          uint64_t imm);

  // Register encoding.
  template <int hibit, int lobit>
  static Instr Rx(CPURegister rx) {
    VIXL_ASSERT(rx.GetCode() != kSPRegInternalCode);
    return ImmUnsignedField<hibit, lobit>(rx.GetCode());
  }

#define CPU_REGISTER_FIELD_NAMES(V) V(d) V(n) V(m) V(a) V(t) V(t2) V(s)
#define REGISTER_ENCODER(N)                                           \
  static Instr R##N(CPURegister r##N) {                               \
    return Rx<R##N##_offset + R##N##_width - 1, R##N##_offset>(r##N); \
  }
  CPU_REGISTER_FIELD_NAMES(REGISTER_ENCODER)
#undef REGISTER_ENCODER
#undef CPU_REGISTER_FIELD_NAMES

  static Instr RmNot31(CPURegister rm) {
    VIXL_ASSERT(rm.GetCode() != kSPRegInternalCode);
    VIXL_ASSERT(!rm.IsZero());
    return Rm(rm);
  }

  // These encoding functions allow the stack pointer to be encoded, and
  // disallow the zero register.
  static Instr RdSP(Register rd) {
    VIXL_ASSERT(!rd.IsZero());
    return (rd.GetCode() & kRegCodeMask) << Rd_offset;
  }

  static Instr RnSP(Register rn) {
    VIXL_ASSERT(!rn.IsZero());
    return (rn.GetCode() & kRegCodeMask) << Rn_offset;
  }

  static Instr RmSP(Register rm) {
    VIXL_ASSERT(!rm.IsZero());
    return (rm.GetCode() & kRegCodeMask) << Rm_offset;
  }

  static Instr Pd(PRegister pd) {
    return Rx<Pd_offset + Pd_width - 1, Pd_offset>(pd);
  }

  static Instr Pm(PRegister pm) {
    return Rx<Pm_offset + Pm_width - 1, Pm_offset>(pm);
  }

  static Instr Pn(PRegister pn) {
    return Rx<Pn_offset + Pn_width - 1, Pn_offset>(pn);
  }

  static Instr PgLow8(PRegister pg) {
    // Governing predicates can be merging, zeroing, or unqualified. They should
    // never have a lane size.
    VIXL_ASSERT(!pg.HasLaneSize());
    return Rx<PgLow8_offset + PgLow8_width - 1, PgLow8_offset>(pg);
  }

  template <int hibit, int lobit>
  static Instr Pg(PRegister pg) {
    // Governing predicates can be merging, zeroing, or unqualified. They should
    // never have a lane size.
    VIXL_ASSERT(!pg.HasLaneSize());
    return Rx<hibit, lobit>(pg);
  }

  // Flags encoding.
  static Instr Flags(FlagsUpdate S) {
    if (S == SetFlags) {
      return 1 << FlagsUpdate_offset;
    } else if (S == LeaveFlags) {
      return 0 << FlagsUpdate_offset;
    }
    VIXL_UNREACHABLE();
    return 0;
  }

  static Instr Cond(Condition cond) { return cond << Condition_offset; }

  // Generic immediate encoding.
  template <int hibit, int lobit>
  static Instr ImmField(int64_t imm) {
    VIXL_STATIC_ASSERT((hibit >= lobit) && (lobit >= 0));
    VIXL_STATIC_ASSERT(hibit < (sizeof(Instr) * kBitsPerByte));
    int fieldsize = hibit - lobit + 1;
    VIXL_ASSERT(IsIntN(fieldsize, imm));
    return static_cast<Instr>(TruncateToUintN(fieldsize, imm) << lobit);
  }

  // For unsigned immediate encoding.
  // TODO: Handle signed and unsigned immediate in satisfactory way.
  template <int hibit, int lobit>
  static Instr ImmUnsignedField(uint64_t imm) {
    VIXL_STATIC_ASSERT((hibit >= lobit) && (lobit >= 0));
    VIXL_STATIC_ASSERT(hibit < (sizeof(Instr) * kBitsPerByte));
    VIXL_ASSERT(IsUintN(hibit - lobit + 1, imm));
    return static_cast<Instr>(imm << lobit);
  }

  // PC-relative address encoding.
  static Instr ImmPCRelAddress(int64_t imm21) {
    VIXL_ASSERT(IsInt21(imm21));
    Instr imm = static_cast<Instr>(TruncateToUint21(imm21));
    Instr immhi = (imm >> ImmPCRelLo_width) << ImmPCRelHi_offset;
    Instr immlo = imm << ImmPCRelLo_offset;
    return (immhi & ImmPCRelHi_mask) | (immlo & ImmPCRelLo_mask);
  }

  // Branch encoding.
  static Instr ImmUncondBranch(int64_t imm26) {
    VIXL_ASSERT(IsInt26(imm26));
    return TruncateToUint26(imm26) << ImmUncondBranch_offset;
  }

  static Instr ImmCondBranch(int64_t imm19) {
    VIXL_ASSERT(IsInt19(imm19));
    return TruncateToUint19(imm19) << ImmCondBranch_offset;
  }

  static Instr ImmCmpBranch(int64_t imm19) {
    VIXL_ASSERT(IsInt19(imm19));
    return TruncateToUint19(imm19) << ImmCmpBranch_offset;
  }

  static Instr ImmTestBranch(int64_t imm14) {
    VIXL_ASSERT(IsInt14(imm14));
    return TruncateToUint14(imm14) << ImmTestBranch_offset;
  }

  static Instr ImmTestBranchBit(unsigned bit_pos) {
    VIXL_ASSERT(IsUint6(bit_pos));
    // Subtract five from the shift offset, as we need bit 5 from bit_pos.
    unsigned bit5 = bit_pos << (ImmTestBranchBit5_offset - 5);
    unsigned bit40 = bit_pos << ImmTestBranchBit40_offset;
    bit5 &= ImmTestBranchBit5_mask;
    bit40 &= ImmTestBranchBit40_mask;
    return bit5 | bit40;
  }

  // Data Processing encoding.
  static Instr SF(Register rd) {
    return rd.Is64Bits() ? SixtyFourBits : ThirtyTwoBits;
  }

  static Instr ImmAddSub(int imm) {
    VIXL_ASSERT(IsImmAddSub(imm));
    if (IsUint12(imm)) {  // No shift required.
      imm <<= ImmAddSub_offset;
    } else {
      imm = ((imm >> 12) << ImmAddSub_offset) | (1 << ImmAddSubShift_offset);
    }
    return imm;
  }

  static Instr SVEImmSetBits(unsigned imms, unsigned lane_size) {
    VIXL_ASSERT(IsUint6(imms));
    VIXL_ASSERT((lane_size == kDRegSize) || IsUint6(imms + 3));
    USE(lane_size);
    return imms << SVEImmSetBits_offset;
  }

  static Instr SVEImmRotate(unsigned immr, unsigned lane_size) {
    VIXL_ASSERT(IsUintN(WhichPowerOf2(lane_size), immr));
    USE(lane_size);
    return immr << SVEImmRotate_offset;
  }

  static Instr SVEBitN(unsigned bitn) {
    VIXL_ASSERT(IsUint1(bitn));
    return bitn << SVEBitN_offset;
  }

  static Instr SVEDtype(unsigned msize_in_bytes_log2,
                        unsigned esize_in_bytes_log2,
                        bool is_signed,
                        int dtype_h_lsb = 23,
                        int dtype_l_lsb = 21) {
    VIXL_ASSERT(msize_in_bytes_log2 <= kDRegSizeInBytesLog2);
    VIXL_ASSERT(esize_in_bytes_log2 <= kDRegSizeInBytesLog2);
    Instr dtype_h = msize_in_bytes_log2;
    Instr dtype_l = esize_in_bytes_log2;
    // Signed forms use the encodings where msize would be greater than esize.
    if (is_signed) {
      dtype_h = dtype_h ^ 0x3;
      dtype_l = dtype_l ^ 0x3;
    }
    VIXL_ASSERT(IsUint2(dtype_h));
    VIXL_ASSERT(IsUint2(dtype_l));
    VIXL_ASSERT((dtype_h > dtype_l) == is_signed);

    return (dtype_h << dtype_h_lsb) | (dtype_l << dtype_l_lsb);
  }

  static Instr SVEDtypeSplit(unsigned msize_in_bytes_log2,
                             unsigned esize_in_bytes_log2,
                             bool is_signed) {
    return SVEDtype(msize_in_bytes_log2,
                    esize_in_bytes_log2,
                    is_signed,
                    23,
                    13);
  }

  static Instr ImmS(unsigned imms, unsigned reg_size) {
    VIXL_ASSERT(((reg_size == kXRegSize) && IsUint6(imms)) ||
                ((reg_size == kWRegSize) && IsUint5(imms)));
    USE(reg_size);
    return imms << ImmS_offset;
  }

  static Instr ImmR(unsigned immr, unsigned reg_size) {
    VIXL_ASSERT(((reg_size == kXRegSize) && IsUint6(immr)) ||
                ((reg_size == kWRegSize) && IsUint5(immr)));
    USE(reg_size);
    VIXL_ASSERT(IsUint6(immr));
    return immr << ImmR_offset;
  }

  static Instr ImmSetBits(unsigned imms, unsigned reg_size) {
    VIXL_ASSERT((reg_size == kWRegSize) || (reg_size == kXRegSize));
    VIXL_ASSERT(IsUint6(imms));
    VIXL_ASSERT((reg_size == kXRegSize) || IsUint6(imms + 3));
    USE(reg_size);
    return imms << ImmSetBits_offset;
  }

  static Instr ImmRotate(unsigned immr, unsigned reg_size) {
    VIXL_ASSERT((reg_size == kWRegSize) || (reg_size == kXRegSize));
    VIXL_ASSERT(((reg_size == kXRegSize) && IsUint6(immr)) ||
                ((reg_size == kWRegSize) && IsUint5(immr)));
    USE(reg_size);
    return immr << ImmRotate_offset;
  }

  static Instr ImmLLiteral(int64_t imm19) {
    VIXL_ASSERT(IsInt19(imm19));
    return TruncateToUint19(imm19) << ImmLLiteral_offset;
  }

  static Instr BitN(unsigned bitn, unsigned reg_size) {
    VIXL_ASSERT((reg_size == kWRegSize) || (reg_size == kXRegSize));
    VIXL_ASSERT((reg_size == kXRegSize) || (bitn == 0));
    USE(reg_size);
    return bitn << BitN_offset;
  }

  static Instr ShiftDP(Shift shift) {
    VIXL_ASSERT(shift == LSL || shift == LSR || shift == ASR || shift == ROR);
    return shift << ShiftDP_offset;
  }

  static Instr ImmDPShift(unsigned amount) {
    VIXL_ASSERT(IsUint6(amount));
    return amount << ImmDPShift_offset;
  }

  static Instr ExtendMode(Extend extend) { return extend << ExtendMode_offset; }

  static Instr ImmExtendShift(unsigned left_shift) {
    VIXL_ASSERT(left_shift <= 4);
    return left_shift << ImmExtendShift_offset;
  }

  static Instr ImmCondCmp(unsigned imm) {
    VIXL_ASSERT(IsUint5(imm));
    return imm << ImmCondCmp_offset;
  }

  static Instr Nzcv(StatusFlags nzcv) {
    return ((nzcv >> Flags_offset) & 0xf) << Nzcv_offset;
  }

  // MemOperand offset encoding.
  static Instr ImmLSUnsigned(int64_t imm12) {
    VIXL_ASSERT(IsUint12(imm12));
    return TruncateToUint12(imm12) << ImmLSUnsigned_offset;
  }

  static Instr ImmLS(int64_t imm9) {
    VIXL_ASSERT(IsInt9(imm9));
    return TruncateToUint9(imm9) << ImmLS_offset;
  }

  static Instr ImmLSPair(int64_t imm7, unsigned access_size_in_bytes_log2) {
    const auto access_size_in_bytes = 1U << access_size_in_bytes_log2;
    VIXL_ASSERT(IsMultiple(imm7, access_size_in_bytes));
    int64_t scaled_imm7 = imm7 / access_size_in_bytes;
    VIXL_ASSERT(IsInt7(scaled_imm7));
    return TruncateToUint7(scaled_imm7) << ImmLSPair_offset;
  }

  static Instr ImmShiftLS(unsigned shift_amount) {
    VIXL_ASSERT(IsUint1(shift_amount));
    return shift_amount << ImmShiftLS_offset;
  }

  static Instr ImmLSPAC(int64_t imm10) {
    VIXL_ASSERT(IsMultiple(imm10, 1 << 3));
    int64_t scaled_imm10 = imm10 / (1 << 3);
    VIXL_ASSERT(IsInt10(scaled_imm10));
    uint32_t s_bit = (scaled_imm10 >> 9) & 1;
    return (s_bit << ImmLSPACHi_offset) |
           (TruncateToUint9(scaled_imm10) << ImmLSPACLo_offset);
  }

  static Instr ImmPrefetchOperation(int imm5) {
    VIXL_ASSERT(IsUint5(imm5));
    return imm5 << ImmPrefetchOperation_offset;
  }

  static Instr ImmException(int imm16) {
    VIXL_ASSERT(IsUint16(imm16));
    return imm16 << ImmException_offset;
  }

  static Instr ImmUdf(int imm16) {
    VIXL_ASSERT(IsUint16(imm16));
    return imm16 << ImmUdf_offset;
  }

  static Instr ImmSystemRegister(int imm16) {
    VIXL_ASSERT(IsUint16(imm16));
    return imm16 << ImmSystemRegister_offset;
  }

  static Instr ImmRMIFRotation(int imm6) {
    VIXL_ASSERT(IsUint6(imm6));
    return imm6 << ImmRMIFRotation_offset;
  }

  static Instr ImmHint(int imm7) {
    VIXL_ASSERT(IsUint7(imm7));
    return imm7 << ImmHint_offset;
  }

  static Instr CRm(int imm4) {
    VIXL_ASSERT(IsUint4(imm4));
    return imm4 << CRm_offset;
  }

  static Instr CRn(int imm4) {
    VIXL_ASSERT(IsUint4(imm4));
    return imm4 << CRn_offset;
  }

  static Instr SysOp(int imm14) {
    VIXL_ASSERT(IsUint14(imm14));
    return imm14 << SysOp_offset;
  }

  static Instr ImmSysOp1(int imm3) {
    VIXL_ASSERT(IsUint3(imm3));
    return imm3 << SysOp1_offset;
  }

  static Instr ImmSysOp2(int imm3) {
    VIXL_ASSERT(IsUint3(imm3));
    return imm3 << SysOp2_offset;
  }

  static Instr ImmBarrierDomain(int imm2) {
    VIXL_ASSERT(IsUint2(imm2));
    return imm2 << ImmBarrierDomain_offset;
  }

  static Instr ImmBarrierType(int imm2) {
    VIXL_ASSERT(IsUint2(imm2));
    return imm2 << ImmBarrierType_offset;
  }

  // Move immediates encoding.
  static Instr ImmMoveWide(uint64_t imm) {
    VIXL_ASSERT(IsUint16(imm));
    return static_cast<Instr>(imm << ImmMoveWide_offset);
  }

  static Instr ShiftMoveWide(int64_t shift) {
    VIXL_ASSERT(IsUint2(shift));
    return static_cast<Instr>(shift << ShiftMoveWide_offset);
  }

  // FP Immediates.
  static Instr ImmFP16(Float16 imm);
  static Instr ImmFP32(float imm);
  static Instr ImmFP64(double imm);

  // FP register type.
  static Instr FPType(VRegister fd) {
    VIXL_ASSERT(fd.IsScalar());
    switch (fd.GetSizeInBits()) {
      case 16:
        return FP16;
      case 32:
        return FP32;
      case 64:
        return FP64;
      default:
        VIXL_UNREACHABLE();
        return 0;
    }
  }

  static Instr FPScale(unsigned scale) {
    VIXL_ASSERT(IsUint6(scale));
    return scale << FPScale_offset;
  }

  // Immediate field checking helpers.
  static bool IsImmAddSub(int64_t immediate);
  static bool IsImmConditionalCompare(int64_t immediate);
  static bool IsImmFP16(Float16 imm);

  static bool IsImmFP32(float imm) { return IsImmFP32(FloatToRawbits(imm)); }

  static bool IsImmFP32(uint32_t bits);

  static bool IsImmFP64(double imm) { return IsImmFP64(DoubleToRawbits(imm)); }

  static bool IsImmFP64(uint64_t bits);
  static bool IsImmLogical(uint64_t value,
                           unsigned width,
                           unsigned* n = NULL,
                           unsigned* imm_s = NULL,
                           unsigned* imm_r = NULL);
  static bool IsImmLSPair(int64_t offset, unsigned access_size_in_bytes_log2);
  static bool IsImmLSScaled(int64_t offset, unsigned access_size_in_bytes_log2);
  static bool IsImmLSUnscaled(int64_t offset);
  static bool IsImmMovn(uint64_t imm, unsigned reg_size);
  static bool IsImmMovz(uint64_t imm, unsigned reg_size);

  // Instruction bits for vector format in data processing operations.
  static Instr VFormat(VRegister vd) {
    if (vd.Is64Bits()) {
      switch (vd.GetLanes()) {
        case 1:
          return NEON_1D;
        case 2:
          return NEON_2S;
        case 4:
          return NEON_4H;
        case 8:
          return NEON_8B;
        default:
          return 0xffffffff;
      }
    } else {
      VIXL_ASSERT(vd.Is128Bits());
      switch (vd.GetLanes()) {
        case 2:
          return NEON_2D;
        case 4:
          return NEON_4S;
        case 8:
          return NEON_8H;
        case 16:
          return NEON_16B;
        default:
          return 0xffffffff;
      }
    }
  }

  // Instruction bits for vector format in floating point data processing
  // operations.
  static Instr FPFormat(VRegister vd) {
    switch (vd.GetLanes()) {
      case 1:
        // Floating point scalar formats.
        switch (vd.GetSizeInBits()) {
          case 16:
            return FP16;
          case 32:
            return FP32;
          case 64:
            return FP64;
          default:
            VIXL_UNREACHABLE();
        }
        break;
      case 2:
        // Two lane floating point vector formats.
        switch (vd.GetSizeInBits()) {
          case 64:
            return NEON_FP_2S;
          case 128:
            return NEON_FP_2D;
          default:
            VIXL_UNREACHABLE();
        }
        break;
      case 4:
        // Four lane floating point vector formats.
        switch (vd.GetSizeInBits()) {
          case 64:
            return NEON_FP_4H;
          case 128:
            return NEON_FP_4S;
          default:
            VIXL_UNREACHABLE();
        }
        break;
      case 8:
        // Eight lane floating point vector format.
        VIXL_ASSERT(vd.Is128Bits());
        return NEON_FP_8H;
      default:
        VIXL_UNREACHABLE();
        return 0;
    }
    VIXL_UNREACHABLE();
    return 0;
  }

  // Instruction bits for vector format in load and store operations.
  static Instr LSVFormat(VRegister vd) {
    if (vd.Is64Bits()) {
      switch (vd.GetLanes()) {
        case 1:
          return LS_NEON_1D;
        case 2:
          return LS_NEON_2S;
        case 4:
          return LS_NEON_4H;
        case 8:
          return LS_NEON_8B;
        default:
          return 0xffffffff;
      }
    } else {
      VIXL_ASSERT(vd.Is128Bits());
      switch (vd.GetLanes()) {
        case 2:
          return LS_NEON_2D;
        case 4:
          return LS_NEON_4S;
        case 8:
          return LS_NEON_8H;
        case 16:
          return LS_NEON_16B;
        default:
          return 0xffffffff;
      }
    }
  }

  // Instruction bits for scalar format in data processing operations.
  static Instr SFormat(VRegister vd) {
    VIXL_ASSERT(vd.GetLanes() == 1);
    switch (vd.GetSizeInBytes()) {
      case 1:
        return NEON_B;
      case 2:
        return NEON_H;
      case 4:
        return NEON_S;
      case 8:
        return NEON_D;
      default:
        return 0xffffffff;
    }
  }

  template <typename T>
  static Instr SVESize(const T& rd) {
    VIXL_ASSERT(rd.IsZRegister() || rd.IsPRegister());
    VIXL_ASSERT(rd.HasLaneSize());
    switch (rd.GetLaneSizeInBytes()) {
      case 1:
        return SVE_B;
      case 2:
        return SVE_H;
      case 4:
        return SVE_S;
      case 8:
        return SVE_D;
      default:
        return 0xffffffff;
    }
  }

  static Instr ImmSVEPredicateConstraint(int pattern) {
    VIXL_ASSERT(IsUint5(pattern));
    return (pattern << ImmSVEPredicateConstraint_offset) &
           ImmSVEPredicateConstraint_mask;
  }

  static Instr ImmNEONHLM(int index, int num_bits) {
    int h, l, m;
    if (num_bits == 3) {
      VIXL_ASSERT(IsUint3(index));
      h = (index >> 2) & 1;
      l = (index >> 1) & 1;
      m = (index >> 0) & 1;
    } else if (num_bits == 2) {
      VIXL_ASSERT(IsUint2(index));
      h = (index >> 1) & 1;
      l = (index >> 0) & 1;
      m = 0;
    } else {
      VIXL_ASSERT(IsUint1(index) && (num_bits == 1));
      h = (index >> 0) & 1;
      l = 0;
      m = 0;
    }
    return (h << NEONH_offset) | (l << NEONL_offset) | (m << NEONM_offset);
  }

  static Instr ImmRotFcadd(int rot) {
    VIXL_ASSERT(rot == 90 || rot == 270);
    return (((rot == 270) ? 1 : 0) << ImmRotFcadd_offset);
  }

  static Instr ImmRotFcmlaSca(int rot) {
    VIXL_ASSERT(rot == 0 || rot == 90 || rot == 180 || rot == 270);
    return (rot / 90) << ImmRotFcmlaSca_offset;
  }

  static Instr ImmRotFcmlaVec(int rot) {
    VIXL_ASSERT(rot == 0 || rot == 90 || rot == 180 || rot == 270);
    return (rot / 90) << ImmRotFcmlaVec_offset;
  }

  static Instr ImmNEONExt(int imm4) {
    VIXL_ASSERT(IsUint4(imm4));
    return imm4 << ImmNEONExt_offset;
  }

  static Instr ImmNEON5(Instr format, int index) {
    VIXL_ASSERT(IsUint4(index));
    int s = LaneSizeInBytesLog2FromFormat(static_cast<VectorFormat>(format));
    int imm5 = (index << (s + 1)) | (1 << s);
    return imm5 << ImmNEON5_offset;
  }

  static Instr ImmNEON4(Instr format, int index) {
    VIXL_ASSERT(IsUint4(index));
    int s = LaneSizeInBytesLog2FromFormat(static_cast<VectorFormat>(format));
    int imm4 = index << s;
    return imm4 << ImmNEON4_offset;
  }

  static Instr ImmNEONabcdefgh(int imm8) {
    VIXL_ASSERT(IsUint8(imm8));
    Instr instr;
    instr = ((imm8 >> 5) & 7) << ImmNEONabc_offset;
    instr |= (imm8 & 0x1f) << ImmNEONdefgh_offset;
    return instr;
  }

  static Instr NEONCmode(int cmode) {
    VIXL_ASSERT(IsUint4(cmode));
    return cmode << NEONCmode_offset;
  }

  static Instr NEONModImmOp(int op) {
    VIXL_ASSERT(IsUint1(op));
    return op << NEONModImmOp_offset;
  }

  // Size of the code generated since label to the current position.
  size_t GetSizeOfCodeGeneratedSince(Label* label) const {
    VIXL_ASSERT(label->IsBound());
    return GetBuffer().GetOffsetFrom(label->GetLocation());
  }
  VIXL_DEPRECATED("GetSizeOfCodeGeneratedSince",
                  size_t SizeOfCodeGeneratedSince(Label* label) const) {
    return GetSizeOfCodeGeneratedSince(label);
  }

  VIXL_DEPRECATED("GetBuffer().GetCapacity()",
                  size_t GetBufferCapacity() const) {
    return GetBuffer().GetCapacity();
  }
  VIXL_DEPRECATED("GetBuffer().GetCapacity()", size_t BufferCapacity() const) {
    return GetBuffer().GetCapacity();
  }

  VIXL_DEPRECATED("GetBuffer().GetRemainingBytes()",
                  size_t GetRemainingBufferSpace() const) {
    return GetBuffer().GetRemainingBytes();
  }
  VIXL_DEPRECATED("GetBuffer().GetRemainingBytes()",
                  size_t RemainingBufferSpace() const) {
    return GetBuffer().GetRemainingBytes();
  }

  PositionIndependentCodeOption GetPic() const { return pic_; }
  VIXL_DEPRECATED("GetPic", PositionIndependentCodeOption pic() const) {
    return GetPic();
  }

  CPUFeatures* GetCPUFeatures() { return &cpu_features_; }

  void SetCPUFeatures(const CPUFeatures& cpu_features) {
    cpu_features_ = cpu_features;
  }

  bool AllowPageOffsetDependentCode() const {
    return (GetPic() == PageOffsetDependentCode) ||
           (GetPic() == PositionDependentCode);
  }

  static Register AppropriateZeroRegFor(const CPURegister& reg) {
    return reg.Is64Bits() ? Register(xzr) : Register(wzr);
  }

 protected:
  void LoadStore(const CPURegister& rt,
                 const MemOperand& addr,
                 LoadStoreOp op,
                 LoadStoreScalingOption option = PreferScaledOffset);

  void LoadStorePAC(const Register& xt,
                    const MemOperand& addr,
                    LoadStorePACOp op);

  void LoadStorePair(const CPURegister& rt,
                     const CPURegister& rt2,
                     const MemOperand& addr,
                     LoadStorePairOp op);
  void LoadStoreStruct(const VRegister& vt,
                       const MemOperand& addr,
                       NEONLoadStoreMultiStructOp op);
  void LoadStoreStruct1(const VRegister& vt,
                        int reg_count,
                        const MemOperand& addr);
  void LoadStoreStructSingle(const VRegister& vt,
                             uint32_t lane,
                             const MemOperand& addr,
                             NEONLoadStoreSingleStructOp op);
  void LoadStoreStructSingleAllLanes(const VRegister& vt,
                                     const MemOperand& addr,
                                     NEONLoadStoreSingleStructOp op);
  void LoadStoreStructVerify(const VRegister& vt,
                             const MemOperand& addr,
                             Instr op);

  // Set `is_load` to false in default as it's only used in the
  // scalar-plus-vector form.
  Instr SVEMemOperandHelper(unsigned msize_in_bytes_log2,
                            int num_regs,
                            const SVEMemOperand& addr,
                            bool is_load = false);

  // E.g. st1b, st1h, ...
  // This supports both contiguous and scatter stores.
  void SVESt1Helper(unsigned msize_in_bytes_log2,
                    const ZRegister& zt,
                    const PRegister& pg,
                    const SVEMemOperand& addr);

  // E.g. ld1b, ld1h, ...
  // This supports both contiguous and gather loads.
  void SVELd1Helper(unsigned msize_in_bytes_log2,
                    const ZRegister& zt,
                    const PRegisterZ& pg,
                    const SVEMemOperand& addr,
                    bool is_signed);

  // E.g. ld1rb, ld1rh, ...
  void SVELd1BroadcastHelper(unsigned msize_in_bytes_log2,
                             const ZRegister& zt,
                             const PRegisterZ& pg,
                             const SVEMemOperand& addr,
                             bool is_signed);

  // E.g. ldff1b, ldff1h, ...
  // This supports both contiguous and gather loads.
  void SVELdff1Helper(unsigned msize_in_bytes_log2,
                      const ZRegister& zt,
                      const PRegisterZ& pg,
                      const SVEMemOperand& addr,
                      bool is_signed);

  // Common code for the helpers above.
  void SVELdSt1Helper(unsigned msize_in_bytes_log2,
                      const ZRegister& zt,
                      const PRegister& pg,
                      const SVEMemOperand& addr,
                      bool is_signed,
                      Instr op);

  // Common code for the helpers above.
  void SVEScatterGatherHelper(unsigned msize_in_bytes_log2,
                              const ZRegister& zt,
                              const PRegister& pg,
                              const SVEMemOperand& addr,
                              bool is_load,
                              bool is_signed,
                              bool is_first_fault);

  // E.g. st2b, st3h, ...
  void SVESt234Helper(int num_regs,
                      const ZRegister& zt1,
                      const PRegister& pg,
                      const SVEMemOperand& addr);

  // E.g. ld2b, ld3h, ...
  void SVELd234Helper(int num_regs,
                      const ZRegister& zt1,
                      const PRegisterZ& pg,
                      const SVEMemOperand& addr);

  // Common code for the helpers above.
  void SVELdSt234Helper(int num_regs,
                        const ZRegister& zt1,
                        const PRegister& pg,
                        const SVEMemOperand& addr,
                        Instr op);

  // E.g. ld1qb, ld1qh, ldnt1b, ...
  void SVELd1St1ScaImmHelper(const ZRegister& zt,
                             const PRegister& pg,
                             const SVEMemOperand& addr,
                             Instr regoffset_op,
                             Instr immoffset_op,
                             int imm_divisor = 1);

  void SVELd1VecScaHelper(const ZRegister& zt,
                          const PRegister& pg,
                          const SVEMemOperand& addr,
                          uint32_t msize,
                          bool is_signed);
  void SVESt1VecScaHelper(const ZRegister& zt,
                          const PRegister& pg,
                          const SVEMemOperand& addr,
                          uint32_t msize);

  void Prefetch(PrefetchOperation op,
                const MemOperand& addr,
                LoadStoreScalingOption option = PreferScaledOffset);
  void Prefetch(int op,
                const MemOperand& addr,
                LoadStoreScalingOption option = PreferScaledOffset);

  // TODO(all): The third parameter should be passed by reference but gcc 4.8.2
  // reports a bogus uninitialised warning then.
  void Logical(const Register& rd,
               const Register& rn,
               const Operand operand,
               LogicalOp op);

  void SVELogicalImmediate(const ZRegister& zd, uint64_t imm, Instr op);

  void LogicalImmediate(const Register& rd,
                        const Register& rn,
                        unsigned n,
                        unsigned imm_s,
                        unsigned imm_r,
                        LogicalOp op);

  void ConditionalCompare(const Register& rn,
                          const Operand& operand,
                          StatusFlags nzcv,
                          Condition cond,
                          ConditionalCompareOp op);

  void AddSubWithCarry(const Register& rd,
                       const Register& rn,
                       const Operand& operand,
                       FlagsUpdate S,
                       AddSubWithCarryOp op);

  void CompareVectors(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      const ZRegister& zm,
                      SVEIntCompareVectorsOp op);

  void CompareVectors(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      int imm,
                      SVEIntCompareSignedImmOp op);

  void CompareVectors(const PRegisterWithLaneSize& pd,
                      const PRegisterZ& pg,
                      const ZRegister& zn,
                      unsigned imm,
                      SVEIntCompareUnsignedImmOp op);

  void SVEIntAddSubtractImmUnpredicatedHelper(
      SVEIntAddSubtractImm_UnpredicatedOp op,
      const ZRegister& zd,
      int imm8,
      int shift);

  void SVEElementCountToRegisterHelper(Instr op,
                                       const Register& rd,
                                       int pattern,
                                       int multiplier);

  Instr EncodeSVEShiftLeftImmediate(int shift, int lane_size_in_bits);

  Instr EncodeSVEShiftRightImmediate(int shift, int lane_size_in_bits);

  void SVEBitwiseShiftImmediate(const ZRegister& zd,
                                const ZRegister& zn,
                                Instr encoded_imm,
                                Instr op);

  void SVEBitwiseShiftImmediatePred(const ZRegister& zdn,
                                    const PRegisterM& pg,
                                    Instr encoded_imm,
                                    Instr op);

  Instr SVEMulIndexHelper(unsigned lane_size_in_bytes_log2,
                          const ZRegister& zm,
                          int index,
                          Instr op_h,
                          Instr op_s,
                          Instr op_d);

  Instr SVEMulLongIndexHelper(const ZRegister& zm, int index);

  Instr SVEMulComplexIndexHelper(const ZRegister& zm, int index);

  void SVEContiguousPrefetchScalarPlusScalarHelper(PrefetchOperation prfop,
                                                   const PRegister& pg,
                                                   const SVEMemOperand& addr,
                                                   int prefetch_size);

  void SVEContiguousPrefetchScalarPlusVectorHelper(PrefetchOperation prfop,
                                                   const PRegister& pg,
                                                   const SVEMemOperand& addr,
                                                   int prefetch_size);

  void SVEGatherPrefetchVectorPlusImmediateHelper(PrefetchOperation prfop,
                                                  const PRegister& pg,
                                                  const SVEMemOperand& addr,
                                                  int prefetch_size);

  void SVEGatherPrefetchScalarPlusImmediateHelper(PrefetchOperation prfop,
                                                  const PRegister& pg,
                                                  const SVEMemOperand& addr,
                                                  int prefetch_size);

  void SVEPrefetchHelper(PrefetchOperation prfop,
                         const PRegister& pg,
                         const SVEMemOperand& addr,
                         int prefetch_size);

  static Instr SVEImmPrefetchOperation(PrefetchOperation prfop) {
    // SVE only supports PLD and PST, not PLI.
    VIXL_ASSERT(((prfop >= PLDL1KEEP) && (prfop <= PLDL3STRM)) ||
                ((prfop >= PSTL1KEEP) && (prfop <= PSTL3STRM)));
    // Check that we can simply map bits.
    VIXL_STATIC_ASSERT(PLDL1KEEP == 0b00000);
    VIXL_STATIC_ASSERT(PSTL1KEEP == 0b10000);
    // Remaining operations map directly.
    return ((prfop & 0b10000) >> 1) | (prfop & 0b00111);
  }

  // Functions for emulating operands not directly supported by the instruction
  // set.
  void EmitShift(const Register& rd,
                 const Register& rn,
                 Shift shift,
                 unsigned amount);
  void EmitExtendShift(const Register& rd,
                       const Register& rn,
                       Extend extend,
                       unsigned left_shift);

  void AddSub(const Register& rd,
              const Register& rn,
              const Operand& operand,
              FlagsUpdate S,
              AddSubOp op);

  void NEONTable(const VRegister& vd,
                 const VRegister& vn,
                 const VRegister& vm,
                 NEONTableOp op);

  // Find an appropriate LoadStoreOp or LoadStorePairOp for the specified
  // registers. Only simple loads are supported; sign- and zero-extension (such
  // as in LDPSW_x or LDRB_w) are not supported.
  static LoadStoreOp LoadOpFor(const CPURegister& rt);
  static LoadStorePairOp LoadPairOpFor(const CPURegister& rt,
                                       const CPURegister& rt2);
  static LoadStoreOp StoreOpFor(const CPURegister& rt);
  static LoadStorePairOp StorePairOpFor(const CPURegister& rt,
                                        const CPURegister& rt2);
  static LoadStorePairNonTemporalOp LoadPairNonTemporalOpFor(
      const CPURegister& rt, const CPURegister& rt2);
  static LoadStorePairNonTemporalOp StorePairNonTemporalOpFor(
      const CPURegister& rt, const CPURegister& rt2);
  static LoadLiteralOp LoadLiteralOpFor(const CPURegister& rt);

  // Convenience pass-through for CPU feature checks.
  bool CPUHas(CPUFeatures::Feature feature0,
              CPUFeatures::Feature feature1 = CPUFeatures::kNone,
              CPUFeatures::Feature feature2 = CPUFeatures::kNone,
              CPUFeatures::Feature feature3 = CPUFeatures::kNone) const {
    return cpu_features_.Has(feature0, feature1, feature2, feature3);
  }

  // Determine whether the target CPU has the specified registers, based on the
  // currently-enabled CPU features. Presence of a register does not imply
  // support for arbitrary operations on it. For example, CPUs with FP have H
  // registers, but most half-precision operations require the FPHalf feature.
  //
  // These are used to check CPU features in loads and stores that have the same
  // entry point for both integer and FP registers.
  bool CPUHas(const CPURegister& rt) const;
  bool CPUHas(const CPURegister& rt, const CPURegister& rt2) const;

  bool CPUHas(SystemRegister sysreg) const;

 private:
  static uint32_t FP16ToImm8(Float16 imm);
  static uint32_t FP32ToImm8(float imm);
  static uint32_t FP64ToImm8(double imm);

  // Instruction helpers.
  void MoveWide(const Register& rd,
                uint64_t imm,
                int shift,
                MoveWideImmediateOp mov_op);
  void DataProcShiftedRegister(const Register& rd,
                               const Register& rn,
                               const Operand& operand,
                               FlagsUpdate S,
                               Instr op);
  void DataProcExtendedRegister(const Register& rd,
                                const Register& rn,
                                const Operand& operand,
                                FlagsUpdate S,
                                Instr op);
  void LoadStorePairNonTemporal(const CPURegister& rt,
                                const CPURegister& rt2,
                                const MemOperand& addr,
                                LoadStorePairNonTemporalOp op);
  void LoadLiteral(const CPURegister& rt, uint64_t imm, LoadLiteralOp op);
  void ConditionalSelect(const Register& rd,
                         const Register& rn,
                         const Register& rm,
                         Condition cond,
                         ConditionalSelectOp op);
  void DataProcessing1Source(const Register& rd,
                             const Register& rn,
                             DataProcessing1SourceOp op);
  void DataProcessing3Source(const Register& rd,
                             const Register& rn,
                             const Register& rm,
                             const Register& ra,
                             DataProcessing3SourceOp op);
  void FPDataProcessing1Source(const VRegister& fd,
                               const VRegister& fn,
                               FPDataProcessing1SourceOp op);
  void FPDataProcessing3Source(const VRegister& fd,
                               const VRegister& fn,
                               const VRegister& fm,
                               const VRegister& fa,
                               FPDataProcessing3SourceOp op);
  void NEONAcrossLanesL(const VRegister& vd,
                        const VRegister& vn,
                        NEONAcrossLanesOp op);
  void NEONAcrossLanes(const VRegister& vd,
                       const VRegister& vn,
                       NEONAcrossLanesOp op,
                       Instr op_half);
  void NEONModifiedImmShiftLsl(const VRegister& vd,
                               const int imm8,
                               const int left_shift,
                               NEONModifiedImmediateOp op);
  void NEONModifiedImmShiftMsl(const VRegister& vd,
                               const int imm8,
                               const int shift_amount,
                               NEONModifiedImmediateOp op);
  void NEONFP2Same(const VRegister& vd, const VRegister& vn, Instr vop);
  void NEON3Same(const VRegister& vd,
                 const VRegister& vn,
                 const VRegister& vm,
                 NEON3SameOp vop);
  void NEON3SameFP16(const VRegister& vd,
                     const VRegister& vn,
                     const VRegister& vm,
                     Instr op);
  void NEONFP3Same(const VRegister& vd,
                   const VRegister& vn,
                   const VRegister& vm,
                   Instr op);
  void NEON3DifferentL(const VRegister& vd,
                       const VRegister& vn,
                       const VRegister& vm,
                       NEON3DifferentOp vop);
  void NEON3DifferentW(const VRegister& vd,
                       const VRegister& vn,
                       const VRegister& vm,
                       NEON3DifferentOp vop);
  void NEON3DifferentHN(const VRegister& vd,
                        const VRegister& vn,
                        const VRegister& vm,
                        NEON3DifferentOp vop);
  void NEONFP2RegMisc(const VRegister& vd,
                      const VRegister& vn,
                      NEON2RegMiscOp vop,
                      double value = 0.0);
  void NEONFP2RegMiscFP16(const VRegister& vd,
                          const VRegister& vn,
                          NEON2RegMiscFP16Op vop,
                          double value = 0.0);
  void NEON2RegMisc(const VRegister& vd,
                    const VRegister& vn,
                    NEON2RegMiscOp vop,
                    int value = 0);
  void NEONFP2RegMisc(const VRegister& vd, const VRegister& vn, Instr op);
  void NEONFP2RegMiscFP16(const VRegister& vd, const VRegister& vn, Instr op);
  void NEONAddlp(const VRegister& vd, const VRegister& vn, NEON2RegMiscOp op);
  void NEONPerm(const VRegister& vd,
                const VRegister& vn,
                const VRegister& vm,
                NEONPermOp op);
  void NEONFPByElement(const VRegister& vd,
                       const VRegister& vn,
                       const VRegister& vm,
                       int vm_index,
                       NEONByIndexedElementOp op,
                       NEONByIndexedElementOp op_half);
  void NEONByElement(const VRegister& vd,
                     const VRegister& vn,
                     const VRegister& vm,
                     int vm_index,
                     NEONByIndexedElementOp op);
  void NEONByElementL(const VRegister& vd,
                      const VRegister& vn,
                      const VRegister& vm,
                      int vm_index,
                      NEONByIndexedElementOp op);
  void NEONShiftImmediate(const VRegister& vd,
                          const VRegister& vn,
                          NEONShiftImmediateOp op,
                          int immh_immb);
  void NEONShiftLeftImmediate(const VRegister& vd,
                              const VRegister& vn,
                              int shift,
                              NEONShiftImmediateOp op);
  void NEONShiftRightImmediate(const VRegister& vd,
                               const VRegister& vn,
                               int shift,
                               NEONShiftImmediateOp op);
  void NEONShiftImmediateL(const VRegister& vd,
                           const VRegister& vn,
                           int shift,
                           NEONShiftImmediateOp op);
  void NEONShiftImmediateN(const VRegister& vd,
                           const VRegister& vn,
                           int shift,
                           NEONShiftImmediateOp op);
  void NEONXtn(const VRegister& vd, const VRegister& vn, NEON2RegMiscOp vop);

  // If *shift is -1, find values of *imm8 and *shift such that IsInt8(*imm8)
  // and *shift is either 0 or 8. Otherwise, leave the values unchanged.
  void ResolveSVEImm8Shift(int* imm8, int* shift);

  Instr LoadStoreStructAddrModeField(const MemOperand& addr);

  // Encode the specified MemOperand for the specified access size and scaling
  // preference.
  Instr LoadStoreMemOperand(const MemOperand& addr,
                            unsigned access_size_in_bytes_log2,
                            LoadStoreScalingOption option);

  // Link the current (not-yet-emitted) instruction to the specified label, then
  // return an offset to be encoded in the instruction. If the label is not yet
  // bound, an offset of 0 is returned.
  ptrdiff_t LinkAndGetByteOffsetTo(Label* label);
  ptrdiff_t LinkAndGetInstructionOffsetTo(Label* label);
  ptrdiff_t LinkAndGetPageOffsetTo(Label* label);

  // A common implementation for the LinkAndGet<Type>OffsetTo helpers.
  template <int element_shift>
  ptrdiff_t LinkAndGetOffsetTo(Label* label);

  // Literal load offset are in words (32-bit).
  ptrdiff_t LinkAndGetWordOffsetTo(RawLiteral* literal);

  // Emit the instruction in buffer_.
  void Emit(Instr instruction) {
    VIXL_STATIC_ASSERT(sizeof(instruction) == kInstructionSize);
    VIXL_ASSERT(AllowAssembler());
    GetBuffer()->Emit32(instruction);
  }

  PositionIndependentCodeOption pic_;

  CPUFeatures cpu_features_;
};


template <typename T>
void Literal<T>::UpdateValue(T new_value, const Assembler* assembler) {
  return UpdateValue(new_value,
                     assembler->GetBuffer().GetStartAddress<uint8_t*>());
}


template <typename T>
void Literal<T>::UpdateValue(T high64, T low64, const Assembler* assembler) {
  return UpdateValue(high64,
                     low64,
                     assembler->GetBuffer().GetStartAddress<uint8_t*>());
}


}  // namespace aarch64

// Required InvalSet template specialisations.
// TODO: These template specialisations should not live in this file.  Move
// Label out of the aarch64 namespace in order to share its implementation
// later.
#define INVAL_SET_TEMPLATE_PARAMETERS                                \
  ptrdiff_t, aarch64::Label::kNPreallocatedLinks, ptrdiff_t,         \
      aarch64::Label::kInvalidLinkKey, aarch64::Label::kReclaimFrom, \
      aarch64::Label::kReclaimFactor
template <>
inline ptrdiff_t InvalSet<INVAL_SET_TEMPLATE_PARAMETERS>::GetKey(
    const ptrdiff_t& element) {
  return element;
}
template <>
inline void InvalSet<INVAL_SET_TEMPLATE_PARAMETERS>::SetKey(ptrdiff_t* element,
                                                            ptrdiff_t key) {
  *element = key;
}
#undef INVAL_SET_TEMPLATE_PARAMETERS

}  // namespace vixl

#endif  // VIXL_AARCH64_ASSEMBLER_AARCH64_H_
