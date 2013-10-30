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

#ifndef VIXL_A64_MACRO_ASSEMBLER_A64_H_
#define VIXL_A64_MACRO_ASSEMBLER_A64_H_

#include "hphp/vixl/globals.h"
#include "hphp/vixl/a64/assembler-a64.h"
#include "hphp/vixl/a64/debugger-a64.h"


#define LS_MACRO_LIST(V)                                      \
  V(Ldrb, Register&, rt, LDRB_w)                              \
  V(Strb, Register&, rt, STRB_w)                              \
  V(Ldrsb, Register&, rt, rt.Is64Bits() ? LDRSB_x : LDRSB_w)  \
  V(Ldrh, Register&, rt, LDRH_w)                              \
  V(Strh, Register&, rt, STRH_w)                              \
  V(Ldrsh, Register&, rt, rt.Is64Bits() ? LDRSH_x : LDRSH_w)  \
  V(Ldr, CPURegister&, rt, LoadOpFor(rt))                     \
  V(Str, CPURegister&, rt, StoreOpFor(rt))                    \
  V(Ldrsw, Register&, rt, LDRSW_x)

namespace vixl {

class MacroAssembler : public Assembler {
 public:
  explicit MacroAssembler(HPHP::CodeBlock& cb)
      : Assembler(cb),
#ifdef DEBUG
        allow_macro_instructions_(true),
#endif
        sp_(sp), tmp0_(ip0), tmp1_(ip1), fptmp0_(d31) {}

  // Logical macros.
  void And(const Register& rd,
           const Register& rn,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);
  void Bic(const Register& rd,
           const Register& rn,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);
  void Orr(const Register& rd,
           const Register& rn,
           const Operand& operand);
  void Orn(const Register& rd,
           const Register& rn,
           const Operand& operand);
  void Eor(const Register& rd,
           const Register& rn,
           const Operand& operand);
  void Eon(const Register& rd,
           const Register& rn,
           const Operand& operand);
  void Tst(const Register& rn, const Operand& operand);
  void LogicalMacro(const Register& rd,
                    const Register& rn,
                    const Operand& operand,
                    LogicalOp op);

  // Add and sub macros.
  void Add(const Register& rd,
           const Register& rn,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);
  void Sub(const Register& rd,
           const Register& rn,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);
  void Cmn(const Register& rn, const Operand& operand);
  void Cmp(const Register& rn, const Operand& operand);
  void Neg(const Register& rd,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);
  void AddSubMacro(const Register& rd,
                   const Register& rn,
                   const Operand& operand,
                   FlagsUpdate S,
                   AddSubOp op);

  // Add/sub with carry macros.
  void Adc(const Register& rd,
           const Register& rn,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);
  void Sbc(const Register& rd,
           const Register& rn,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);
  void Ngc(const Register& rd,
           const Operand& operand,
           FlagsUpdate S = LeaveFlags);
  void AddSubWithCarryMacro(const Register& rd,
                            const Register& rn,
                            const Operand& operand,
                            FlagsUpdate S,
                            AddSubWithCarryOp op);

  // Move macros.
  void Mov(const Register& rd, uint64_t imm);
  void Mov(const Register& rd, const Operand& operand);
  template<typename T>
  void Mov(const Register& rd, T* imm) {
    static_assert(sizeof(T*) == sizeof(uint64_t), "");
    Mov(rd, reinterpret_cast<uint64_t>(imm));
  }
  void Mvn(const Register& rd, uint64_t imm) {
    Mov(rd, ~imm);
  };
  void Mvn(const Register& rd, const Operand& operand);
  bool IsImmMovn(uint64_t imm, unsigned reg_size);
  bool IsImmMovz(uint64_t imm, unsigned reg_size);

  // Conditional compare macros.
  void Ccmp(const Register& rn,
            const Operand& operand,
            StatusFlags nzcv,
            Condition cond);
  void Ccmn(const Register& rn,
            const Operand& operand,
            StatusFlags nzcv,
            Condition cond);
  void ConditionalCompareMacro(const Register& rn,
                               const Operand& operand,
                               StatusFlags nzcv,
                               Condition cond,
                               ConditionalCompareOp op);

  // Load/store macros.
#define DECLARE_FUNCTION(FN, REGTYPE, REG, OP) \
  void FN(const REGTYPE REG, const MemOperand& addr);
  LS_MACRO_LIST(DECLARE_FUNCTION)
#undef DECLARE_FUNCTION

