// Copyright 2013, ARM Limited
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

#ifndef VIXL_A64_ASSEMBLER_A64_H_
#define VIXL_A64_ASSEMBLER_A64_H_

#include <list>

#include "hphp/util/data-block.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/vixl/globals.h"
#include "hphp/vixl/utils.h"
#include "hphp/vixl/a64/instructions-a64.h"

namespace vixl {

typedef uint64_t RegList;
constexpr int kRegListSizeInBits = sizeof(RegList) * 8;

// Registers.

// Some CPURegister methods can return Register and FPRegister types, so we
// need to declare them in advance.
class Register;
class FPRegister;
class MemOperand;


class CPURegister {
 public:
  enum RegisterType {
    // The kInvalid value is used to detect uninitialized static instances,
    // which are always zero-initialized before any constructors are called.
    kInvalid = 0,
    kRegister,
    kFPRegister,
  };

  constexpr CPURegister() : code_(0), size_(0), type_(kInvalid) {}

  constexpr CPURegister(unsigned code, unsigned size, RegisterType type)
      : code_(code), size_(size), type_(type) {}

  constexpr unsigned code() const {
    return code_;
  }

  constexpr RegisterType type() const {
    return type_;
  }

  RegList Bit() const {
    assert(code_ < (sizeof(RegList) * 8));
    return IsValid() ? (static_cast<RegList>(1) << code_) : 0;
  }

  constexpr unsigned size() const {
    return size_;
  }

  int SizeInBytes() const {
    assert(IsValid());
    assert(size() % 8 == 0);
    return size_ / 8;
  }

  int SizeInBits() const {
    assert(IsValid());
    return size_;
  }

  bool Is32Bits() const {
    assert(IsValid());
    return size_ == 32;
  }

  bool Is64Bits() const {
    assert(IsValid());
    return size_ == 64;
  }

  bool IsValid() const {
    return IsValidRegister() || IsValidFPRegister();
  }

  bool IsValidRegister() const {
    return IsRegister() &&
           ((size_ == kWRegSize) || (size_ == kXRegSize)) &&
           ((code_ < kNumberOfRegisters) || (code_ == kSPRegInternalCode));
  }

  bool IsValidFPRegister() const {
    return IsFPRegister() &&
           ((size_ == kSRegSize) || (size_ == kDRegSize)) &&
           (code_ < kNumberOfFPRegisters);
  }

  bool Is(const CPURegister& other) const {
    return (code_ == other.code_) && (size_ == other.size_) &&
           (type_ == other.type_);
  }

  inline bool IsZero() const {
    assert(IsValid());
    return IsRegister() && (code_ == kZeroRegCode);
  }

  inline bool IsSP() const {
    assert(IsValid());
    return IsRegister() && (code_ == kSPRegInternalCode);
  }

  inline bool IsRegister() const {
    return type_ == kRegister;
  }

  inline bool IsFPRegister() const {
    return type_ == kFPRegister;
  }

  const Register& W() const;
  const Register& X() const;
  const FPRegister& S() const;
  const FPRegister& D() const;

  inline bool IsSameSizeAndType(const CPURegister& other) const {
    return (size_ == other.size_) && (type_ == other.type_);
  }

 protected:
  unsigned code_;
  unsigned size_;
  RegisterType type_;
};


class Register : public CPURegister {
 public:
  constexpr explicit Register() : CPURegister() {}
  constexpr explicit Register(const CPURegister& other)
      : CPURegister(other.code(), other.size(), other.type()) {
  }
  constexpr explicit Register(unsigned code, unsigned size)
      : CPURegister(code, size, kRegister) {}

  bool IsValid() const {
    return CPURegister::IsValid() && IsRegister();
  }

  MemOperand operator[](const ptrdiff_t offset) const;
  MemOperand operator[](const Register& offset) const;

  static const Register& WRegFromCode(unsigned code);
  static const Register& XRegFromCode(unsigned code);

  // V8 compatibility.
  static const int kNumRegisters = kNumberOfRegisters;
  static const int kNumAllocatableRegisters = kNumberOfRegisters - 1;

 private:
  static const Register wregisters[];
  static const Register xregisters[];
};


class FPRegister : public CPURegister {
 public:
  constexpr FPRegister() : CPURegister() {}
  constexpr explicit FPRegister(const CPURegister& other)
      : CPURegister(other.code(), other.size(), other.type()) {
  }
  constexpr FPRegister(unsigned code, unsigned size)
      : CPURegister(code, size, kFPRegister) {}

  bool IsValid() const {
    return CPURegister::IsValid()  && IsFPRegister();
  }

  static const FPRegister& SRegFromCode(unsigned code);
  static const FPRegister& DRegFromCode(unsigned code);

  // V8 compatibility.
  static const int kNumRegisters = kNumberOfFPRegisters;
  static const int kNumAllocatableRegisters = kNumberOfFPRegisters - 1;

