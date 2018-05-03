/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 2018 Intel Corporation                                 |
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
#ifndef incl_HPHP_UTIL_ASM_X64_INTELXED_H_
#define incl_HPHP_UTIL_ASM_X64_INTELXED_H_

extern "C" {
#include <xed-interface.h>
}

#include <functional>
#include <tbb/concurrent_unordered_map.h>

/*
 * A macro assembler for x64, based on the Intel XED library, that strives
 * for low coupling to the runtime environment and ease of extendability.
 */
namespace HPHP { namespace jit {

struct XedInit {
  XedInit() {
    xed_tables_init();
  }
};

///////////////////////////////////////////////////////////////////////////////

typedef bool(*immFitFunc)(int64_t, int);

///////////////////////////////////////////////////////////////////////////////

struct XedAssembler final : public X64AssemblerBase {
private:
  static constexpr xed_state_t kXedState = {
    XED_MACHINE_MODE_LONG_64,
    XED_ADDRESS_WIDTH_64b
  };

  CodeAddress dest() const {
    codeBlock.assertCanEmit(XED_MAX_INSTRUCTION_BYTES);
    return codeBlock.toDestAddress(codeBlock.frontier());
  }

  static constexpr auto nullrip = RIPRelativeRef(DispRIP(0));
  static constexpr auto immfitSigned = deltaFits;
  static constexpr auto immfitUnsigned = (immFitFunc)magFits;

public:
  explicit XedAssembler(CodeBlock& cb) : X64AssemblerBase(cb) {}

  XedAssembler(const XedAssembler&) = delete;
  XedAssembler& operator=(const XedAssembler&) = delete;
  /*
   * The following section defines the main interface for emitting
   * x64.
   *
   * Simple Examples:
   *
   *   a.  movq   (rax, rbx);       // order is AT&T: src, dest
   *   a.  loadq  (*rax, rbx);      // loads from *rax
   *   a.  loadq  (rax[0], rbx);    // also loads from *rax
   *   a.  storeq (rcx, rax[0xc]);  // store to rax + 0xc
   *   a.  addq   (0x1, rbx);       // increment rbx
   *
   * Addressing with index registers:
   *
   *   a.  movl   (index, ecx);
   *   a.  loadq  (*rax, rbx);
   *   a.  storeq (rbx, rbx[rcx*8]);
   *   a.  call   (rax);            // indirect call
   *
   */

#define BYTE_LOAD_OP(name, instr)                                     \
  void name##b(MemoryRef m, Reg8 r)     { xedInstrMR(instr, m, r); }

#define LOAD_OP(name, instr)                                          \
  void name##q(MemoryRef m, Reg64 r) { xedInstrMR(instr, m, r); }     \
  void name##l(MemoryRef m, Reg32 r) { xedInstrMR(instr, m, r); }     \
  void name##w(MemoryRef m, Reg16 r) { xedInstrMR(instr, m, r); }     \
  void name##q(RIPRelativeRef m, Reg64 r) { xedInstrMR(instr, m, r); }\
  BYTE_LOAD_OP(name, instr)

#define BYTE_STORE_OP(name, instr)                                    \
  void name##b(Reg8 r, MemoryRef m)  { xedInstrRM(instr, r, m); }     \
  void name##b(Immed i, MemoryRef m) { xedInstrIM(instr, i, m,        \
                                                  sz::byte); }

#define STORE_OP(name, instr)                                         \
  void name##w(Immed i, MemoryRef m) {                                \
    xedInstrIM(instr, i, m, IMMPROP(sz::word,                         \
                                    sz::word | sz::byte,              \
                                    immfitSigned), sz::word);         \
  }                                                                   \
  void name##l(Immed i, MemoryRef m) {                                \
    xedInstrIM(instr, i, m, IMMPROP(sz::dword,                        \
                                    sz::dword | sz::byte,             \
                                    immfitSigned), sz::dword);        \
  }                                                                   \
  void name##w(Reg16 r, MemoryRef m) { xedInstrRM(instr, r, m); }     \
  void name##l(Reg32 r, MemoryRef m) { xedInstrRM(instr, r, m); }     \
  void name##q(Reg64 r, MemoryRef m) { xedInstrRM(instr, r, m); }     \
  BYTE_STORE_OP(name, instr)

#define BYTE_REG_OP(name, instr)                                      \
  void name##b(Reg8 r1, Reg8 r2) { xedInstrRR(instr, r1, r2);}        \
  void name##b(Immed i, Reg8 r)  { xedInstrIR(instr, i, r); }

#define REG_OP(name, instr)                                           \
  void name##q(Reg64 r1, Reg64 r2)   { xedInstrRR(instr, r1, r2); }   \
  void name##l(Reg32 r1, Reg32 r2)   { xedInstrRR(instr, r1, r2); }   \
  void name##w(Reg16 r1, Reg16 r2)   { xedInstrRR(instr, r1, r2); }   \
  void name##l(Immed i, Reg32 r) {                                    \
    xedInstrIR(instr, i, r, IMMPROP(sz::dword,                        \
                                    sz::dword | sz::byte,             \
                                    immfitSigned));                   \
  }                                                                   \
  void name##w(Immed i, Reg16 r) {                                    \
    xedInstrIR(instr, i, r, IMMPROP(sz::word,                         \
                                    sz::word | sz::byte,              \
                                    immfitSigned));                   \
  }                                                                   \
  BYTE_REG_OP(name, instr)

#define IMM64_STORE_OP(name, instr)                                   \
  void name##q(Immed i, MemoryRef m) {                                \
    xedInstrIM(instr, i, m, IMMPROP(sz::dword,                        \
                                    sz::dword | sz::byte,             \
                                    immfitSigned), sz::qword);        \
  }

#define IMM64R_OP(name, instr)                                        \
  void name##q(Immed imm, Reg64 r) {                                  \
    always_assert(imm.fits(sz::dword));                               \
    xedInstrIR(instr, imm, r, IMMPROP(sz::dword,                      \
                                      sz::dword | sz::byte,           \
                                      immfitSigned));                 \
  }