  void LoadStoreMacro(const CPURegister& rt,
                      const MemOperand& addr,
                      LoadStoreOp op);

  // Push or pop up to 4 registers of the same width to or from the stack,
  // using the current stack pointer as set by SetStackPointer.
  //
  // If an argument register is 'NoReg', all further arguments are also assumed
  // to be 'NoReg', and are thus not pushed or popped.
  //
  // Arguments are ordered such that "Push(a, b);" is functionally equivalent
  // to "Push(a); Push(b);".
  //
  // It is valid to push the same register more than once, and there is no
  // restriction on the order in which registers are specified.
  //
  // It is not valid to pop into the same register more than once in one
  // operation, not even into the zero register.
  //
  // If the current stack pointer (as set by SetStackPointer) is sp, then it
  // must be aligned to 16 bytes on entry and the total size of the specified
  // registers must also be a multiple of 16 bytes.
  //
  // Even if the current stack pointer is not the system stack pointer (sp),
  // Push (and derived methods) will still modify the system stack pointer in
  // order to comply with ABI rules about accessing memory below the system
  // stack pointer.
  //
  // Other than the registers passed into Pop, the stack pointer and (possibly)
  // the system stack pointer, these methods do not modify any other registers.
  // Scratch registers such as Tmp0() and Tmp1() are preserved.
  void Push(const CPURegister& src0, const CPURegister& src1 = NoReg,
            const CPURegister& src2 = NoReg, const CPURegister& src3 = NoReg);
  void Pop(const CPURegister& dst0, const CPURegister& dst1 = NoReg,
           const CPURegister& dst2 = NoReg, const CPURegister& dst3 = NoReg);

  // Alternative forms of Push and Pop, taking a RegList or CPURegList that
  // specifies the registers that are to be pushed or popped. Higher-numbered
  // registers are associated with higher memory addresses (as in the A32 push
  // and pop instructions).
  //
  // (Push|Pop)SizeRegList allow you to specify the register size as a
  // parameter. Only kXRegSize, kWRegSize, kDRegSize and kSRegSize are
  // supported.
  //
  // Otherwise, (Push|Pop)(CPU|X|W|D|S)RegList is preferred.
  void PushCPURegList(CPURegList registers);
  void PopCPURegList(CPURegList registers);

  void PushSizeRegList(RegList registers, unsigned reg_size,
      CPURegister::RegisterType type = CPURegister::kRegister) {
    PushCPURegList(CPURegList(type, reg_size, registers));
  }
  void PopSizeRegList(RegList registers, unsigned reg_size,
      CPURegister::RegisterType type = CPURegister::kRegister) {
    PopCPURegList(CPURegList(type, reg_size, registers));
  }
  void PushXRegList(RegList regs) {
    PushSizeRegList(regs, kXRegSize);
  }
  void PopXRegList(RegList regs) {
    PopSizeRegList(regs, kXRegSize);
  }
  void PushWRegList(RegList regs) {
    PushSizeRegList(regs, kWRegSize);
  }
  void PopWRegList(RegList regs) {
    PopSizeRegList(regs, kWRegSize);
  }
  inline void PushDRegList(RegList regs) {
    PushSizeRegList(regs, kDRegSize, CPURegister::kFPRegister);
  }
  inline void PopDRegList(RegList regs) {
    PopSizeRegList(regs, kDRegSize, CPURegister::kFPRegister);
  }
  inline void PushSRegList(RegList regs) {
    PushSizeRegList(regs, kSRegSize, CPURegister::kFPRegister);
  }
  inline void PopSRegList(RegList regs) {
    PopSizeRegList(regs, kSRegSize, CPURegister::kFPRegister);
  }

  // Push the specified register 'count' times.
  void PushMultipleTimes(int count, Register src);

  // Poke 'src' onto the stack. The offset is in bytes.
  //
  // If the current stack pointer (as set by SetStackPointer) is sp, then sp
  // must be aligned to 16 bytes.
  void Poke(const Register& src, const Operand& offset);

  // Peek at a value on the stack, and put it in 'dst'. The offset is in bytes.
  //
  // If the current stack pointer (as set by SetStackPointer) is sp, then sp
  // must be aligned to 16 bytes.
  void Peek(const Register& dst, const Operand& offset);