 private:
  static const FPRegister sregisters[];
  static const FPRegister dregisters[];
};


// No*Reg is used to indicate an unused argument, or an error case. Note that
// these all compare equal (using the Is() method). The Register and FPRegister
// variants are provided for convenience.
const Register NoReg;
const FPRegister NoFPReg;
const CPURegister NoCPUReg;


#define DEFINE_REGISTERS(N)  \
const Register w##N(N, kWRegSize);  \
const Register x##N(N, kXRegSize);
REGISTER_CODE_LIST(DEFINE_REGISTERS)
#undef DEFINE_REGISTERS
const Register wsp(kSPRegInternalCode, kWRegSize);
const Register sp(kSPRegInternalCode, kXRegSize);


#define DEFINE_FPREGISTERS(N)  \
const FPRegister s##N(N, kSRegSize);  \
const FPRegister d##N(N, kDRegSize);
REGISTER_CODE_LIST(DEFINE_FPREGISTERS)
#undef DEFINE_FPREGISTERS


// Registers aliases.
const Register ip0 = x16;
const Register ip1 = x17;
const Register lr = x30;
const Register xzr = x31;
const Register wzr = w31;


// AreAliased returns true if any of the named registers overlap. Arguments
// set to NoReg are ignored. The system stack pointer may be specified.
bool AreAliased(const CPURegister& reg1,
                const CPURegister& reg2,
                const CPURegister& reg3 = NoReg,
                const CPURegister& reg4 = NoReg,
                const CPURegister& reg5 = NoReg,
                const CPURegister& reg6 = NoReg,
                const CPURegister& reg7 = NoReg,
                const CPURegister& reg8 = NoReg);


// AreSameSizeAndType returns true if all of the specified registers have the
// same size, and are of the same type. The system stack pointer may be
// specified. Arguments set to NoReg are ignored, as are any subsequent
// arguments. At least one argument (reg1) must be valid (not NoCPUReg).
bool AreSameSizeAndType(const CPURegister& reg1,
                        const CPURegister& reg2,
                        const CPURegister& reg3 = NoCPUReg,
                        const CPURegister& reg4 = NoCPUReg,
                        const CPURegister& reg5 = NoCPUReg,
                        const CPURegister& reg6 = NoCPUReg,
                        const CPURegister& reg7 = NoCPUReg,
                        const CPURegister& reg8 = NoCPUReg);


// Lists of registers.
class CPURegList {
 public:
  inline explicit CPURegList(CPURegister reg1,
                             CPURegister reg2 = NoCPUReg,
                             CPURegister reg3 = NoCPUReg,
                             CPURegister reg4 = NoCPUReg)
      : list_(reg1.Bit() | reg2.Bit() | reg3.Bit() | reg4.Bit()),
        size_(reg1.size()), type_(reg1.type()) {
    assert(AreSameSizeAndType(reg1, reg2, reg3, reg4));
    assert(IsValid());
  }

  inline CPURegList(CPURegister::RegisterType type, unsigned size, RegList list)
      : list_(list), size_(size), type_(type) {
    assert(IsValid());
  }

  inline CPURegList(CPURegister::RegisterType type, unsigned size,
                    unsigned first_reg, unsigned last_reg)
      : size_(size), type_(type) {
    assert(((type == CPURegister::kRegister) &&
            (last_reg < kNumberOfRegisters)) ||
           ((type == CPURegister::kFPRegister) &&
            (last_reg < kNumberOfFPRegisters)));
    assert(last_reg >= first_reg);
    list_ = (1UL << (last_reg + 1)) - 1;
    list_ &= ~((1UL << first_reg) - 1);
    assert(IsValid());
  }

  inline CPURegister::RegisterType type() const {
    assert(IsValid());
    return type_;
  }

  // Combine another CPURegList into this one. Registers that already exist in
  // this list are left unchanged. The type and size of the registers in the
  // 'other' list must match those in this list.
  void Combine(const CPURegList& other) {
    assert(IsValid());
    assert(other.type() == type_);
    assert(other.RegisterSizeInBits() == size_);
    list_ |= other.list();
  }

  // Remove every register in the other CPURegList from this one. Registers that
  // do not exist in this list are ignored. The type and size of the registers
  // in the 'other' list must match those in this list.
  void Remove(const CPURegList& other) {
    assert(IsValid());
    assert(other.type() == type_);
    assert(other.RegisterSizeInBits() == size_);
    list_ &= ~other.list();
  }

  // Variants of Combine and Remove which take a single register.
  inline void Combine(const CPURegister& other) {
    assert(other.type() == type_);
    assert(other.size() == size_);
    Combine(other.code());
  }

  inline void Remove(const CPURegister& other) {
    assert(other.type() == type_);
    assert(other.size() == size_);
    Remove(other.code());
  }

  // Variants of Combine and Remove which take a single register by its code;
  // the type and size of the register is inferred from this list.
  inline void Combine(int code) {
    assert(IsValid());
    assert(CPURegister(code, size_, type_).IsValid());
    list_ |= (1UL << code);
  }

  inline void Remove(int code) {
    assert(IsValid());
    assert(CPURegister(code, size_, type_).IsValid());
    list_ &= ~(1UL << code);
  }

  inline RegList list() const {
    assert(IsValid());
    return list_;
  }

  // Remove all callee-saved registers from the list. This can be useful when
  // preparing registers for an AAPCS64 function call, for example.
  void RemoveCalleeSaved();

  CPURegister PopLowestIndex();
  CPURegister PopHighestIndex();

  // AAPCS64 callee-saved registers.
  static CPURegList GetCalleeSaved(unsigned size = kXRegSize);
  static CPURegList GetCalleeSavedFP(unsigned size = kDRegSize);

  // AAPCS64 caller-saved registers. Note that this includes lr.
  static CPURegList GetCallerSaved(unsigned size = kXRegSize);
  static CPURegList GetCallerSavedFP(unsigned size = kDRegSize);

  inline bool IsEmpty() const {
    assert(IsValid());
    return list_ == 0;
  }

  inline bool IncludesAliasOf(const CPURegister& other) const {
    assert(IsValid());
    return (type_ == other.type()) && (other.Bit() & list_);
  }

  inline int Count() const {
    assert(IsValid());
    return CountSetBits(list_, kRegListSizeInBits);
  }

  inline unsigned RegisterSizeInBits() const {
    assert(IsValid());
    return size_;
  }

  inline unsigned RegisterSizeInBytes() const {
    int size_in_bits = RegisterSizeInBits();
    assert((size_in_bits % 8) == 0);
    return size_in_bits / 8;
  }

 private:
  RegList list_;
  unsigned size_;
  CPURegister::RegisterType type_;

  bool IsValid() const;
};


// AAPCS64 callee-saved registers.
extern const CPURegList kCalleeSaved;
extern const CPURegList kCalleeSavedFP;


// AAPCS64 caller-saved registers. Note that this includes lr.
extern const CPURegList kCallerSaved;
extern const CPURegList kCallerSavedFP;


// Operand.
class Operand {
 public:
  // #<immediate>
  // where <immediate> is int64_t.
  // This is allowed to be an implicit constructor because Operand is
  // a wrapper class that doesn't normally perform any type conversion.
  /* implicit */ Operand(int64_t immediate);