#define FULL_OP(name, instr)                                          \
  LOAD_OP(name, instr)                                                \
  STORE_OP(name, instr)                                               \
  REG_OP(name, instr)                                                 \
  IMM64_STORE_OP(name, instr)                                         \
  IMM64R_OP(name, instr)

  // We rename x64's mov to store and load for improved code
  // readability.
#define IMMPROP(size, allsizes, func) size
  LOAD_OP        (load, XED_ICLASS_MOV)
  STORE_OP       (store,XED_ICLASS_MOV)
  IMM64_STORE_OP (store,XED_ICLASS_MOV)
  REG_OP         (mov,  XED_ICLASS_MOV)
  FULL_OP        (test, XED_ICLASS_TEST)
#undef IMMPROP

#define IMMPROP(size, allsizes, func) allsizes, func
  FULL_OP(add, XED_ICLASS_ADD)
  FULL_OP(xor, XED_ICLASS_XOR)
  FULL_OP(sub, XED_ICLASS_SUB)
  FULL_OP(and, XED_ICLASS_AND)
  FULL_OP(or,  XED_ICLASS_OR)
  FULL_OP(cmp, XED_ICLASS_CMP)
  FULL_OP(sbb, XED_ICLASS_SBB)
#undef IMMPROP

#undef IMM64R_OP
#undef FULL_OP
#undef REG_OP
#undef STORE_OP
#undef LOAD_OP
#undef BYTE_LOAD_OP
#undef BYTE_STORE_OP
#undef BYTE_REG_OP
#undef IMM64_STORE_OP

  // 64-bit immediates work with mov to a register.
  void movq(Immed64 imm, Reg64 r) { xedInstrIR(XED_ICLASS_MOV, imm, r,
                                               sz::qword | sz::dword,
                                               immfitSigned); }

  // movzbx is a special snowflake. We don't have movzbq because it behaves
  // exactly the same as movzbl but takes an extra byte.
  void loadzbl(MemoryRef m, Reg32 r)        { xedInstrMR(XED_ICLASS_MOVZX,
                                                         m, r, sz::byte); }
  void movzbl(Reg8 src, Reg32 dest)         { xedInstrRR(XED_ICLASS_MOVZX,
                                                         src, dest); }
  void movsbl(Reg8 src, Reg32 dest)         { xedInstrRR(XED_ICLASS_MOVSX,
                                                         src, dest); }
  void movzwl(Reg16 src, Reg32 dest)        { xedInstrRR(XED_ICLASS_MOVZX,
                                                         src, dest); }

  void loadsbq(MemoryRef m, Reg64 r)        { xedInstrMR(XED_ICLASS_MOVSX,
                                                         m, r); }
  void movsbq(Reg8 src, Reg64 dest)         { xedInstrRR(XED_ICLASS_MOVSX,
                                                         src, dest); }

  void lea(MemoryRef p, Reg64 reg)       { xedInstrMR(XED_ICLASS_LEA, p, reg); }
  void lea(RIPRelativeRef p, Reg64 reg)  { xedInstrMR(XED_ICLASS_LEA, p, reg); }

  void xchgq(Reg64 r1, Reg64 r2) { xedInstrRR(XED_ICLASS_XCHG, r1, r2); }
  void xchgl(Reg32 r1, Reg32 r2) { xedInstrRR(XED_ICLASS_XCHG, r1, r2); }
  void xchgb(Reg8 r1, Reg8 r2)   { xedInstrRR(XED_ICLASS_XCHG, r1, r2); }

  void imul(Reg64 r1, Reg64 r2)  { xedInstrRR(XED_ICLASS_IMUL, r1, r2); }

  void push(Reg64 r)  { xedInstrR(XED_ICLASS_PUSH,  r); }
  void pushl(Reg32 r) { xedInstrR(XED_ICLASS_PUSH,  r); }
  void pop (Reg64 r)  { xedInstrR(XED_ICLASS_POP,   r); }
  void idiv(Reg64 r)  { xedInstrR(XED_ICLASS_IDIV,  r); }
  void incq(Reg64 r)  { xedInstrR(XED_ICLASS_INC,   r); }
  void incl(Reg32 r)  { xedInstrR(XED_ICLASS_INC,   r); }
  void incw(Reg16 r)  { xedInstrR(XED_ICLASS_INC,   r); }
  void decq(Reg64 r)  { xedInstrR(XED_ICLASS_DEC,   r); }
  void decl(Reg32 r)  { xedInstrR(XED_ICLASS_DEC,   r); }
  void decw(Reg16 r)  { xedInstrR(XED_ICLASS_DEC,   r); }
  void notb(Reg8 r)   { xedInstrR(XED_ICLASS_NOT,   r); }
  void not(Reg64 r)   { xedInstrR(XED_ICLASS_NOT,   r); }
  void neg(Reg64 r)   { xedInstrR(XED_ICLASS_NEG,   r); }
  void negb(Reg8 r)   { xedInstrR(XED_ICLASS_NEG,   r); }
  void ret()          { xedInstr(XED_ICLASS_RET_NEAR); }
  void ret(Immed i)   { xedInstrI(XED_ICLASS_IRET, i,
                                                    sz::word); }
  void cqo()          { xedInstr(XED_ICLASS_CQO); }
  void nop()          { xedInstr(XED_ICLASS_NOP,    sz::byte); }
  void int3()         { xedInstr(XED_ICLASS_INT3,   sz::byte); }
  void ud2()          { xedInstr(XED_ICLASS_UD2,    sz::byte); }
  void pushf()        { xedInstr(XED_ICLASS_PUSHF,  sz::word); }
  void popf()         { xedInstr(XED_ICLASS_POPF,   sz::word); }
  void lock()         { always_assert(false); }

  void push(MemoryRef m)      { xedInstrM(XED_ICLASS_PUSH, m); }
  void pop (MemoryRef m)      { xedInstrM(XED_ICLASS_POP, m); }
  void incq(MemoryRef m)      { xedInstrM(XED_ICLASS_INC, m); }
  void incl(MemoryRef m)      { xedInstrM(XED_ICLASS_INC, m, sz::dword); }
  void incw(MemoryRef m)      { xedInstrM(XED_ICLASS_INC, m, sz::word); }
  void decqlock(MemoryRef m)  { xedInstrM(XED_ICLASS_DEC_LOCK, m); }
  void decq(MemoryRef m)      { xedInstrM(XED_ICLASS_DEC, m); }
  void decl(MemoryRef m)      { xedInstrM(XED_ICLASS_DEC, m, sz::dword); }
  void decw(MemoryRef m)      { xedInstrM(XED_ICLASS_DEC, m, sz::word); }

  //special case for push(imm)
  void push(Immed64 i) {
    xed_encoder_operand_t op = toXedOperand(i, sz::byte | sz::word | sz::dword,
                                            immfitSigned);
    xedEmit(XED_ICLASS_PUSH, op, op.width_bits < 32 ? 16 : 64);
  }

  void movups(RegXMM x, MemoryRef m)        { xedInstrRM(XED_ICLASS_MOVUPS,
                                                         x, m, sz::qword * 2); }
  void movups(MemoryRef m, RegXMM x)        { xedInstrMR(XED_ICLASS_MOVUPS,
                                                         m, x, sz::qword * 2); }
  void movdqu(RegXMM x, MemoryRef m)        { xedInstrRM(XED_ICLASS_MOVDQU,
                                                         x, m); }
  void movdqu(MemoryRef m, RegXMM x)        { xedInstrMR(XED_ICLASS_MOVDQU,
                                                         m, x); }
  void movdqa(RegXMM x, RegXMM y)           { xedInstrRR(XED_ICLASS_MOVDQA,
                                                         y, x); }
  void movdqa(RegXMM x, MemoryRef m)        { xedInstrRM(XED_ICLASS_MOVDQA,
                                                         x, m); }
  void movdqa(MemoryRef m, RegXMM x)        { xedInstrMR(XED_ICLASS_MOVDQA,
                                                         m, x); }
  void movsd (RegXMM x, RegXMM y)           { xedInstrRR(XED_ICLASS_MOVSD_XMM,
                                                         y, x); }
  void movsd (RegXMM x, MemoryRef m)        { xedInstrRM(XED_ICLASS_MOVSD_XMM,
                                                         x, m); }
  void movsd (MemoryRef m, RegXMM x)        { xedInstrMR(XED_ICLASS_MOVSD_XMM,
                                                         m, x); }
  void movsd (RIPRelativeRef m, RegXMM x)   { xedInstrMR(XED_ICLASS_MOVSD_XMM,
                                                         m, x); }
  void lddqu (MemoryRef m, RegXMM x)        { xedInstrMR(XED_ICLASS_LDDQU,
                                                         m, x); }
  void unpcklpd(RegXMM s, RegXMM d)         { xedInstrRR(XED_ICLASS_UNPCKLPD,
                                                         d, s); }

  void rorq  (Immed i, Reg64 r) { xedInstrIR(XED_ICLASS_ROR, i, r, sz::byte); }
  void shlq  (Immed i, Reg64 r) { xedInstrIR(XED_ICLASS_SHL, i, r, sz::byte); }
  void shrq  (Immed i, Reg64 r) { xedInstrIR(XED_ICLASS_SHR, i, r, sz::byte); }
  void sarq  (Immed i, Reg64 r) { xedInstrIR(XED_ICLASS_SAR, i, r, sz::byte); }
  void shll  (Immed i, Reg32 r) { xedInstrIR(XED_ICLASS_SHL, i, r, sz::byte); }
  void shrl  (Immed i, Reg32 r) { xedInstrIR(XED_ICLASS_SHR, i, r, sz::byte); }
  void shlw  (Immed i, Reg16 r) { xedInstrIR(XED_ICLASS_SHL, i, r, sz::byte); }
  void shrw  (Immed i, Reg16 r) { xedInstrIR(XED_ICLASS_SHR, i, r, sz::byte); }

  void shlq (Reg64 r) { xedInstrRR_CL(XED_ICLASS_SHL, r); }
  void shrq (Reg64 r) { xedInstrRR_CL(XED_ICLASS_SHR, r); }
  void sarq (Reg64 r) { xedInstrRR_CL(XED_ICLASS_SAR, r); }

  void roundsd (RoundDirection d, RegXMM src, RegXMM dst) {
    Immed i((int)d);
    xedInstrIRR(XED_ICLASS_ROUNDSD, dst, src, i, sz::byte);
  }

  void cmpsd(RegXMM src, RegXMM dst, ComparisonPred pred) {
    Immed i((int)pred);
    xedInstrIRR(XED_ICLASS_CMPSD_XMM, dst, src, i, sz::byte);
  }

  /*
   * Control-flow directives.  Primitive labeling/patching facilities
   * are available, as well as slightly higher-level ones via the
   * Label class.
   */

  void jmp(Reg64 r)            { xedInstrR(XED_ICLASS_JMP,        r); }
  void jmp(MemoryRef m)        { xedInstrM(XED_ICLASS_JMP,        m); }
  void jmp(RIPRelativeRef m)   { xedInstrM(XED_ICLASS_JMP,        m); }
  void call(Reg64 r)           { xedInstrR(XED_ICLASS_CALL_NEAR,  r); }
  void call(MemoryRef m)       { xedInstrM(XED_ICLASS_CALL_NEAR,  m); }
  void call(RIPRelativeRef m)  { xedInstrM(XED_ICLASS_CALL_NEAR,  m); }

  void jmp8(CodeAddress dest)  { xedInstrRelBr(XED_ICLASS_JMP,
                                               dest, sz::byte); }

  void jmp(CodeAddress dest) {
    xedInstrRelBr(XED_ICLASS_JMP, dest, sz::dword);
  }

  void call(CodeAddress dest) {
    xedInstrRelBr(XED_ICLASS_CALL_NEAR, dest, sz::dword);
  }

  void jcc(ConditionCode cond, CodeAddress dest) {
    xedInstrRelBr(ccToXedJump(cond), dest, sz::dword);
  }

  void jcc8(ConditionCode cond, CodeAddress dest) {
    xedInstrRelBr(ccToXedJump(cond), dest, sz::byte);
  }

  using X64AssemblerBase::call;
  using X64AssemblerBase::jmp;
  using X64AssemblerBase::jmp8;
  using X64AssemblerBase::jcc;
  using X64AssemblerBase::jcc8;

  void setcc(int cc, Reg8 byteReg) {
    xedInstrR(ccToXedSetCC(cc), byteReg);
  }

  void psllq(Immed i, RegXMM r) { xedInstrIR(XED_ICLASS_PSLLQ, i, r,
                                             sz::byte); }
  void psrlq(Immed i, RegXMM r) { xedInstrIR(XED_ICLASS_PSRLQ, i, r,
                                             sz::byte); }

  void movq_rx(Reg64 rsrc, RegXMM rdest) {
    xedInstrRR(XED_ICLASS_MOVQ, rsrc, rdest);
  }
  void movq_xr(RegXMM rsrc, Reg64 rdest) {
    xedInstrRR(XED_ICLASS_MOVQ, rsrc, rdest);
  }

  void addsd(RegXMM src, RegXMM srcdest) {
    xedInstrRR(XED_ICLASS_ADDSD, srcdest, src);
  }
  void mulsd(RegXMM src, RegXMM srcdest) {
    xedInstrRR(XED_ICLASS_MULSD, srcdest, src);
  }
  void subsd(RegXMM src, RegXMM srcdest) {
    xedInstrRR(XED_ICLASS_SUBSD, srcdest, src);
  }
  void pxor(RegXMM src, RegXMM srcdest) {
    xedInstrRR(XED_ICLASS_PXOR, srcdest, src);
  }
  void cvtsi2sd(Reg64 src, RegXMM dest) {
    xedInstrRR(XED_ICLASS_CVTSI2SD, src, dest);
  }
  void cvtsi2sd(MemoryRef m, RegXMM dest) {
    xedInstrMR(XED_ICLASS_CVTSI2SD, m, dest);
  }
  void ucomisd(RegXMM l, RegXMM r) {
    xedInstrRR(XED_ICLASS_UCOMISD, l, r);
  }
  void sqrtsd(RegXMM src, RegXMM dest) {
    xedInstrRR(XED_ICLASS_SQRTSD, dest, src);
  }

  void divsd(RegXMM src, RegXMM srcdest) {
    xedInstrRR(XED_ICLASS_DIVSD, srcdest, src);
  }
  void cvttsd2siq(RegXMM src, Reg64 dest) {
    xedInstrRR(XED_ICLASS_CVTTSD2SI, src, dest);
  }