  // Claim or drop stack space without actually accessing memory.
  //
  // If the current stack pointer (as set by SetStackPointer) is sp, then it
  // must be aligned to 16 bytes and the size claimed or dropped must be a
  // multiple of 16 bytes.
  void Claim(const Operand& size);
  void Drop(const Operand& size);

  // Preserve the callee-saved registers (as defined by AAPCS64).
  //
  // Higher-numbered registers are pushed before lower-numbered registers, and
  // thus get higher addresses.
  // Floating-point registers are pushed before general-purpose registers, and
  // thus get higher addresses.
  //
  // This method must not be called unless StackPointer() is sp, and it is
  // aligned to 16 bytes.
  void PushCalleeSavedRegisters();

  // Restore the callee-saved registers (as defined by AAPCS64).
  //
  // Higher-numbered registers are popped after lower-numbered registers, and
  // thus come from higher addresses.
  // Floating-point registers are popped after general-purpose registers, and
  // thus come from higher addresses.
  //
  // This method must not be called unless StackPointer() is sp, and it is
  // aligned to 16 bytes.
  void PopCalleeSavedRegisters();

  // Remaining instructions are simple pass-through calls to the assembler.
  void Adr(const Register& rd, Label* label) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    adr(rd, label);
  }
  void Asr(const Register& rd, const Register& rn, unsigned shift) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    asr(rd, rn, shift);
  }
  void Asr(const Register& rd, const Register& rn, const Register& rm) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    asrv(rd, rn, rm);
  }
  void B(Label* label) {
    b(label);
  }
  void B(Label* label, Condition cond) {
    assert(allow_macro_instructions_);
    assert((cond != al) && (cond != nv));
    b(label, cond);
  }
  void Bfi(const Register& rd,
           const Register& rn,
           unsigned lsb,
           unsigned width) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    bfi(rd, rn, lsb, width);
  }
  void Bfxil(const Register& rd,
             const Register& rn,
             unsigned lsb,
             unsigned width) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    bfxil(rd, rn, lsb, width);
  }
  void Bind(Label* label) {
    assert(allow_macro_instructions_);
    bind(label);
  }
  void Bl(Label* label) {
    assert(allow_macro_instructions_);
    bl(label);
  }
  void Blr(const Register& xn) {
    assert(allow_macro_instructions_);
    assert(!xn.IsZero());
    blr(xn);
  }
  void Br(const Register& xn) {
    assert(allow_macro_instructions_);
    assert(!xn.IsZero());
    br(xn);
  }
  void Brk(int code = 0) {
    assert(allow_macro_instructions_);
    brk(code);
  }
  void Cbnz(const Register& rt, Label* label) {
    assert(allow_macro_instructions_);
    assert(!rt.IsZero());
    cbnz(rt, label);
  }
  void Cbz(const Register& rt, Label* label) {
    assert(allow_macro_instructions_);
    assert(!rt.IsZero());
    cbz(rt, label);
  }
  void Cinc(const Register& rd, const Register& rn, Condition cond) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    cinc(rd, rn, cond);
  }
  void Cinv(const Register& rd, const Register& rn, Condition cond) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    cinv(rd, rn, cond);
  }
  void Cls(const Register& rd, const Register& rn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    cls(rd, rn);
  }
  void Clz(const Register& rd, const Register& rn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    clz(rd, rn);
  }
  void Cneg(const Register& rd, const Register& rn, Condition cond) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    cneg(rd, rn, cond);
  }
  void Csel(const Register& rd,
            const Register& rn,
            const Register& rm,
            Condition cond) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    assert((cond != al) && (cond != nv));
    csel(rd, rn, rm, cond);
  }
  void Cset(const Register& rd, Condition cond) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    cset(rd, cond);
  }
  void Csetm(const Register& rd, Condition cond) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    csetm(rd, cond);
  }
  void Csinc(const Register& rd,
             const Register& rn,
             const Register& rm,
             Condition cond) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    assert((cond != al) && (cond != nv));
    csinc(rd, rn, rm, cond);
  }
  void Csinv(const Register& rd,
             const Register& rn,
             const Register& rm,
             Condition cond) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    assert((cond != al) && (cond != nv));
    csinv(rd, rn, rm, cond);
  }
  void Csneg(const Register& rd,
             const Register& rn,
             const Register& rm,
             Condition cond) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    assert((cond != al) && (cond != nv));
    csneg(rd, rn, rm, cond);
  }
  void Extr(const Register& rd,
            const Register& rn,
            const Register& rm,
            unsigned lsb) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    extr(rd, rn, rm, lsb);
  }
  void Fabs(const FPRegister& fd, const FPRegister& fn) {
    assert(allow_macro_instructions_);
    fabs(fd, fn);
  }
  void Fadd(const FPRegister& fd, const FPRegister& fn, const FPRegister& fm) {
    assert(allow_macro_instructions_);
    fadd(fd, fn, fm);
  }
  void Fccmp(const FPRegister& fn,
             const FPRegister& fm,
             StatusFlags nzcv,
             Condition cond) {
    assert(allow_macro_instructions_);
    assert((cond != al) && (cond != nv));
    fccmp(fn, fm, nzcv, cond);
  }
  void Fcmp(const FPRegister& fn, const FPRegister& fm) {
    assert(allow_macro_instructions_);
    fcmp(fn, fm);
  }
  void Fcmp(const FPRegister& fn, double value) {
    assert(allow_macro_instructions_);
    if (value != 0.0) {
      FPRegister tmp = AppropriateTempFor(fn);
      Fmov(tmp, value);
      fcmp(fn, tmp);
    } else {
      fcmp(fn, value);
    }
  }
  void Fcsel(const FPRegister& fd,
             const FPRegister& fn,
             const FPRegister& fm,
             Condition cond) {
    assert(allow_macro_instructions_);
    assert((cond != al) && (cond != nv));
    fcsel(fd, fn, fm, cond);
  }
  void Fcvt(const FPRegister& fd, const FPRegister& fn) {
    assert(allow_macro_instructions_);
    fcvt(fd, fn);
  }
  void Fcvtms(const Register& rd, const FPRegister& fn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    fcvtms(rd, fn);
  }
  void Fcvtmu(const Register& rd, const FPRegister& fn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    fcvtmu(rd, fn);
  }
  void Fcvtns(const Register& rd, const FPRegister& fn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    fcvtns(rd, fn);
  }
  void Fcvtnu(const Register& rd, const FPRegister& fn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    fcvtnu(rd, fn);
  }
  void Fcvtzs(const Register& rd, const FPRegister& fn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    fcvtzs(rd, fn);
  }
  void Fcvtzu(const Register& rd, const FPRegister& fn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    fcvtzu(rd, fn);
  }
  void Fdiv(const FPRegister& fd, const FPRegister& fn, const FPRegister& fm) {
    assert(allow_macro_instructions_);
    fdiv(fd, fn, fm);
  }
  void Fmax(const FPRegister& fd, const FPRegister& fn, const FPRegister& fm) {
    assert(allow_macro_instructions_);
    fmax(fd, fn, fm);
  }
  void Fmin(const FPRegister& fd, const FPRegister& fn, const FPRegister& fm) {
    assert(allow_macro_instructions_);
    fmin(fd, fn, fm);
  }
  void Fmov(FPRegister fd, FPRegister fn) {
    assert(allow_macro_instructions_);
    // Only emit an instruction if fd and fn are different, and they are both D
    // registers. fmov(s0, s0) is not a no-op because it clears the top word of
    // d0. Technically, fmov(d0, d0) is not a no-op either because it clears
    // the top of q0, but FPRegister does not currently support Q registers.
    if (!fd.Is(fn) || !fd.Is64Bits()) {
      fmov(fd, fn);
    }
  }
  void Fmov(FPRegister fd, Register rn) {
    assert(allow_macro_instructions_);
    assert(!rn.IsZero());
    fmov(fd, rn);
  }
  void Fmov(FPRegister fd, double imm) {
    assert(allow_macro_instructions_);
    fmov(fd, imm);
  }
  void Fmov(Register rd, FPRegister fn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    fmov(rd, fn);
  }
  void Fmul(const FPRegister& fd, const FPRegister& fn, const FPRegister& fm) {
    assert(allow_macro_instructions_);
    fmul(fd, fn, fm);
  }
  void Fmsub(const FPRegister& fd,
             const FPRegister& fn,
             const FPRegister& fm,
             const FPRegister& fa) {
    assert(allow_macro_instructions_);
    fmsub(fd, fn, fm, fa);
  }
  void Fneg(const FPRegister& fd, const FPRegister& fn) {
    assert(allow_macro_instructions_);
    fneg(fd, fn);
  }
  void Frintn(const FPRegister& fd, const FPRegister& fn) {
    assert(allow_macro_instructions_);
    frintn(fd, fn);
  }
  void Frintz(const FPRegister& fd, const FPRegister& fn) {
    assert(allow_macro_instructions_);
    frintz(fd, fn);
  }
  void Fsqrt(const FPRegister& fd, const FPRegister& fn) {
    assert(allow_macro_instructions_);
    fsqrt(fd, fn);
  }
  void Fsub(const FPRegister& fd, const FPRegister& fn, const FPRegister& fm) {
    assert(allow_macro_instructions_);
    fsub(fd, fn, fm);
  }
  void Hint(SystemHint code) {
    assert(allow_macro_instructions_);
    hint(code);
  }
  void Hlt(int code) {
    assert(allow_macro_instructions_);
    hlt(code);
  }
  void Ldnp(const CPURegister& rt,
            const CPURegister& rt2,
            const MemOperand& src) {
    assert(allow_macro_instructions_);
    ldnp(rt, rt2, src);
  }
  void Ldp(const CPURegister& rt,
           const CPURegister& rt2,
           const MemOperand& src) {
    assert(allow_macro_instructions_);
    ldp(rt, rt2, src);
  }
  void Ldpsw(const Register& rt, const Register& rt2, const MemOperand& src) {
    assert(allow_macro_instructions_);
    ldpsw(rt, rt2, src);
  }
  void Ldr(const FPRegister& ft, double imm) {
    assert(allow_macro_instructions_);
    ldr(ft, imm);
  }
  void Ldr(const Register& rt, uint64_t imm) {
    assert(allow_macro_instructions_);
    assert(!rt.IsZero());
    ldr(rt, imm);
  }
  void Ldr(const Register& rt, Label* label) {
    ldr(rt, label);
  }
  void Lsl(const Register& rd, const Register& rn, unsigned shift) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    lsl(rd, rn, shift);
  }
  void Lsl(const Register& rd, const Register& rn, const Register& rm) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    lslv(rd, rn, rm);
  }
  void Lsr(const Register& rd, const Register& rn, unsigned shift) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    lsr(rd, rn, shift);
  }
  void Lsr(const Register& rd, const Register& rn, const Register& rm) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    lsrv(rd, rn, rm);
  }
  void Madd(const Register& rd,
            const Register& rn,
            const Register& rm,
            const Register& ra) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    assert(!ra.IsZero());
    madd(rd, rn, rm, ra);
  }
  void Mneg(const Register& rd, const Register& rn, const Register& rm) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    mneg(rd, rn, rm);
  }
  void Mov(const Register& rd, const Register& rn) {
    assert(allow_macro_instructions_);
    mov(rd, rn);
  }
  void Mrs(const Register& rt, SystemRegister sysreg) {
    assert(allow_macro_instructions_);
    assert(!rt.IsZero());
    mrs(rt, sysreg);
  }
  void Msr(SystemRegister sysreg, const Register& rt) {
    assert(allow_macro_instructions_);
    assert(!rt.IsZero());
    msr(sysreg, rt);
  }
  void Msub(const Register& rd,
            const Register& rn,
            const Register& rm,
            const Register& ra) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    assert(!ra.IsZero());
    msub(rd, rn, rm, ra);
  }
  void Mul(const Register& rd, const Register& rn, const Register& rm) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    mul(rd, rn, rm);
  }
  void Nop() {
    assert(allow_macro_instructions_);
    nop();
  }
  void Rbit(const Register& rd, const Register& rn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    rbit(rd, rn);
  }
  void Ret(const Register& xn = lr) {
    assert(allow_macro_instructions_);
    assert(!xn.IsZero());
    ret(xn);
  }
  void Rev(const Register& rd, const Register& rn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    rev(rd, rn);
  }
  void Rev16(const Register& rd, const Register& rn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    rev16(rd, rn);
  }
  void Rev32(const Register& rd, const Register& rn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    rev32(rd, rn);
  }
  void Ror(const Register& rd, const Register& rs, unsigned shift) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rs.IsZero());
    ror(rd, rs, shift);
  }
  void Ror(const Register& rd, const Register& rn, const Register& rm) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    rorv(rd, rn, rm);
  }
  void Sbfiz(const Register& rd,
             const Register& rn,
             unsigned lsb,
             unsigned width) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    sbfiz(rd, rn, lsb, width);
  }
  void Sbfx(const Register& rd,
            const Register& rn,
            unsigned lsb,
            unsigned width) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    sbfx(rd, rn, lsb, width);
  }
  void Scvtf(const FPRegister& fd, const Register& rn, unsigned fbits = 0) {
    assert(allow_macro_instructions_);
    assert(!rn.IsZero());
    scvtf(fd, rn, fbits);
  }
  void Sdiv(const Register& rd, const Register& rn, const Register& rm) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    sdiv(rd, rn, rm);
  }
  void Smaddl(const Register& rd,
              const Register& rn,
              const Register& rm,
              const Register& ra) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    assert(!ra.IsZero());
    smaddl(rd, rn, rm, ra);
  }
  void Smsubl(const Register& rd,
              const Register& rn,
              const Register& rm,
              const Register& ra) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    assert(!ra.IsZero());
    smsubl(rd, rn, rm, ra);
  }
  void Smull(const Register& rd, const Register& rn, const Register& rm) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    smull(rd, rn, rm);
  }
  void Smulh(const Register& xd, const Register& xn, const Register& xm) {
    assert(allow_macro_instructions_);
    assert(!xd.IsZero());
    assert(!xn.IsZero());
    assert(!xm.IsZero());
    smulh(xd, xn, xm);
  }
  void Stnp(const CPURegister& rt,
            const CPURegister& rt2,
            const MemOperand& dst) {
    assert(allow_macro_instructions_);
    stnp(rt, rt2, dst);
  }
  void Stp(const CPURegister& rt,
           const CPURegister& rt2,
           const MemOperand& dst) {
    assert(allow_macro_instructions_);
    stp(rt, rt2, dst);
  }
  void Sxtb(const Register& rd, const Register& rn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    sxtb(rd, rn);
  }
  void Sxth(const Register& rd, const Register& rn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    sxth(rd, rn);
  }
  void Sxtw(const Register& rd, const Register& rn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    sxtw(rd, rn);
  }
  void Tbnz(const Register& rt, unsigned bit_pos, Label* label) {
    assert(allow_macro_instructions_);
    assert(!rt.IsZero());
    tbnz(rt, bit_pos, label);
  }
  void Tbz(const Register& rt, unsigned bit_pos, Label* label) {
    assert(allow_macro_instructions_);
    assert(!rt.IsZero());
    tbz(rt, bit_pos, label);
  }
  void Ubfiz(const Register& rd,
             const Register& rn,
             unsigned lsb,
             unsigned width) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    ubfiz(rd, rn, lsb, width);
  }
  void Ubfx(const Register& rd,
            const Register& rn,
            unsigned lsb,
            unsigned width) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    ubfx(rd, rn, lsb, width);
  }
  void Ucvtf(const FPRegister& fd, const Register& rn, unsigned fbits = 0) {
    assert(allow_macro_instructions_);
    assert(!rn.IsZero());
    ucvtf(fd, rn, fbits);
  }
  void Udiv(const Register& rd, const Register& rn, const Register& rm) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    udiv(rd, rn, rm);
  }
  void Umaddl(const Register& rd,
              const Register& rn,
              const Register& rm,
              const Register& ra) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    assert(!ra.IsZero());
    umaddl(rd, rn, rm, ra);
  }
  void Umsubl(const Register& rd,
              const Register& rn,
              const Register& rm,
              const Register& ra) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    assert(!rm.IsZero());
    assert(!ra.IsZero());
    umsubl(rd, rn, rm, ra);
  }
  void Unreachable() {
    assert(allow_macro_instructions_);
#ifdef USE_SIMULATOR
    hlt(kUnreachableOpcode);
#else
    // Branch to 0 to generate a segfault.
    // lr - kInstructionSize is the address of the offending instruction.
    blr(xzr);
#endif
  }
  void Uxtb(const Register& rd, const Register& rn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    uxtb(rd, rn);
  }
  void Uxth(const Register& rd, const Register& rn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    uxth(rd, rn);
  }
  void Uxtw(const Register& rd, const Register& rn) {
    assert(allow_macro_instructions_);
    assert(!rd.IsZero());
    assert(!rn.IsZero());
    uxtw(rd, rn);
  }

  // Push the system stack pointer (sp) down to allow the same to be done to
  // the current stack pointer (according to StackPointer()). This must be
  // called _before_ accessing the memory.
  //
  // This is necessary when pushing or otherwise adding things to the stack, to
  // satisfy the AAPCS64 constraint that the memory below the system stack
  // pointer is not accessed.
  //
  // This method asserts that StackPointer() is not sp, since the call does
  // not make sense in that context.
  //
  // TODO: This method can only accept values of 'space' that can be encoded in
  // one instruction. Refer to the implementation for details.
  void BumpSystemStackPointer(const Operand& space);