  // rm, {<shift> #<shift_amount>}
  // where <shift> is one of {LSL, LSR, ASR, ROR}.
  //       <shift_amount> is uint6_t.
  // This is allowed to be an implicit constructor because Operand is
  // a wrapper class that doesn't normally perform any type conversion.
  /* implicit */ Operand(Register reg,
                         Shift shift = LSL,
                         unsigned shift_amount = 0);

  // rm, {<extend> {#<shift_amount>}}
  // where <extend> is one of {UXTB, UXTH, UXTW, UXTX, SXTB, SXTH, SXTW, SXTX}.
  //       <shift_amount> is uint2_t.
  explicit Operand(Register reg, Extend extend, unsigned shift_amount = 0);

  bool IsImmediate() const;
  bool IsShiftedRegister() const;
  bool IsExtendedRegister() const;

  // This returns an LSL shift (<= 4) operand as an equivalent extend operand,
  // which helps in the encoding of instructions that use the stack pointer.
  Operand ToExtendedRegister() const;

  int64_t immediate() const {
    assert(IsImmediate());
    return immediate_;
  }

  Register reg() const {
    assert(IsShiftedRegister() || IsExtendedRegister());
    return reg_;
  }

  Shift shift() const {
    assert(IsShiftedRegister());
    return shift_;
  }

  Extend extend() const {
    assert(IsExtendedRegister());
    return extend_;
  }

  unsigned shift_amount() const {
    assert(IsShiftedRegister() || IsExtendedRegister());
    return shift_amount_;
  }

 private:
  int64_t immediate_;
  Register reg_;
  Shift shift_;
  Extend extend_;
  unsigned shift_amount_;
};


// MemOperand represents the addressing mode of a load or store instruction.
class MemOperand {
 public:
  explicit MemOperand(Register base,
                      ptrdiff_t offset = 0,
                      AddrMode addrmode = Offset);
  explicit MemOperand(Register base,
                      Register regoffset,
                      Shift shift = LSL,
                      unsigned shift_amount = 0);
  explicit MemOperand(Register base,
                      Register regoffset,
                      Extend extend,
                      unsigned shift_amount = 0);
  explicit MemOperand(Register base,
                      const Operand& offset,
                      AddrMode addrmode = Offset);

  const Register& base() const { return base_; }
  const Register& regoffset() const { return regoffset_; }
  ptrdiff_t offset() const { return offset_; }
  AddrMode addrmode() const { return addrmode_; }
  Shift shift() const { return shift_; }
  Extend extend() const { return extend_; }
  unsigned shift_amount() const { return shift_amount_; }
  bool IsImmediateOffset() const;
  bool IsRegisterOffset() const;
  bool IsPreIndex() const;
  bool IsPostIndex() const;

 private:
  Register base_;
  Register regoffset_;
  ptrdiff_t offset_;
  AddrMode addrmode_;
  Shift shift_;
  Extend extend_;
  unsigned shift_amount_;
};


class Label {
 public:
  Label() : is_bound_(false), link_(nullptr), target_(nullptr) {}
  ~Label() {
    // If the label has been linked to, it needs to be bound to a target.
    assert(!IsLinked() || IsBound());
  }

  inline HPHP::CodeAddress link() const { return link_; }
  inline HPHP::CodeAddress target() const { return target_; }

  inline bool IsBound() const { return is_bound_; }
  inline bool IsLinked() const { return link_ != nullptr; }

  inline void set_link(HPHP::CodeAddress new_link) { link_ = new_link; }

  static const int kEndOfChain = 0;

 private:
  // Indicates if the label has been bound, ie its location is fixed.
  bool is_bound_;
  // Branches instructions branching to this label form a chained list, with
  // their offset indicating where the next instruction is located.
  // link_ points to the latest branch instruction generated branching to this
  // branch.
  // If link_ is not nullptr, the label has been linked to.
  HPHP::CodeAddress link_;
  // The label location.
  HPHP::CodeAddress target_;

  friend class Assembler;
};


// TODO: Obtain better values for these, based on real-world data.
const int kLiteralPoolCheckInterval = 4 * KBytes;
const int kRecommendedLiteralPoolRange = 2 * kLiteralPoolCheckInterval;


// Control whether a branch over the literal pool should also be emitted. This
// is needed if the literal pool has to be emitted in the middle of the JITted
// code.
enum LiteralPoolEmitOption {
  JumpRequired,
  NoJumpRequired
};


// Literal pool entry.
class Literal {
 public:
  Literal(HPHP::CodeAddress pc, uint64_t imm, unsigned size)
      : pc_(pc), value_(imm), size_(size) {}

 private:
  HPHP::CodeAddress pc_;
  int64_t value_;
  unsigned size_;

  friend class Assembler;
};


// Assembler.
class Assembler {
 public:
  explicit Assembler(HPHP::CodeBlock& cb);

  // The destructor asserts that one of the following is true:
  //  * The Assembler object has not been used.
  //  * Nothing has been emitted since the last Reset() call.
  //  * Nothing has been emitted since the last FinalizeCode() call.
  ~Assembler();

  // System functions.

  // Start generating code from the beginning of the buffer, discarding any code
  // and data that has already been emitted into the buffer.
  //
  // In order to avoid any accidental transfer of state, Reset ASSERTs that the
  // constant pool is not blocked.
  void Reset();

  // Finalize a code buffer of generated instructions. This function must be
  // called before executing or copying code from the buffer.
  void FinalizeCode();

  // Label.
  // Bind a label to the current PC.
  void bind(Label* label);
  int UpdateAndGetByteOffsetTo(Label* label);
  inline int UpdateAndGetInstructionOffsetTo(Label* label) {
    assert(Label::kEndOfChain == 0);
    return UpdateAndGetByteOffsetTo(label) >> kInstructionSizeLog2;
  }