private:
  // XED conditional jump conversion functions
#define CC_TO_XED_ARRAY(xed_instr) {                            \
    XED_ICLASS_##xed_instr##O,    /*CC_O                  */    \
    XED_ICLASS_##xed_instr##NO,   /*CC_NO                 */    \
    XED_ICLASS_##xed_instr##B,    /*CC_B, CC_NAE          */    \
    XED_ICLASS_##xed_instr##NB,   /*CC_AE, CC_NB, CC_NC   */    \
    XED_ICLASS_##xed_instr##Z,    /*CC_E, CC_Z            */    \
    XED_ICLASS_##xed_instr##NZ,   /*CC_NE, CC_NZ          */    \
    XED_ICLASS_##xed_instr##BE,   /*CC_BE, CC_NA          */    \
    XED_ICLASS_##xed_instr##NBE,  /*CC_A, CC_NBE          */    \
    XED_ICLASS_##xed_instr##S,    /*CC_S                  */    \
    XED_ICLASS_##xed_instr##NS,   /*CC_NS                 */    \
    XED_ICLASS_##xed_instr##P,    /*CC_P                  */    \
    XED_ICLASS_##xed_instr##NP,   /*CC_NP                 */    \
    XED_ICLASS_##xed_instr##L,    /*CC_L, CC_NGE          */    \
    XED_ICLASS_##xed_instr##NL,   /*CC_GE, CC_NL          */    \
    XED_ICLASS_##xed_instr##LE,   /*CC_LE, CC_NG          */    \
    XED_ICLASS_##xed_instr##NLE   /*CC_G, CC_NLE          */    \
  }

  ALWAYS_INLINE
  xed_iclass_enum_t ccToXedJump(ConditionCode c) {
    assertx(c != CC_None);
    static const xed_iclass_enum_t jumps[] = CC_TO_XED_ARRAY(J);
    return jumps[(int)c];
  }

  ALWAYS_INLINE
  xed_iclass_enum_t ccToXedSetCC(int c) {
    assertx(c != -1);
    static const xed_iclass_enum_t setccs[] = CC_TO_XED_ARRAY(SET);
    return setccs[c];
  }

  ALWAYS_INLINE
  xed_iclass_enum_t ccToXedCMov(ConditionCode c) {
    assertx(c != CC_None);
    static const xed_iclass_enum_t cmovs[] = CC_TO_XED_ARRAY(CMOV);
    return cmovs[(int)c];
  }

  // XED emit functions

  ALWAYS_INLINE
  uint32_t xedEmitImpl(xed_iclass_enum_t instr, CodeAddress destination,
                       std::function<void(xed_encoder_instruction_t*)>
                                          xedFunc) {
    xed_encoder_instruction_t instruction;
    xed_encoder_request_t request;
    uint32_t encodedSize = 0;
    xed_error_enum_t xedError;
    xed_bool_t convert_ok;

    xedFunc(&instruction);

    xed_encoder_request_zero(&request);
    convert_ok = xed_convert_to_encoder_request(&request, &instruction);
    always_assert(convert_ok && "Unable to convert instruction"
                  " to encoder request");
    xedError = xed_encode(&request, destination, XED_MAX_INSTRUCTION_BYTES,
                          &encodedSize);
    always_assert_flog(xedError == XED_ERROR_NONE,
                       "XED: Error when encoding {}(): {}",
                       xed_iclass_enum_t2str(instr),
                       xed_error_enum_t2str(xedError));
    return encodedSize;
  }

  ALWAYS_INLINE
  uint32_t xedEmit(xed_iclass_enum_t instr,
                   xed_uint_t effOperandSizeBits,
                   CodeAddress destination) {
    return xedEmitImpl(instr, destination,
                        [&](xed_encoder_instruction_t* i) {
                          xed_inst0(i, kXedState, instr, effOperandSizeBits);
                        });
  }

  ALWAYS_INLINE
  void xedEmit(xed_iclass_enum_t instr, xed_uint_t effOperandSizeBits = 0) {
    codeBlock.moveFrontier(xedEmit(instr, effOperandSizeBits, dest()));
  }

  ALWAYS_INLINE
  uint32_t xedEmit(xed_iclass_enum_t instr,
                   const xed_encoder_operand_t& op,
                   xed_uint_t effOperandSizeBits,
                   CodeAddress destination) {
    return xedEmitImpl(instr, destination,
                        [&](xed_encoder_instruction_t* i) {
                          xed_inst1(i, kXedState, instr,
                                    effOperandSizeBits, op);
                        });
  }

  ALWAYS_INLINE
  void xedEmit(xed_iclass_enum_t instr, const xed_encoder_operand_t& op,
               xed_uint_t effOperandSizeBits = 0) {
    codeBlock.moveFrontier(xedEmit(instr, op, effOperandSizeBits, dest()));
  }

  ALWAYS_INLINE
  uint32_t xedEmit(xed_iclass_enum_t instr,
                   const xed_encoder_operand_t& op_1,
                   const xed_encoder_operand_t& op_2,
                   xed_uint_t effOperandSizeBits,
                   CodeAddress destination) {
    return xedEmitImpl(instr, destination,
                        [&](xed_encoder_instruction_t* i) {
                          xed_inst2(i, kXedState, instr,
                                    effOperandSizeBits, op_1, op_2);
                        });
  }

  ALWAYS_INLINE
  void xedEmit(xed_iclass_enum_t instr,
               const xed_encoder_operand_t& op_1,
               const xed_encoder_operand_t& op_2,
               xed_uint_t effOperandSizeBits = 0) {
    codeBlock.moveFrontier(xedEmit(instr, op_1, op_2, effOperandSizeBits,
                                   dest()));
  }

  ALWAYS_INLINE
  uint32_t xedEmit(xed_iclass_enum_t instr,
                   const xed_encoder_operand_t& op_1,
                   const xed_encoder_operand_t& op_2,
                   const xed_encoder_operand_t& op_3,
                   xed_uint_t effOperandSizeBits,
                   CodeAddress destination) {
    return xedEmitImpl(instr, destination,
                        [&](xed_encoder_instruction_t* i) {
                          xed_inst3(i, kXedState, instr,
                                    effOperandSizeBits, op_1, op_2, op_3);
                        });
  }

  ALWAYS_INLINE
  void xedEmit(xed_iclass_enum_t instr,
               const xed_encoder_operand_t& op_1,
               const xed_encoder_operand_t& op_2,
               const xed_encoder_operand_t& op_3,
               xed_uint_t effOperandSizeBits = 0) {
    codeBlock.moveFrontier(xedEmit(instr, op_1, op_2, op_3, effOperandSizeBits,
                                   dest()));
  }