#if DEBUG
  void SetAllowMacroInstructions(bool value) {
    allow_macro_instructions_ = value;
  }

  bool AllowMacroInstructions() const {
    return allow_macro_instructions_;
  }
#endif

  // Set the current stack pointer, but don't generate any code.
  // Note that this does not directly affect LastStackPointer().
  void SetStackPointer(const Register& stack_pointer) {
    assert(!AreAliased(stack_pointer, Tmp0(), Tmp1()));
    sp_ = stack_pointer;
  }

  // Return the current stack pointer, as set by SetStackPointer.
  const Register& StackPointer() const {
    return sp_;
  }

  // Set the registers used internally by the MacroAssembler as scratch
  // registers. These registers are used to implement behaviours which are not
  // directly supported by A64, and where an intermediate result is required.
  //
  // Both tmp0 and tmp1 may be set to any X register except for xzr, sp,
  // and StackPointer(). Also, they must not be the same register (though they
  // may both be NoReg).
  //
  // It is valid to set either or both of these registers to NoReg if you don't
  // want the MacroAssembler to use any scratch registers. In a debug build, the
  // Assembler will assert that any registers it uses are valid. Be aware that
  // this check is not present in release builds. If this is a problem, use the
  // Assembler directly.
  void SetScratchRegisters(const Register& tmp0, const Register& tmp1) {
    assert(!AreAliased(xzr, sp, tmp0, tmp1));
    assert(!AreAliased(StackPointer(), tmp0, tmp1));
    tmp0_ = tmp0;
    tmp1_ = tmp1;
  }

  const Register& Tmp0() const {
    return tmp0_;
  }

  const Register& Tmp1() const {
    return tmp1_;
  }

  void SetFPScratchRegister(const FPRegister& fptmp0) {
    fptmp0_ = fptmp0;
  }

  const FPRegister& FPTmp0() const {
    return fptmp0_;
  }

  const Register AppropriateTempFor(
      const Register& target,
      const CPURegister& forbidden = NoCPUReg) const {
    Register candidate = forbidden.Is(Tmp0()) ? Tmp1() : Tmp0();
    assert(!candidate.Is(target));
    return Register(candidate.code(), target.size());
  }

  const FPRegister AppropriateTempFor(
      const FPRegister& target,
      const CPURegister& forbidden = NoCPUReg) const {
    USE(forbidden);
    FPRegister candidate = FPTmp0();
    assert(!candidate.Is(forbidden));
    assert(!candidate.Is(target));
    return FPRegister(candidate.code(), target.size());
  }

  // Like printf, but print at run-time from generated code.
  //
  // The caller must ensure that arguments for floating-point placeholders
  // (such as %e, %f or %g) are FPRegisters, and that arguments for integer
  // placeholders are Registers.
  //
  // A maximum of four arguments may be given to any single Printf call. The
  // arguments must be of the same type, but they do not need to have the same
  // size.
  //
  // The following registers cannot be printed:
  //    Tmp0(), Tmp1(), StackPointer(), sp.
  //
  // This function automatically preserves caller-saved registers so that
  // calling code can use Printf at any point without having to worry about
  // corruption. The preservation mechanism generates a lot of code. If this is
  // a problem, preserve the important registers manually and then call
  // PrintfNoPreserve. Callee-saved registers are not used by Printf, and are
  // implicitly preserved.
  //
  // This function assumes (and asserts) that the current stack pointer is
  // callee-saved, not caller-saved. This is most likely the case anyway, as a
  // caller-saved stack pointer doesn't make a lot of sense.
  void Printf(const char * format,
              const CPURegister& arg0 = NoCPUReg,
              const CPURegister& arg1 = NoCPUReg,
              const CPURegister& arg2 = NoCPUReg,
              const CPURegister& arg3 = NoCPUReg);

  // Like Printf, but don't preserve any caller-saved registers, not even 'lr'.
  //
  // The return code from the system printf call will be returned in x0.
  void PrintfNoPreserve(const char * format,
                        const CPURegister& arg0 = NoCPUReg,
                        const CPURegister& arg1 = NoCPUReg,
                        const CPURegister& arg2 = NoCPUReg,
                        const CPURegister& arg3 = NoCPUReg);

  // Trace control when running the debug simulator.
  //
  // For example:
  //
  // __ Trace(LOG_REGS, TRACE_ENABLE);
  // Will add registers to the trace if it wasn't already the case.
  //
  // __ Trace(LOG_DISASM, TRACE_DISABLE);
  // Will stop logging disassembly. It has no effect if the disassembly wasn't
  // already being logged.
  void Trace(TraceParameters parameters, TraceCommand command);

  // Log the requested data independently of what is being traced.
  //
  // For example:
  //
  // __ Log(LOG_FLAGS)
  // Will output the flags.
  void Log(TraceParameters parameters);

  // Enable or disable instrumentation when an Instrument visitor is attached to
  // the simulator.
  void EnableInstrumentation();
  void DisableInstrumentation();

  // Add a marker to the instrumentation data produced by an Instrument visitor.
  // The name is a two character string that will be attached to the marker in
  // the output data.
  void AnnotateInstrumentation(const char* marker_name);

  // Pseudo-instruction that will call a function made of host machine code
  // using host calling conventions. It will take the function address from x16
  // (inter-procedural scratch reg). Arguments will be taken from simulated
  // registers in the usual sequence, and the return value will be put in
  // simulated x0. All caller-saved registers will be smashed.
  void HostCall(uint8_t argc);

 private:
  // The actual Push and Pop implementations. These don't generate any code
  // other than that required for the push or pop. This allows
  // (Push|Pop)CPURegList to bundle together setup code for a large block of
  // registers.
  //
  // Note that size is per register, and is specified in bytes.
  void PushHelper(int count, int size,
                  const CPURegister& src0, const CPURegister& src1,
                  const CPURegister& src2, const CPURegister& src3);
  void PopHelper(int count, int size,
                 const CPURegister& dst0, const CPURegister& dst1,
                 const CPURegister& dst2, const CPURegister& dst3);

  // Perform necessary maintenance operations before a push or pop.
  //
  // Note that size is per register, and is specified in bytes.
  void PrepareForPush(int count, int size);
  void PrepareForPop(int count, int size);