  HPHP::JIT::TCA frontier() const {
    return cb_.frontier();
  }

  bool isFrontierAligned(size_t align) const {
    return cb_.isFrontierAligned(align);
  }

  // Instruction set functions.

  // Branch / Jump instructions.
  // Branch to register.
  void br(const Register& xn);

  // Branch with link to register.
  void blr(const Register& xn);

  // Branch to register with return hint.
  void ret(const Register& xn = lr);

  // Unconditional branch to label.
  void b(Label* label);

  // Conditional branch to label.
  void b(Label* label, Condition cond);

  // Unconditional branch to PC offset.
  void b(int imm26);

  // Conditional branch to PC offset.
  void b(int imm19, Condition cond);

  // Branch with link to label.
  void bl(Label* label);

  // Branch with link to PC offset.
  void bl(int imm26);

  // Compare and branch to label if zero.
  void cbz(const Register& rt, Label* label);

  // Compare and branch to PC offset if zero.
  void cbz(const Register& rt, int imm19);

  // Compare and branch to label if not zero.
  void cbnz(const Register& rt, Label* label);

  // Compare and branch to PC offset if not zero.
  void cbnz(const Register& rt, int imm19);

  // Test bit and branch to label if zero.
  void tbz(const Register& rt, unsigned bit_pos, Label* label);

  // Test bit and branch to PC offset if zero.
  void tbz(const Register& rt, unsigned bit_pos, int imm14);

  // Test bit and branch to label if not zero.
  void tbnz(const Register& rt, unsigned bit_pos, Label* label);

  // Test bit and branch to PC offset if not zero.
  void tbnz(const Register& rt, unsigned bit_pos, int imm14);

  // Address calculation instructions.
  // Calculate a PC-relative address. Unlike for branches the offset in adr is
  // unscaled (i.e. the result can be unaligned).

  // Calculate the address of a label.
  void adr(const Register& rd, Label* label);

  // Calculate the address of a PC offset.
  void adr(const Register& rd, int imm21);

  // Data Processing instructions.
  // Add.
  void add(const Register& rd,
           const Register& rn,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);

  // Compare negative.
  void cmn(const Register& rn, const Operand& operand);

  // Subtract.
  void sub(const Register& rd,
           const Register& rn,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);

  // Compare.
  void cmp(const Register& rn, const Operand& operand);

  // Negate.
  void neg(const Register& rd,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);

  // Add with carry bit.
  void adc(const Register& rd,
           const Register& rn,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);

  // Subtract with carry bit.
  void sbc(const Register& rd,
           const Register& rn,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);

  // Negate with carry bit.
  void ngc(const Register& rd,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);

  // Logical instructions.
  // Bitwise and (A & B).
  void and_(const Register& rd,
            const Register& rn,
            const Operand& operand,
            FlagsUpdate S = LeaveFlags);

  // Bit test and set flags.
  void tst(const Register& rn, const Operand& operand);

  // Bit clear (A & ~B).
  void bic(const Register& rd,
           const Register& rn,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);

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
  inline void bfi(const Register& rd,
                  const Register& rn,
                  unsigned lsb,
                  unsigned width) {
    assert(width >= 1);
    assert(lsb + width <= rn.size());
    bfm(rd, rn, (rd.size() - lsb) & (rd.size() - 1), width - 1);
  }

  // Bitfield extract and insert low.
  inline void bfxil(const Register& rd,
                    const Register& rn,
                    unsigned lsb,
                    unsigned width) {
    assert(width >= 1);
    assert(lsb + width <= rn.size());
    bfm(rd, rn, lsb, lsb + width - 1);
  }

  // Sbfm aliases.
  // Arithmetic shift right.
  inline void asr(const Register& rd, const Register& rn, unsigned shift) {
    assert(shift < rd.size());
    sbfm(rd, rn, shift, rd.size() - 1);
  }

  // Signed bitfield insert with zero at right.
  inline void sbfiz(const Register& rd,
                    const Register& rn,
                    unsigned lsb,
                    unsigned width) {
    assert(width >= 1);
    assert(lsb + width <= rn.size());
    sbfm(rd, rn, (rd.size() - lsb) & (rd.size() - 1), width - 1);
  }

  // Signed bitfield extract.
  inline void sbfx(const Register& rd,
                   const Register& rn,
                   unsigned lsb,
                   unsigned width) {
    assert(width >= 1);
    assert(lsb + width <= rn.size());
    sbfm(rd, rn, lsb, lsb + width - 1);
  }

  // Signed extend byte.
  inline void sxtb(const Register& rd, const Register& rn) {
    sbfm(rd, rn, 0, 7);
  }

  // Signed extend halfword.
  inline void sxth(const Register& rd, const Register& rn) {
    sbfm(rd, rn, 0, 15);
  }

  // Signed extend word.
  inline void sxtw(const Register& rd, const Register& rn) {
    sbfm(rd, rn, 0, 31);
  }

  // Ubfm aliases.
  // Logical shift left.
  inline void lsl(const Register& rd, const Register& rn, unsigned shift) {
    unsigned reg_size = rd.size();
    assert(shift < reg_size);
    ubfm(rd, rn, (reg_size - shift) % reg_size, reg_size - shift - 1);
  }

  // Logical shift right.
  inline void lsr(const Register& rd, const Register& rn, unsigned shift) {
    assert(shift < rd.size());
    ubfm(rd, rn, shift, rd.size() - 1);
  }

  // Unsigned bitfield insert with zero at right.
  inline void ubfiz(const Register& rd,
                    const Register& rn,
                    unsigned lsb,
                    unsigned width) {
    assert(width >= 1);
    assert(lsb + width <= rn.size());
    ubfm(rd, rn, (rd.size() - lsb) & (rd.size() - 1), width - 1);
  }