public:
  static constexpr auto kInt3Size = sz::byte;
  static constexpr auto kUd2Size = sz::word;

  void emitInt3s(int n) {
    static uint8_t instr = 0;
    if (n == 0) return;
    if (instr == 0) {
      xedEmit(XED_ICLASS_INT3, sz::byte, &instr);
    }
    for (auto i = 0; i < n; ++i) {
      byte(instr);
    }
  }

  void emitNop(int n) {
    if (n == 0) return;
    static const xed_iclass_enum_t nops[] = {
      XED_ICLASS_INVALID,
      XED_ICLASS_NOP,
      XED_ICLASS_NOP2,
      XED_ICLASS_NOP3,
      XED_ICLASS_NOP4,
      XED_ICLASS_NOP5,
      XED_ICLASS_NOP6,
      XED_ICLASS_NOP7,
      XED_ICLASS_NOP8,
      XED_ICLASS_NOP9,
    };
    // While n >= 9, emit 9 byte NOPs
    while (n >= 9) {
      xedInstr(XED_ICLASS_NOP9, sz::nosize);
      n -= 9;
    }
    // Emit remaining NOPs (if any)
    if (n) {
      xedInstr(nops[n], sz::nosize);
    }
  }

  void pad() {
    auto remaining = available();
    static uint8_t padInstructs[kUd2Size + kInt3Size] = {0};
    if (padInstructs[0] == 0) {
      xedEmit(XED_ICLASS_UD2, sz::nosize, padInstructs);
      xedEmit(XED_ICLASS_INT3, sz::nosize, padInstructs + kUd2Size);
    }
    while (remaining >= kUd2Size) {
      bytes(kUd2Size, padInstructs);
      remaining -= kUd2Size;
    }
    while (remaining >= kInt3Size) {
      byte(padInstructs[kUd2Size]);
      remaining -= kInt3Size;
    }
  }

  ALWAYS_INLINE
  XedAssembler& prefix(const MemoryRef& mr) {
    return *this;
  }