#if DEBUG
  // Tell whether any of the macro instruction can be used. When false the
  // MacroAssembler will assert if a method which can emit a variable number
  // of instructions is called.
  bool allow_macro_instructions_;
#endif

  // The register to use as a stack pointer for stack operations.
  Register sp_;

  // Scratch registers used internally by the MacroAssembler.
  Register tmp0_;
  Register tmp1_;
  FPRegister fptmp0_;
};


// Use this scope when you need a one-to-one mapping between methods and
// instructions. This scope prevents the MacroAssembler from being called and
// literal pools from being emitted. It also asserts the number of instructions
// emitted is what you specified when creating the scope.
class InstructionAccurateScope {
 public:
  explicit InstructionAccurateScope(MacroAssembler* masm)
      : masm_(masm), size_(0) {
    masm_->BlockLiteralPool();
#ifdef DEBUG
    old_allow_macro_instructions_ = masm_->AllowMacroInstructions();
    masm_->SetAllowMacroInstructions(false);
#endif
  }

  InstructionAccurateScope(MacroAssembler* masm, int count)
      : masm_(masm), size_(count * kInstructionSize) {
    masm_->BlockLiteralPool();
#ifdef DEBUG
    masm_->bind(&start_);
    old_allow_macro_instructions_ = masm_->AllowMacroInstructions();
    masm_->SetAllowMacroInstructions(false);
#endif
  }

  ~InstructionAccurateScope() {
    masm_->ReleaseLiteralPool();
#ifdef DEBUG
    if (start_.IsBound()) {
      assert(masm_->SizeOfCodeGeneratedSince(&start_) == size_);
    }
    masm_->SetAllowMacroInstructions(old_allow_macro_instructions_);
#endif
  }

 private:
  MacroAssembler* masm_;
  uint64_t size_;
#ifdef DEBUG
  Label start_;
  bool old_allow_macro_instructions_;
#endif
};


}  // namespace vixl

#endif  // VIXL_A64_MACRO_ASSEMBLER_A64_H_