  // Unsigned bitfield extract.
  inline void ubfx(const Register& rd,
                   const Register& rn,
                   unsigned lsb,
                   unsigned width) {
    assert(width >= 1);
    assert(lsb + width <= rn.size());
    ubfm(rd, rn, lsb, lsb + width - 1);
  }

  // Unsigned extend byte.
  inline void uxtb(const Register& rd, const Register& rn) {
    ubfm(rd, rn, 0, 7);
  }

  // Unsigned extend halfword.
  inline void uxth(const Register& rd, const Register& rn) {
    ubfm(rd, rn, 0, 15);
  }

  // Unsigned extend word.
  inline void uxtw(const Register& rd, const Register& rn) {
    ubfm(rd, rn, 0, 31);
  }

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
  inline void ror(const Register& rd, const Register& rs, unsigned shift) {
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

  // Multiply.
  void mul(const Register& rd, const Register& rn, const Register& rm);

  // Negated multiply.
  void mneg(const Register& rd, const Register& rn, const Register& rm);

  // Signed long multiply: 32 x 32 -> 64-bit.
  void smull(const Register& rd, const Register& rn, const Register& rm);

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
  void smaddl(const Register& rd,
              const Register& rn,
              const Register& rm,
              const Register& ra);

  // Unsigned long multiply and accumulate: 32 x 32 + 64 -> 64-bit.
  void umaddl(const Register& rd,
              const Register& rn,
              const Register& rm,
              const Register& ra);

  // Signed long multiply and subtract: 64 - (32 x 32) -> 64-bit.
  void smsubl(const Register& rd,
              const Register& rn,
              const Register& rm,
              const Register& ra);

  // Unsigned long multiply and subtract: 64 - (32 x 32) -> 64-bit.
  void umsubl(const Register& rd,
              const Register& rn,
              const Register& rm,
              const Register& ra);

  // Signed integer divide.
  void sdiv(const Register& rd, const Register& rn, const Register& rm);

  // Unsigned integer divide.
  void udiv(const Register& rd, const Register& rn, const Register& rm);

  // Bit reverse.
  void rbit(const Register& rd, const Register& rn);

  // Reverse bytes in 16-bit half words.
  void rev16(const Register& rd, const Register& rn);

  // Reverse bytes in 32-bit words.
  void rev32(const Register& rd, const Register& rn);

  // Reverse bytes.
  void rev(const Register& rd, const Register& rn);

  // Count leading zeroes.
  void clz(const Register& rd, const Register& rn);

  // Count leading sign bits.
  void cls(const Register& rd, const Register& rn);

  // Memory instructions.
  // Load integer or FP register.
  void ldr(const CPURegister& rt, const MemOperand& src);

  // Store integer or FP register.
  void str(const CPURegister& rt, const MemOperand& dst);

  // PC-relative load.
  void ldr(const Register& rt, Label* label);

  // Load word with sign extension.
  void ldrsw(const Register& rt, const MemOperand& src);

  // Load byte.
  void ldrb(const Register& rt, const MemOperand& src);

  // Store byte.
  void strb(const Register& rt, const MemOperand& dst);

  // Load byte with sign extension.
  void ldrsb(const Register& rt, const MemOperand& src);

  // Load half-word.
  void ldrh(const Register& rt, const MemOperand& src);

  // Store half-word.
  void strh(const Register& rt, const MemOperand& dst);

  // Load half-word with sign extension.
  void ldrsh(const Register& rt, const MemOperand& src);

  // Load integer or FP register pair.
  void ldp(const CPURegister& rt, const CPURegister& rt2,
           const MemOperand& src);

  // Store integer or FP register pair.
  void stp(const CPURegister& rt, const CPURegister& rt2,
           const MemOperand& dst);

  // Load word pair with sign extension.
  void ldpsw(const Register& rt, const Register& rt2, const MemOperand& src);

  // Load integer or FP register pair, non-temporal.
  void ldnp(const CPURegister& rt, const CPURegister& rt2,
            const MemOperand& src);

  // Store integer or FP register pair, non-temporal.
  void stnp(const CPURegister& rt, const CPURegister& rt2,
            const MemOperand& dst);

  // Load literal to register.
  void ldr(const Register& rt, uint64_t imm);

  // Load literal to FP register.
  void ldr(const FPRegister& ft, double imm);

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

  // Misc instructions.
  // Monitor debug-mode breakpoint.
  void brk(int code);

  // Halting debug-mode breakpoint.
  void hlt(int code);

  // Move register to register.
  void mov(const Register& rd, const Register& rn);

  // Move inverted operand to register.
  void mvn(const Register& rd, const Operand& operand);

  // System instructions.
  // Move to register from system register.
  void mrs(const Register& rt, SystemRegister sysreg);

  // Move from register to system register.
  void msr(SystemRegister sysreg, const Register& rt);

  // System hint.
  void hint(SystemHint code);

  // Alias for system instructions.
  // No-op.
  void nop() {
    hint(NOP);
  }

  // FP instructions.
  // Move immediate to FP register.
  void fmov(FPRegister fd, double imm);

  // Move FP register to register.
  void fmov(Register rd, FPRegister fn);

  // Move register to FP register.
  void fmov(FPRegister fd, Register rn);

  // Move FP register to FP register.
  void fmov(FPRegister fd, FPRegister fn);

  // FP add.
  void fadd(const FPRegister& fd, const FPRegister& fn, const FPRegister& fm);

  // FP subtract.
  void fsub(const FPRegister& fd, const FPRegister& fn, const FPRegister& fm);

  // FP multiply.
  void fmul(const FPRegister& fd, const FPRegister& fn, const FPRegister& fm);

  // FP multiply and subtract.
  void fmsub(const FPRegister& fd,
             const FPRegister& fn,
             const FPRegister& fm,
             const FPRegister& fa);

  // FP divide.
  void fdiv(const FPRegister& fd, const FPRegister& fn, const FPRegister& fm);

  // FP maximum.
  void fmax(const FPRegister& fd, const FPRegister& fn, const FPRegister& fm);

  // FP minimum.
  void fmin(const FPRegister& fd, const FPRegister& fn, const FPRegister& fm);

  // FP absolute.
  void fabs(const FPRegister& fd, const FPRegister& fn);

  // FP negate.
  void fneg(const FPRegister& fd, const FPRegister& fn);

  // FP square root.
  void fsqrt(const FPRegister& fd, const FPRegister& fn);

  // FP round to integer (nearest with ties to even).
  void frintn(const FPRegister& fd, const FPRegister& fn);

  // FP round to integer (towards zero).
  void frintz(const FPRegister& fd, const FPRegister& fn);

  // FP compare registers.
  void fcmp(const FPRegister& fn, const FPRegister& fm);

  // FP compare immediate.
  void fcmp(const FPRegister& fn, double value);

  // FP conditional compare.
  void fccmp(const FPRegister& fn,
             const FPRegister& fm,
             StatusFlags nzcv,
             Condition cond);

  // FP conditional select.
  void fcsel(const FPRegister& fd,
             const FPRegister& fn,
             const FPRegister& fm,
             Condition cond);

  // Common FP Convert function.
  void FPConvertToInt(const Register& rd,
                      const FPRegister& fn,
                      FPIntegerConvertOp op);

  // FP convert between single and double precision.
  void fcvt(const FPRegister& fd, const FPRegister& fn);

  // Convert FP to unsigned integer (round towards -infinity).
  void fcvtmu(const Register& rd, const FPRegister& fn);

  // Convert FP to signed integer (round towards -infinity).
  void fcvtms(const Register& rd, const FPRegister& fn);

  // Convert FP to unsigned integer (nearest with ties to even).
  void fcvtnu(const Register& rd, const FPRegister& fn);

  // Convert FP to signed integer (nearest with ties to even).
  void fcvtns(const Register& rd, const FPRegister& fn);

  // Convert FP to unsigned integer (round towards zero).
  void fcvtzu(const Register& rd, const FPRegister& fn);

  // Convert FP to signed integer (round towards zero).
  void fcvtzs(const Register& rd, const FPRegister& fn);

  // Convert signed integer or fixed point to FP.
  void scvtf(const FPRegister& fd, const Register& rn, unsigned fbits = 0);

  // Convert unsigned integer or fixed point to FP.
  void ucvtf(const FPRegister& fd, const Register& rn, unsigned fbits = 0);

  // Emit generic instructions.
  // Emit raw instructions into the instruction stream.
  inline void dci(Instr raw_inst) { Emit(raw_inst); }

  // Emit 32 bits of data into the instruction stream.
  inline void dc32(uint32_t data) { EmitData(&data, sizeof(data)); }

  // Emit 64 bits of data into the instruction stream.
  inline void dc64(uint64_t data) { EmitData(&data, sizeof(data)); }

  template<typename T>
  inline void dc64(T* data) { dc64(reinterpret_cast<uint64_t>(data)); }

  // Copy a string into the instruction stream, including the terminating NUL
  // character. The instruction pointer (pc_) is then aligned correctly for
  // subsequent instructions.
  void EmitStringData(const char * string) {
    assert(string != nullptr);

    size_t len = strlen(string) + 1;
    EmitData(string, len);

    // Pad with NUL characters until pc_ is aligned.
    const char pad[] = {'\0', '\0', '\0', '\0'};
    assert(sizeof(pad) == kInstructionSize);
    auto next_pc = AlignUp(cb_.frontier(), kInstructionSize);
    EmitData(&pad, next_pc - cb_.frontier());
  }

  // Code generation helpers.

  // Register encoding.
  static Instr Rd(CPURegister rd) {
    assert(rd.code() != kSPRegInternalCode);
    return rd.code() << Rd_offset;
  }

  static Instr Rn(CPURegister rn) {
    assert(rn.code() != kSPRegInternalCode);
    return rn.code() << Rn_offset;
  }

  static Instr Rm(CPURegister rm) {
    assert(rm.code() != kSPRegInternalCode);
    return rm.code() << Rm_offset;
  }

  static Instr Ra(CPURegister ra) {
    assert(ra.code() != kSPRegInternalCode);
    return ra.code() << Ra_offset;
  }

  static Instr Rt(CPURegister rt) {
    assert(rt.code() != kSPRegInternalCode);
    return rt.code() << Rt_offset;
  }

  static Instr Rt2(CPURegister rt2) {
    assert(rt2.code() != kSPRegInternalCode);
    return rt2.code() << Rt2_offset;
  }

  // These encoding functions allow the stack pointer to be encoded, and
  // disallow the zero register.
  static Instr RdSP(Register rd) {
    assert(!rd.IsZero());
    if (rd.code() == kSPRegInternalCode) {
      return kSPRegCode << Rd_offset;
    } else {
      return rd.code() << Rd_offset;
    }
  }

  static Instr RnSP(Register rn) {
    assert(!rn.IsZero());
    if (rn.code() == kSPRegInternalCode) {
      return kSPRegCode << Rn_offset;
    } else {
      return rn.code() << Rn_offset;
    }
  }

  // Flags encoding.
  static Instr Flags(FlagsUpdate S) {
    if (S == SetFlags) {
      return 1 << FlagsUpdate_offset;
    } else if (S == LeaveFlags) {
      return 0 << FlagsUpdate_offset;
    }
    not_reached();
    return 0;
  }

  static Instr Cond(Condition cond) {
    return cond << Condition_offset;
  }

  // PC-relative address encoding.
  static Instr ImmPCRelAddress(int imm21) {
    assert(is_int21(imm21));
    Instr imm = static_cast<Instr>(truncate_to_int21(imm21));
    Instr immhi = (imm >> ImmPCRelLo_width) << ImmPCRelHi_offset;
    Instr immlo = imm << ImmPCRelLo_offset;
    return (immhi & ImmPCRelHi_mask) | (immlo & ImmPCRelLo_mask);
  }

  static Instr ImmPCRelLoadStoreAddress(int imm19) {
    assert(is_int19(imm19));
    return truncate_to_int19(imm19) << ImmLLiteral_offset;
  }

  // Branch encoding.
  static Instr ImmUncondBranch(int imm26) {
    assert(is_int26(imm26));
    return truncate_to_int26(imm26) << ImmUncondBranch_offset;
  }

  static Instr ImmCondBranch(int imm19) {
    assert(is_int19(imm19));
    return truncate_to_int19(imm19) << ImmCondBranch_offset;
  }

  static Instr ImmCmpBranch(int imm19) {
    assert(is_int19(imm19));
    return truncate_to_int19(imm19) << ImmCmpBranch_offset;
  }

  static Instr ImmTestBranch(int imm14) {
    assert(is_int14(imm14));
    return truncate_to_int14(imm14) << ImmTestBranch_offset;
  }

  static Instr ImmTestBranchBit(unsigned bit_pos) {
    assert(is_uint6(bit_pos));
    // Subtract five from the shift offset, as we need bit 5 from bit_pos.
    unsigned b5 = bit_pos << (ImmTestBranchBit5_offset - 5);
    unsigned b40 = bit_pos << ImmTestBranchBit40_offset;
    b5 &= ImmTestBranchBit5_mask;
    b40 &= ImmTestBranchBit40_mask;
    return b5 | b40;
  }

  // Data Processing encoding.
  static Instr SF(Register rd) {
      return rd.Is64Bits() ? SixtyFourBits : ThirtyTwoBits;
  }

  static Instr ImmAddSub(int64_t imm) {
    assert(IsImmAddSub(imm));
    if (is_uint12(imm)) {  // No shift required.
      return imm << ImmAddSub_offset;
    } else {
      return ((imm >> 12) << ImmAddSub_offset) | (1 << ShiftAddSub_offset);
    }
  }

  static inline Instr ImmS(unsigned imms, unsigned reg_size) {
    assert(((reg_size == kXRegSize) && is_uint6(imms)) ||
           ((reg_size == kWRegSize) && is_uint5(imms)));
    USE(reg_size);
    return imms << ImmS_offset;
  }

  static inline Instr ImmR(unsigned immr, unsigned reg_size) {
    assert(((reg_size == kXRegSize) && is_uint6(immr)) ||
           ((reg_size == kWRegSize) && is_uint5(immr)));
    USE(reg_size);
    assert(is_uint6(immr));
    return immr << ImmR_offset;
  }

  static inline Instr ImmSetBits(unsigned imms, unsigned reg_size) {
    assert((reg_size == kWRegSize) || (reg_size == kXRegSize));
    assert(is_uint6(imms));
    assert((reg_size == kXRegSize) || is_uint6(imms + 3));
    USE(reg_size);
    return imms << ImmSetBits_offset;
  }

  static inline Instr ImmRotate(unsigned immr, unsigned reg_size) {
    assert((reg_size == kWRegSize) || (reg_size == kXRegSize));
    assert(((reg_size == kXRegSize) && is_uint6(immr)) ||
           ((reg_size == kWRegSize) && is_uint5(immr)));
    USE(reg_size);
    return immr << ImmRotate_offset;
  }

  static inline Instr ImmLLiteral(int imm19) {
    assert(is_int19(imm19));
    return truncate_to_int19(imm19) << ImmLLiteral_offset;
  }

  static inline Instr BitN(unsigned bitn, unsigned reg_size) {
    assert((reg_size == kWRegSize) || (reg_size == kXRegSize));
    assert((reg_size == kXRegSize) || (bitn == 0));
    USE(reg_size);
    return bitn << BitN_offset;
  }

  static Instr ShiftDP(Shift shift) {
    assert(shift == LSL || shift == LSR || shift == ASR || shift == ROR);
    return shift << ShiftDP_offset;
  }

  static Instr ImmDPShift(unsigned amount) {
    assert(is_uint6(amount));
    return amount << ImmDPShift_offset;
  }

  static Instr ExtendMode(Extend extend) {
    return extend << ExtendMode_offset;
  }

  static Instr ImmExtendShift(unsigned left_shift) {
    assert(left_shift <= 4);
    return left_shift << ImmExtendShift_offset;
  }

  static Instr ImmCondCmp(unsigned imm) {
    assert(is_uint5(imm));
    return imm << ImmCondCmp_offset;
  }

  static Instr Nzcv(StatusFlags nzcv) {
    return ((nzcv >> Flags_offset) & 0xf) << Nzcv_offset;
  }

  // MemOperand offset encoding.
  static Instr ImmLSUnsigned(int imm12) {
    assert(is_uint12(imm12));
    return imm12 << ImmLSUnsigned_offset;
  }

  static Instr ImmLS(int imm9) {
    assert(is_int9(imm9));
    return truncate_to_int9(imm9) << ImmLS_offset;
  }

  static Instr ImmLSPair(int imm7, LSDataSize size) {
    assert(((imm7 >> size) << size) == imm7);
    int scaled_imm7 = imm7 >> size;
    assert(is_int7(scaled_imm7));
    return truncate_to_int7(scaled_imm7) << ImmLSPair_offset;
  }

  static Instr ImmShiftLS(unsigned shift_amount) {
    assert(is_uint1(shift_amount));
    return shift_amount << ImmShiftLS_offset;
  }

  static Instr ImmException(int imm16) {
    assert(is_uint16(imm16));
    return imm16 << ImmException_offset;
  }

  static Instr ImmSystemRegister(int imm15) {
    assert(is_uint15(imm15));
    return imm15 << ImmSystemRegister_offset;
  }

  static Instr ImmHint(int imm7) {
    assert(is_uint7(imm7));
    return imm7 << ImmHint_offset;
  }

  static LSDataSize CalcLSDataSize(LoadStoreOp op) {
    assert((SizeLS_offset + SizeLS_width) == (kInstructionSize * 8));
    return static_cast<LSDataSize>(op >> SizeLS_offset);
  }

  // Move immediates encoding.
  static Instr ImmMoveWide(uint64_t imm) {
    assert(is_uint16(imm));
    return imm << ImmMoveWide_offset;
  }

  static Instr ShiftMoveWide(int64_t shift) {
    assert(is_uint2(shift));
    return shift << ShiftMoveWide_offset;
  }

  // FP Immediates.
  static Instr ImmFP32(float imm);
  static Instr ImmFP64(double imm);

  // FP register type.
  static Instr FPType(FPRegister fd) {
    return fd.Is64Bits() ? FP64 : FP32;
  }

  static Instr FPScale(unsigned scale) {
    assert(is_uint6(scale));
    return scale << FPScale_offset;
  }

  // Size of the code generated in bytes
  uint64_t SizeOfCodeGenerated() const {
    assert(cb_.available() > 0);
    return cb_.used();
  }

  // Size of the code generated since label to the current position.
  uint64_t SizeOfCodeGeneratedSince(Label* label) const {
    assert(label->IsBound());
    auto addr = label->target();
    assert(cb_.frontier() >= addr);
    return cb_.frontier() - addr;
  }


  inline void BlockLiteralPool() {
    literal_pool_monitor_++;
  }

  inline void ReleaseLiteralPool() {
    if (--literal_pool_monitor_ == 0) {
      // Has the literal pool been blocked for too long?
      assert(literals_.empty() ||
             (cb_.frontier() < (literals_.back()->pc_ + kMaxLoadLiteralRange)));
    }
  }

  inline bool IsLiteralPoolBlocked() {
    return literal_pool_monitor_ != 0;
  }

  void CheckLiteralPool(LiteralPoolEmitOption option = JumpRequired);
  void EmitLiteralPool(LiteralPoolEmitOption option = NoJumpRequired);
  size_t LiteralPoolSize();

  static bool IsImmLogical(uint64_t value, unsigned width) {
    unsigned ignored;
    return IsImmLogical(value, width, &ignored, &ignored, &ignored);
  }

  static bool IsImmArithmetic(int64_t value) {
    return IsImmAddSub(value);
  }

 protected:
  inline const Register& AppropriateZeroRegFor(const CPURegister& reg) const {
    return reg.Is64Bits() ? xzr : wzr;
  }


  void LoadStore(const CPURegister& rt,
                 const MemOperand& addr,
                 LoadStoreOp op);
  static bool IsImmLSUnscaled(ptrdiff_t offset);
  static bool IsImmLSScaled(ptrdiff_t offset, LSDataSize size);

  void Logical(const Register& rd,
               const Register& rn,
               const Operand& operand,
               LogicalOp op);
  void LogicalImmediate(const Register& rd,
                        const Register& rn,
                        unsigned n,
                        unsigned imm_s,
                        unsigned imm_r,
                        LogicalOp op);
  static bool IsImmLogical(uint64_t value,
                           unsigned width,
                           unsigned* n,
                           unsigned* imm_s,
                           unsigned* imm_r);

  void ConditionalCompare(const Register& rn,
                          const Operand& operand,
                          StatusFlags nzcv,
                          Condition cond,
                          ConditionalCompareOp op);
  static bool IsImmConditionalCompare(int64_t immediate);

  void AddSubWithCarry(const Register& rd,
                       const Register& rn,
                       const Operand& operand,
                       FlagsUpdate S,
                       AddSubWithCarryOp op);

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
  static bool IsImmAddSub(int64_t immediate);

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


 private:
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
  void LoadStorePair(const CPURegister& rt,
                     const CPURegister& rt2,
                     const MemOperand& addr,
                     LoadStorePairOp op);
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
  void FPDataProcessing1Source(const FPRegister& fd,
                               const FPRegister& fn,
                               FPDataProcessing1SourceOp op);
  void FPDataProcessing2Source(const FPRegister& fd,
                               const FPRegister& fn,
                               const FPRegister& fm,
                               FPDataProcessing2SourceOp op);
  void FPDataProcessing3Source(const FPRegister& fd,
                               const FPRegister& fn,
                               const FPRegister& fm,
                               const FPRegister& fa,
                               FPDataProcessing3SourceOp op);

  // Encoding helpers.
  static bool IsImmFP32(float imm);
  static bool IsImmFP64(double imm);

  void RecordLiteral(int64_t imm, unsigned size);

  // Emit the instruction at pc_.
  void Emit(Instr instruction) {
    assert(cb_.canEmit(sizeof(instruction)));
    assert(sizeof(instruction) == sizeof(uint32_t));

#ifdef DEBUG
    finalized_ = false;
#endif

    cb_.dword(instruction);
    CheckBufferSpace();
  }

  // Emit data inline in the instruction stream.
  void EmitData(void const * data, unsigned size) {
    assert(cb_.canEmit(size));

#ifdef DEBUG
    finalized_ = false;
#endif

    // TODO: Record this 'instruction' as data, so that it can be disassembled
    // correctly.
    cb_.bytes(size, reinterpret_cast<const uint8_t*>(data));
    CheckBufferSpace();
  }

  inline void CheckBufferSpace() {
    assert(cb_.available() > 0);
    if (cb_.frontier() > next_literal_pool_check_) {
      CheckLiteralPool();
    }
  }

  // The buffer into which code and relocation info are generated.
  HPHP::CodeBlock& cb_;

  std::list<Literal*> literals_;
  HPHP::CodeAddress next_literal_pool_check_;
  unsigned literal_pool_monitor_;

  friend class BlockLiteralPoolScope;

#ifdef DEBUG
  bool finalized_;
#endif
};

class BlockLiteralPoolScope {
 public:
  explicit BlockLiteralPoolScope(Assembler* assm) : assm_(assm) {
    assm_->BlockLiteralPool();
  }

  ~BlockLiteralPoolScope() {
    assm_->ReleaseLiteralPool();
  }

 private:
  Assembler* assm_;
};
}  // namespace vixl

#endif  // VIXL_A64_ASSEMBLER_A64_H_