public:
  /*
   * The following functions use a naming convention for an older API
   * to the assembler; conditional loads and moves haven't yet been
   * ported.
   */

  // CMOVcc [rbase + off], rdest
  inline void cload_reg64_disp_reg64(ConditionCode cc, Reg64 rbase,
                                     int off, Reg64 rdest) {
    MemoryRef m(DispReg(rbase, off));
    xedInstrMR(ccToXedCMov(cc), m, rdest);
  }
  inline void cload_reg64_disp_reg32(ConditionCode cc, Reg64 rbase,
                                     int off, Reg32 rdest) {
    MemoryRef m(DispReg(rbase, off));
    xedInstrMR(ccToXedCMov(cc), m, rdest);
  }
  inline void cmov_reg64_reg64(ConditionCode cc, Reg64 rsrc, Reg64 rdest) {
    xedInstrRR(ccToXedCMov(cc), rsrc, rdest);
  }

private:
  /*
   * The following section contains conversion methods that take a Reg8/32/64,
   * RegXMM, MemoryRef, RipRelative struct and convert it to a
   * xed_encoder_operand_t.
   */

  static constexpr int bytesToBits(int sz) {
    return (sz << 3);
  }

  static constexpr int bitsToBytes(int bits) {
    return (bits >> 3);
  }

  union XedImmValue {
    int8_t    b;
    uint8_t   ub;
    int16_t   w;
    int32_t   l;
    int64_t   q;
    uint64_t  uq;

    template<typename immtype>
    XedImmValue(const immtype& imm, int immSize) {
      uq = 0;
      switch (immSize) {
        case sz::byte:
          b = imm.b(); break;
        case sz::word:
          w = imm.w(); break;
        case sz::dword:
          l = imm.l(); break;
        case sz::qword:
          q = imm.q(); break;
      }
    }
  };

  xed_reg_enum_t xedFromReg(const Reg64& reg) {
    return xed_reg_enum_t(int(reg) + XED_REG_RAX);
  }

  xed_reg_enum_t xedFromReg(const Reg32& reg) {
    return xed_reg_enum_t(int(reg) + XED_REG_EAX);
  }

  xed_reg_enum_t xedFromReg(const Reg16& reg) {
    return xed_reg_enum_t(int(reg) + XED_REG_AX);
  }

  xed_reg_enum_t xedFromReg(const Reg8& reg) {
    auto regid = int(reg);
    if ((regid & 0x80) == 0) {
      return xed_reg_enum_t(regid + XED_REG_AL);
    }
    return xed_reg_enum_t((regid - 0x84) + XED_REG_AH);
  }

  xed_reg_enum_t xedFromReg(const RegXMM& reg) {
    return xed_reg_enum_t(int(reg) + XED_REG_XMM0);
  }

  int getDisplSize(intptr_t value) {
    if (value == 0) return sz::nosize;
    return deltaFits(value, sz::byte) ? sz::byte : sz::dword;
  }

  xed_enc_displacement_t xedDispFromValue(intptr_t value, int size) {
    switch (size) {
      case sz::nosize:  return {0, 0};
      case sz::byte:    return {(xed_uint64_t)safe_cast<int8_t>(value),
                                (xed_uint32_t)bytesToBits(size)};
      default:          return {(xed_uint64_t)safe_cast<int32_t>(value),
                                (xed_uint32_t)bytesToBits(size)};
    }
  }

  xed_enc_displacement_t xedDispFromValue(intptr_t value) {
    return xedDispFromValue(value, getDisplSize(value));
  }

  template<typename regtype>
  xed_encoder_operand_t toXedOperand(const regtype& reg) {
    return xed_reg(xedFromReg(reg));
  }

  xed_encoder_operand_t toXedOperand(xed_reg_enum_t reg) {
    return xed_reg(reg);
  }

  xed_encoder_operand_t toXedOperand(const MemoryRef& m, int memSize) {
    static const xed_reg_enum_t segmentRegs[] = {
      XED_REG_INVALID, //Segment::DS (no segment register override)
      XED_REG_FS,      //Segment::FS
      XED_REG_GS       //Segment::GS
    };
    xed_reg_enum_t base = int(m.r.base) != -1 ?
                          xedFromReg(m.r.base) : XED_REG_INVALID;
    xed_reg_enum_t index = int(m.r.index) != -1 ?
                           xedFromReg(m.r.index) : XED_REG_INVALID;
    return xed_mem_gbisd(segmentRegs[int(m.segment)],
                         base, index, m.r.scale,
                         xedDispFromValue(m.r.disp), bytesToBits(memSize));
  }

  xed_encoder_operand_t toXedOperand(const RIPRelativeRef& r, int memSize) {
    return xed_mem_bd(XED_REG_RIP, xedDispFromValue(r.r.disp, sz::dword),
                      bytesToBits(memSize));
  }

  template<typename immtype>
  xed_encoder_operand_t toXedOperand(const immtype& immed, int immSize) {
    return xed_imm0(XedImmValue(immed, immSize).uq, bytesToBits(immSize));
  }

  xed_encoder_operand_t toXedOperand(CodeAddress address, int size) {
    return xed_relbr(safe_cast<int32_t>((int64_t)address), bytesToBits(size));
  }

  template<typename immtype>
  xed_encoder_operand_t toXedOperand(const immtype& immed,
                                     int immSizes, immFitFunc func) {
    immSizes = reduceImmSize(immed.q(), immSizes, func);
    return xed_imm0(XedImmValue(immed, immSizes).uq, bytesToBits(immSizes));
  }

  int reduceImmSize(int64_t value, int allowedSizes, immFitFunc func) {
    for (int crtSize = sz::byte; crtSize < sz::qword; crtSize <<= 1) {
      if ((allowedSizes & crtSize) && (*func)(value, crtSize)) return crtSize;
    }
    assertx((allowedSizes & sz::qword) &&
            "Could not find an optimal size for Immed");
    return sz::qword;
  }

  /*
   * Cache sizes for instruction types in a certain xedInstr context.
   * This helps with emitting instructions where you need to know in advance
   * the length of the instruction being emitted (such as when one of
   * the operands is a RIPRelativeRef) by caching the size of the instruction
   * and removing the need to call xedEmit twice each time (once to get
   * the size, and once to actually emit the instruction).
   */
  typedef tbb::concurrent_unordered_map<int32_t, uint32_t> XedLenCache;

  ALWAYS_INLINE
  uint32_t xedCacheLen(XedLenCache* lenCache,
                       std::function<uint32_t(void)> xedFunc, uint32_t key) {
    auto res = lenCache->find(key);
    if (res != lenCache->end()) {
      return res->second;
    }
    auto instrLen = xedFunc();
    lenCache->insert({key, instrLen});
    return instrLen;
  }

  static constexpr uint32_t xedLenCacheKey(xed_iclass_enum_t instr,
                                           uint32_t size) {
    // 16 bits should fit a xed_iclass_enum_t value (there are currently ~1560
    // distinct values).
    return uint32_t(instr) | (size << 16);
  }

// XEDInstr* wrappers

#define XED_WRAP_IMPL() \
  XED_WRAP_X(64)        \
  XED_WRAP_X(32)        \
  XED_WRAP_X(16)        \
  XED_WRAP_X(8)

  // instr(reg)

#define XED_INSTR_WRAPPER_IMPL(bitsize)                             \
  ALWAYS_INLINE                                                     \
  void xedInstrR(xed_iclass_enum_t instr, const Reg##bitsize& r) {  \
    xedEmit(instr, toXedOperand(r), bitsize);                       \
  }

#define XED_WRAP_X XED_INSTR_WRAPPER_IMPL
  XED_WRAP_IMPL()
#undef XED_WRAP_X

  // instr(imm, reg)

#define XED_INSTIR_WRAPPER_IMPL(bitsize)                            \
  ALWAYS_INLINE                                                     \
  void xedInstrIR(xed_iclass_enum_t instr, const Immed& i,          \
                  const Reg##bitsize& r,                            \
                  int immSize = bitsToBytes(bitsize)) {             \
    xedEmit(instr, toXedOperand(r), toXedOperand(i, immSize),       \
            bitsize);                                               \
  }                                                                 \
                                                                    \
  ALWAYS_INLINE                                                     \
  void xedInstrIR(xed_iclass_enum_t instr, const Immed& i,          \
                  const Reg##bitsize& r,                            \
                  int immSize, immFitFunc fitFunc) {                \
    xedEmit(instr, toXedOperand(r),                                 \
            toXedOperand(i, immSize, fitFunc), bitsize);            \
  }

#define XED_WRAP_X XED_INSTIR_WRAPPER_IMPL
  XED_WRAP_IMPL()
#undef XED_WRAP_X

  ALWAYS_INLINE
  void xedInstrIR(xed_iclass_enum_t instr, const Immed64& i, const Reg64& r) {
    xedEmit(instr, toXedOperand(r), toXedOperand(i, sz::qword),
            bytesToBits(sz::qword));
  }

  ALWAYS_INLINE
  void xedInstrIR(xed_iclass_enum_t instr, const Immed64& i, const Reg64& r,
                  int immSize, immFitFunc fitFunc) {
    xedEmit(instr, toXedOperand(r), toXedOperand(i, immSize, fitFunc),
            bytesToBits(sz::qword));
  }

  ALWAYS_INLINE
  void xedInstrIR(xed_iclass_enum_t instr, const Immed& i,
                  const RegXMM& r, int immSize) {
    xedEmit(instr, toXedOperand(r), toXedOperand(i, immSize));
  }

  // instr(reg, reg)

#define XED_INSTRR_WRAPPER_IMPL(bitsize)                            \
  ALWAYS_INLINE                                                     \
  void xedInstrRR(xed_iclass_enum_t instr, const Reg##bitsize& r1,  \
                  const Reg##bitsize& r2) {                         \
    xedEmit(instr, toXedOperand(r2), toXedOperand(r1), bitsize);    \
  }

#define XED_WRAP_X XED_INSTRR_WRAPPER_IMPL
  XED_WRAP_IMPL()
#undef XED_WRAP_X

  ALWAYS_INLINE
  void xedInstrRR_CL(xed_iclass_enum_t instr, const Reg64& r) {
    xedEmit(instr, toXedOperand(r), toXedOperand(XED_REG_CL),
            bytesToBits(sz::qword));
  }

  ALWAYS_INLINE
  void xedInstrRR(xed_iclass_enum_t instr, const Reg8& r1, const Reg32& r2) {
    xedEmit(instr, toXedOperand(r2), toXedOperand(r1), bytesToBits(sz::byte));
  }

  ALWAYS_INLINE
  void xedInstrRR(xed_iclass_enum_t instr, const Reg16& r1, const Reg32& r2) {
    xedEmit(instr, toXedOperand(r2), toXedOperand(r1), bytesToBits(sz::word));
  }

  ALWAYS_INLINE
  void xedInstrRR(xed_iclass_enum_t instr, const Reg8& r1, const Reg64& r2) {
    xedEmit(instr, toXedOperand(r2), toXedOperand(r1), bytesToBits(sz::byte));
  }

  ALWAYS_INLINE
  void xedInstrRR(xed_iclass_enum_t instr, const Reg64& r1, const RegXMM& r2) {
    xedEmit(instr, toXedOperand(r2), toXedOperand(r1));
  }

  ALWAYS_INLINE
  void xedInstrRR(xed_iclass_enum_t instr, const RegXMM& r1, const Reg64& r2) {
    xedEmit(instr, toXedOperand(r2), toXedOperand(r1));
  }

  // Most instr(xmm_1, xmm_2) instructions take operands in reverse order
  // compared to instr(reg_1, reg_2): source and destination are swapped

  ALWAYS_INLINE
  void xedInstrRR(xed_iclass_enum_t instr, const RegXMM& r1, const RegXMM& r2) {
    xedEmit(instr, toXedOperand(r1), toXedOperand(r2));
  }

  // instr(imm)

  ALWAYS_INLINE
  void xedInstrI(xed_iclass_enum_t instr, const Immed& i, int immSize) {
    xedEmit(instr, toXedOperand(i, immSize), bytesToBits(immSize));
  }

  // instr(mem)

  ALWAYS_INLINE
  void xedInstrM(xed_iclass_enum_t instr, const MemoryRef& m,
                 int size = sz::qword) {
      xedEmit(instr, toXedOperand(m, size), bytesToBits(size));
  }

  ALWAYS_INLINE
  void xedInstrM(xed_iclass_enum_t instr, RIPRelativeRef m,
                 int size = sz::qword) {
    static XedLenCache lenCache;
    auto instrLen = xedCacheLen(
                      &lenCache,
                      [&]() {
                        return xedEmit(instr, toXedOperand(nullrip, size),
                                       0, dest());
                      }, xedLenCacheKey(instr, 0));
    m.r.disp -= ((int64_t)frontier() + (int64_t)instrLen);
    xedEmit(instr, toXedOperand(m, size), 0);
  }

  // instr(imm, mem)

  ALWAYS_INLINE
  void xedInstrIM(xed_iclass_enum_t instr, const Immed& i, const MemoryRef& m,
                  int size = sz::qword) {
    xedEmit(instr,  toXedOperand(m, size), toXedOperand(i, size),
            bytesToBits(size));
  }

  ALWAYS_INLINE
  void xedInstrIM(xed_iclass_enum_t instr, const Immed& i, const MemoryRef& m,
                  int immSize, int memSize) {
    xedEmit(instr, toXedOperand(m, memSize), toXedOperand(i, immSize),
            bytesToBits(memSize));
  }

  ALWAYS_INLINE
  void xedInstrIM(xed_iclass_enum_t instr, const Immed& i, const MemoryRef& m,
                  int immSize, immFitFunc fitFunc, int memSize) {
    xedEmit(instr, toXedOperand(m, memSize), toXedOperand(i, immSize, fitFunc),
            bytesToBits(memSize));
  }

  // instr(mem, reg)

#define XED_INSTMR_WRAPPER_IMPL(bitsize)                                \
  ALWAYS_INLINE                                                         \
  void xedInstrMR(xed_iclass_enum_t instr, const MemoryRef& m,          \
                  const Reg##bitsize& r,                                \
                  int memSize = bitsToBytes(bitsize)) {                 \
    xedEmit(instr, toXedOperand(r), toXedOperand(m, memSize), bitsize); \
  }                                                                     \
                                                                        \
  ALWAYS_INLINE                                                         \
  void xedInstrMR(xed_iclass_enum_t instr, RIPRelativeRef m,            \
                  const Reg##bitsize& r) {                              \
    static XedLenCache lenCache;                                        \
    auto instrLen = xedCacheLen(                                        \
                      &lenCache,                                        \
                      [&] {                                             \
                        return xedEmit(                                 \
                                instr, toXedOperand(r),                 \
                                toXedOperand(nullrip,                   \
                                             bitsToBytes(bitsize)),     \
                                bitsize, dest());                       \
                      }, xedLenCacheKey(instr, 0));                     \
    m.r.disp -= ((int64_t)frontier() + (int64_t)instrLen);              \
    xedEmit(instr, toXedOperand(r),                                     \
            toXedOperand(m, bitsToBytes(bitsize)), bitsize);            \
  }

#define XED_WRAP_X XED_INSTMR_WRAPPER_IMPL
  XED_WRAP_IMPL()
#undef XED_WRAP_X

  ALWAYS_INLINE
  void xedInstrMR(xed_iclass_enum_t instr, const MemoryRef& m,
                  const RegXMM& r, int memSize = sz::qword) {
    xedEmit(instr, toXedOperand(r), toXedOperand(m, memSize));
  }

  ALWAYS_INLINE
  void xedInstrMR(xed_iclass_enum_t instr, RIPRelativeRef m,
                  const RegXMM& r, int memSize = sz::qword) {
    static XedLenCache lenCache;
    auto instrLen = xedCacheLen(
                      &lenCache,
                      [&]() {
                        return xedEmit(
                                instr, toXedOperand(r),
                                toXedOperand(nullrip, memSize),
                                0, dest());
                      }, xedLenCacheKey(instr, 0));
    m.r.disp -= ((int64_t)frontier() + (int64_t)instrLen);
    xedEmit(instr, toXedOperand(r), toXedOperand(m, memSize));
  }

  // instr(reg, mem)

#define XED_INSTRM_WRAPPER_IMPL(bitsize)                            \
  ALWAYS_INLINE                                                     \
  void xedInstrRM(xed_iclass_enum_t instr, const Reg##bitsize& r,   \
                  const MemoryRef& m) {                             \
    xedEmit(instr, toXedOperand(m, bitsToBytes(bitsize)),           \
            toXedOperand(r), bitsize);                              \
  }

#define XED_WRAP_X XED_INSTRM_WRAPPER_IMPL
  XED_WRAP_IMPL()
#undef XED_WRAP_X

  ALWAYS_INLINE
  void xedInstrRM(xed_iclass_enum_t instr, const RegXMM& r,
                  const MemoryRef& m, int memSize = sz::qword) {
    xedEmit(instr, toXedOperand(m, memSize), toXedOperand(r));
  }

  // instr(xmm, xmm, imm)

  ALWAYS_INLINE
  void xedInstrIRR(xed_iclass_enum_t instr, const RegXMM& r1, const RegXMM& r2,
                   const Immed& i, int immSize) {
    xedEmit(instr, toXedOperand(r1), toXedOperand(r2),
            toXedOperand(i, immSize));
  }

  // instr(relbr)

  void xedInstrRelBr(xed_iclass_enum_t instr,
                     CodeAddress destination, int size) {
    static XedLenCache lenCache;
    auto instrLen = xedCacheLen(
                      &lenCache,
                      [&]() {
                        return xedEmit(instr, toXedOperand((CodeAddress)0,
                                       size), 0, dest());
                      }, xedLenCacheKey(instr, size));
    auto target = destination - (frontier() + instrLen);
    xedEmit(instr, toXedOperand((CodeAddress)target, size));
  }

  // instr()

  ALWAYS_INLINE
  void xedInstr(xed_iclass_enum_t instr, int size = sz::qword) {
    xedEmit(instr, bytesToBits(size));
  }
};

}}

#endif
