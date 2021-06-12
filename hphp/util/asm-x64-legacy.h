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

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

enum X64InstrFlags {
  IF_REVERSE    = 0x0001, // The operand encoding for some instructions are
                          // "backwards" in x64; these instructions are
                          // called "reverse" instructions. There are a few
                          // details about emitting "reverse" instructions:
                          // (1) for the R_M address mode, we use the MR
                          // opcode, (2) for M_R and R address modes, we use
                          // the RM opcode, and (3) for the R_R address mode,
                          // we still use MR opcode, but we have to swap the
                          // first argument and the second argument.

  IF_TWOBYTEOP  = 0x0002, // Some instructions have two byte opcodes. For
                          // these instructions, an additional byte (0x0F) is
                          // emitted before the standard opcode byte.

  IF_JCC        = 0x0004, // instruction is jcc
  IF_IMUL       = 0x0008, // instruction is imul
  IF_HAS_IMM8   = 0x0010, // instruction has an encoding that takes an 8-bit
                          // immediate
  IF_SHIFT      = 0x0020, // instruction is rol, ror, rcl, rcr, shl, shr, sar
  IF_RET        = 0x0040, // instruction is ret
  IF_SHIFTD     = 0x0080, // instruction is shld, shrd
  IF_NO_REXW    = 0x0100, // rexW prefix is not needed
  IF_MOV        = 0x0200, // instruction is mov
  IF_COMPACTR   = 0x0400, // instruction supports compact-R encoding
  IF_RAX        = 0x0800, // instruction supports special rax encoding
  IF_XCHG       = 0x1000, // instruction is xchg (not xchgb)
  IF_BYTEREG    = 0x2000, // instruction is movzbq, movsbq
  IF_66PREFIXED = 0x4000, // instruction requires a manditory 0x66 prefix
  IF_F3PREFIXED = 0x8000, // instruction requires a manditory 0xf3 prefix
  IF_F2PREFIXED = 0x10000, // instruction requires a manditory 0xf2 prefix
  IF_THREEBYTEOP = 0x20000, // instruction requires a 0x0F 0x3[8A] prefix
  IF_ROUND       = 0x40000, // instruction is round(sp)d
};

/*
  Address mode to table index map:
      Table index 0 <- R_R / M_R(n) / R_M(r) / R(n)
      Table index 1 <- R_M(n) / M_R(r) / R(r)
      Table index 2 <- I / R_I / M_I / R_R_I / M_R_I / R_M_I
      Table index 3 <- "/digit" value used by the above address modes
      Table index 4 <- special R_I (for rax)
      Table index 5 <- compact-R / none

  (n) - for normal instructions only (IF_REVERSE flag is not set)
  (r) - for reverse instructions only (IF_REVERSE flag is set)

  0xF1 is used to indicate invalid opcodes.
*/

struct X64Instr {
  unsigned char table[6];
  unsigned long flags;
};

//                                    0    1    2    3    4    5     flags
const X64Instr instr_divsd =   { { 0x5E,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x10102 };
const X64Instr instr_movups =  { { 0x10,0x11,0xF1,0x00,0xF1,0xF1 }, 0x0103  };
const X64Instr instr_movdqa =  { { 0x6F,0x7F,0xF1,0x00,0xF1,0xF1 }, 0x4103  };
const X64Instr instr_movdqu =  { { 0x6F,0x7F,0xF1,0x00,0xF1,0xF1 }, 0x8103  };
const X64Instr instr_movsd =   { { 0x11,0x10,0xF1,0x00,0xF1,0xF1 }, 0x10102 };
const X64Instr instr_gpr2xmm = { { 0x6e,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x4002  };
const X64Instr instr_xmm2gpr = { { 0x7e,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x4002  };
const X64Instr instr_xmmsub =  { { 0x5c,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x10102 };
const X64Instr instr_xmmadd =  { { 0x58,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x10102 };
const X64Instr instr_xmmmul =  { { 0x59,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x10102 };
const X64Instr instr_xmmsqrt = { { 0x51,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x10102 };
const X64Instr instr_ucomisd = { { 0x2e,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x4102  };
const X64Instr instr_pxor=     { { 0xef,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x4102  };
const X64Instr instr_psrlq=    { { 0xF1,0xF1,0x73,0x02,0xF1,0xF1 }, 0x4112  };
const X64Instr instr_psllq=    { { 0xF1,0xF1,0x73,0x06,0xF1,0xF1 }, 0x4112  };
const X64Instr instr_cvtsi2sd= { { 0x2a,0x2a,0xF1,0x00,0xF1,0xF1 }, 0x10002 };
const X64Instr instr_cvttsd2si={ { 0x2c,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x10002 };
const X64Instr instr_lddqu =   { { 0xF0,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x10103 };
const X64Instr instr_unpcklpd ={ { 0x14,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x4102  };
const X64Instr instr_jmp =     { { 0xFF,0xF1,0xE9,0x04,0xE9,0xF1 }, 0x0910  };
const X64Instr instr_call =    { { 0xFF,0xF1,0xE8,0x02,0xE8,0xF1 }, 0x0900  };
const X64Instr instr_push =    { { 0xFF,0xF1,0x68,0x06,0xF1,0x50 }, 0x0510  };
const X64Instr instr_pop =     { { 0x8F,0xF1,0xF1,0x00,0xF1,0x58 }, 0x0500  };
const X64Instr instr_inc =     { { 0xFF,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0000  };
const X64Instr instr_dec =     { { 0xFF,0xF1,0xF1,0x01,0xF1,0xF1 }, 0x0000  };
const X64Instr instr_not =     { { 0xF7,0xF1,0xF1,0x02,0xF1,0xF1 }, 0x0000  };
const X64Instr instr_notb =    { { 0xF6,0xF1,0xF1,0x02,0xF1,0xF1 }, 0x0000  };
const X64Instr instr_neg =     { { 0xF7,0xF1,0xF1,0x03,0xF1,0xF1 }, 0x0000  };
const X64Instr instr_negb =    { { 0xF6,0xF1,0xF1,0x03,0xF1,0xF1 }, 0x0000  };
const X64Instr instr_add =     { { 0x01,0x03,0x81,0x00,0x05,0xF1 }, 0x0810  };
const X64Instr instr_addb =    { { 0x00,0x02,0x80,0x00,0x04,0xF1 }, 0x0810  };
const X64Instr instr_sub =     { { 0x29,0x2B,0x81,0x05,0x2D,0xF1 }, 0x0810  };
const X64Instr instr_subb =    { { 0x28,0x2A,0x80,0x05,0x2C,0xF1 }, 0x0810  };
const X64Instr instr_and =     { { 0x21,0x23,0x81,0x04,0x25,0xF1 }, 0x0810  };
const X64Instr instr_andb =    { { 0x20,0x22,0x80,0x04,0x24,0xF1 }, 0x0810  };
const X64Instr instr_or  =     { { 0x09,0x0B,0x81,0x01,0x0D,0xF1 }, 0x0810  };
const X64Instr instr_orb =     { { 0x08,0x0A,0x80,0x01,0x0C,0xF1 }, 0x0810  };
const X64Instr instr_xor =     { { 0x31,0x33,0x81,0x06,0x35,0xF1 }, 0x0810  };
const X64Instr instr_xorb =    { { 0x30,0x32,0x80,0x06,0x34,0xF1 }, 0x0810  };
const X64Instr instr_mov =     { { 0x89,0x8B,0xC7,0x00,0xF1,0xB8 }, 0x0600  };
const X64Instr instr_movb =    { { 0x88,0x8A,0xC6,0x00,0xF1,0xB0 }, 0x0610  };
const X64Instr instr_test =    { { 0x85,0x85,0xF7,0x00,0xA9,0xF1 }, 0x0800  };
const X64Instr instr_testb =   { { 0x84,0x84,0xF6,0x00,0xA8,0xF1 }, 0x0810  };
const X64Instr instr_cmp =     { { 0x39,0x3B,0x81,0x07,0x3D,0xF1 }, 0x0810  };
const X64Instr instr_cmpb =    { { 0x38,0x3A,0x80,0x07,0x3C,0xF1 }, 0x0810  };
const X64Instr instr_sbb =     { { 0x19,0x1B,0x81,0x03,0x1D,0xF1 }, 0x0810  };
const X64Instr instr_sbbb =    { { 0x18,0x1A,0x80,0x03,0x1C,0xF1 }, 0x0810  };
const X64Instr instr_adc =     { { 0x11,0x13,0x81,0x02,0x15,0xF1 }, 0x0810  };
const X64Instr instr_lea =     { { 0xF1,0x8D,0xF1,0x00,0xF1,0xF1 }, 0x0000  };
const X64Instr instr_xchgb =   { { 0x86,0x86,0xF1,0x00,0xF1,0xF1 }, 0x0000  };
const X64Instr instr_xchg =    { { 0x87,0x87,0xF1,0x00,0xF1,0x90 }, 0x1000  };
const X64Instr instr_imul =    { { 0xAF,0xF7,0x69,0x05,0xF1,0xF1 }, 0x0019  };
const X64Instr instr_mul =     { { 0xF7,0xF1,0xF1,0x04,0xF1,0xF1 }, 0x0000  };
const X64Instr instr_div =     { { 0xF7,0xF1,0xF1,0x06,0xF1,0xF1 }, 0x0000  };
const X64Instr instr_idiv =    { { 0xF7,0xF1,0xF1,0x07,0xF1,0xF1 }, 0x0000  };
const X64Instr instr_cdq =     { { 0xF1,0xF1,0xF1,0x00,0xF1,0x99 }, 0x0400  };
const X64Instr instr_ret =     { { 0xF1,0xF1,0xC2,0x00,0xF1,0xC3 }, 0x0540  };
const X64Instr instr_jcc =     { { 0xF1,0xF1,0x80,0x00,0xF1,0xF1 }, 0x0114  };
const X64Instr instr_cmovcc =  { { 0x40,0x40,0xF1,0x00,0xF1,0xF1 }, 0x0003  };
const X64Instr instr_setcc =   { { 0x90,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0102  };
const X64Instr instr_movswx =  { { 0xBF,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0003  };
const X64Instr instr_movsbx =  { { 0xBE,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x2003  };
const X64Instr instr_movzwx =  { { 0xB7,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0003  };
const X64Instr instr_movzbx =  { { 0xB6,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x2003  };
const X64Instr instr_cwde =    { { 0xF1,0xF1,0xF1,0x00,0xF1,0x98 }, 0x0400  };
const X64Instr instr_cqo =     { { 0xF1,0xF1,0xF1,0x00,0xF1,0x99 }, 0x0000  };
const X64Instr instr_rol =     { { 0xD3,0xF1,0xC1,0x00,0xF1,0xF1 }, 0x0020  };
const X64Instr instr_ror =     { { 0xD3,0xF1,0xC1,0x01,0xF1,0xF1 }, 0x0020  };
const X64Instr instr_rcl =     { { 0xD3,0xF1,0xC1,0x02,0xF1,0xF1 }, 0x0020  };
const X64Instr instr_rcr =     { { 0xD3,0xF1,0xC1,0x03,0xF1,0xF1 }, 0x0020  };
const X64Instr instr_shl =     { { 0xD3,0xF1,0xC1,0x04,0xF1,0xF1 }, 0x0020  };
const X64Instr instr_shr =     { { 0xD3,0xF1,0xC1,0x05,0xF1,0xF1 }, 0x0020  };
const X64Instr instr_sar =     { { 0xD3,0xF1,0xC1,0x07,0xF1,0xF1 }, 0x0020  };
const X64Instr instr_xadd =    { { 0xC1,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0002  };
const X64Instr instr_cmpxchg = { { 0xB1,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0002  };
const X64Instr instr_nop =     { { 0xF1,0xF1,0xF1,0x00,0xF1,0x90 }, 0x0500  };
const X64Instr instr_shld =    { { 0xA5,0xF1,0xA4,0x00,0xF1,0xF1 }, 0x0082  };
const X64Instr instr_shrd =    { { 0xAD,0xF1,0xAC,0x00,0xF1,0xF1 }, 0x0082  };
const X64Instr instr_int3 =    { { 0xF1,0xF1,0xF1,0x00,0xF1,0xCC }, 0x0500  };
const X64Instr instr_roundsd = { { 0xF1,0xF1,0x0b,0x00,0xF1,0xF1 }, 0x64112 };
const X64Instr instr_cmpsd =   { { 0xF1,0xF1,0xC2,0xF1,0xF1,0xF1 }, 0x10112 };
const X64Instr instr_crc32 =   { { 0xF1,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x30001 };
const X64Instr instr_prefetch ={ { 0x18,0xF1,0xF1,0x02,0xF1,0xF1 }, 0x0002  };

///////////////////////////////////////////////////////////////////////////////

/**
 * Copyright (c) 2009, Andrew J. Paroski
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The names of the contributors may not be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL ANDREW J. PAROSKI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

struct X64Assembler final : public X64AssemblerBase {
public:
  explicit X64Assembler(CodeBlock& cb) : X64AssemblerBase(cb) {}

  X64Assembler(const X64Assembler&) = delete;
  X64Assembler& operator=(const X64Assembler&) = delete;

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
  void name##b(MemoryRef m, Reg8 r)        { instrMR(instr, m, r); }  \

#define LOAD_OP(name, instr)                                          \
  void name##q(MemoryRef m, Reg64 r) { instrMR(instr, m, r); }        \
  void name##l(MemoryRef m, Reg32 r) { instrMR(instr, m, r); }        \
  void name##w(MemoryRef m, Reg16 r) { instrMR(instr, m, r); }        \
  void name##q(RIPRelativeRef m, Reg64 r) { instrMR(instr, m, r); }   \
  BYTE_LOAD_OP(name, instr##b)

#define BYTE_STORE_OP(name, instr)                                    \
  void name##b(Reg8 r, MemoryRef m)        { instrRM(instr, r, m); }  \
  void name##b(Immed i, MemoryRef m)       { instrIM8(instr, i, m); } \

#define STORE_OP(name, instr)                                           \
  void name##w(Immed i, MemoryRef m) { instrIM16(instr, i, m); }        \
  void name##l(Immed i, MemoryRef m) { instrIM32(instr, i, m); }        \
  void name##w(Reg16 r, MemoryRef m) { instrRM(instr, r, m); }          \
  void name##l(Reg32 r, MemoryRef m) { instrRM(instr, r, m); }          \
  void name##q(Reg64 r, MemoryRef m) { instrRM(instr, r, m); }          \
  BYTE_STORE_OP(name, instr ## b)

#define BYTE_REG_OP(name, instr)                              \
  void name##b(Reg8 r1, Reg8 r2) { instrRR(instr, r1, r2); }  \
  void name##b(Immed i, Reg8 r)  { instrIR(instr, i, r); }    \

#define REG_OP(name, instr)                                       \
  void name##q(Reg64 r1, Reg64 r2)   { instrRR(instr, r1, r2); }  \
  void name##l(Reg32 r1, Reg32 r2)   { instrRR(instr, r1, r2); }  \
  void name##w(Reg16 r1, Reg16 r2)   { instrRR(instr, r1, r2); }  \
  void name##l(Immed i, Reg32 r)     { instrIR(instr, i, r); }    \
  void name##w(Immed i, Reg16 r)     { instrIR(instr, i, r); }    \
  BYTE_REG_OP(name, instr##b)

  /*
   * For when we a have a memory operand and the operand size is
   * 64-bits, only a 32-bit (sign-extended) immediate is supported.
   */
#define IMM64_STORE_OP(name, instr)             \
  void name##q(Immed i, MemoryRef m) {          \
    return instrIM(instr, i, m);                \
  }

  /*
   * For instructions other than movq, even when the operand size is
   * 64 bits only a 32-bit (sign-extended) immediate is supported.
   */
#define IMM64R_OP(name, instr)                  \
  void name##q(Immed imm, Reg64 r) {            \
    always_assert(imm.fits(sz::dword));         \
    return instrIR(instr, imm, r);              \
  }

#define FULL_OP(name, instr)                    \
  LOAD_OP(name, instr)                          \
  STORE_OP(name, instr)                         \
  REG_OP(name, instr)                           \
  IMM64_STORE_OP(name, instr)                   \
  IMM64R_OP(name, instr)

  // We rename x64's mov to store and load for improved code
  // readability.
  LOAD_OP        (load,  instr_mov)
  STORE_OP       (store, instr_mov)
  IMM64_STORE_OP (store, instr_mov)
  REG_OP         (mov,   instr_mov)

  FULL_OP(add, instr_add)
  FULL_OP(xor, instr_xor)
  FULL_OP(sub, instr_sub)
  FULL_OP(and, instr_and)
  FULL_OP(or,  instr_or)
  FULL_OP(test,instr_test)
  FULL_OP(cmp, instr_cmp)
  FULL_OP(sbb, instr_sbb)

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
  void movq(Immed64 imm, Reg64 r) { instrIR(instr_mov, imm, r); }

  // movzbx is a special snowflake. We don't have movzbq because it behaves
  // exactly the same as movzbl but takes an extra byte.
  void loadzbl(MemoryRef m, Reg32 r)        { instrMR(instr_movzbx,
                                                      m, rbyte(r)); }
  void loadzwl(MemoryRef m, Reg32 r)        { instrMR(instr_movzwx, m, r); }
  void movzbl(Reg8 src, Reg32 dest)         { emitRR32(instr_movzbx,
                                                       rn(src), rn(dest)); }
  void movsbl(Reg8 src, Reg32 dest)         { emitRR(instr_movsbx,
                                                       rn(src), rn(dest)); }
  void movzwl(Reg16 src, Reg32 dest)        { emitRR32(instr_movzwx,
                                                       rn(src), rn(dest)); }

  void loadsbq(MemoryRef m, Reg64 r)        { instrMR(instr_movsbx,
                                                      m, r); }
  void movsbq(Reg8 src, Reg64 dest)         { emitRR(instr_movsbx,
                                                       rn(src), rn(dest)); }
  void crc32q(Reg64 src, Reg64 dest)        { instrRR(instr_crc32, src, dest); }

  void lea(MemoryRef p, Reg64 reg)        { instrMR(instr_lea, p, reg); }
  void lea(RIPRelativeRef p, Reg64 reg)   { instrMR(instr_lea, p, reg); }

  void xchgq(Reg64 r1, Reg64 r2) { instrRR(instr_xchg, r1, r2); }
  void xchgl(Reg32 r1, Reg32 r2) { instrRR(instr_xchg, r1, r2); }
  void xchgb(Reg8 r1, Reg8 r2)   { instrRR(instr_xchgb, r1, r2); }

  void imul(Reg64 r1, Reg64 r2)  { instrRR(instr_imul, r1, r2); }

  void push(Reg64 r)  { instrR(instr_push, r); }
  void pushl(Reg32 r) { instrR(instr_push, r); }
  void pop (Reg64 r)  { instrR(instr_pop,  r); }
  void idiv(Reg64 r)  { instrR(instr_idiv, r); }
  void incq(Reg64 r)  { instrR(instr_inc,  r); }
  void incl(Reg32 r)  { instrR(instr_inc,  r); }
  void incw(Reg16 r)  { instrR(instr_inc,  r); }
  void decq(Reg64 r)  { instrR(instr_dec,  r); }
  void decl(Reg32 r)  { instrR(instr_dec,  r); }
  void decw(Reg16 r)  { instrR(instr_dec,  r); }
  void notb(Reg8 r)   { instrR(instr_notb, r); }
  void not(Reg64 r)   { instrR(instr_not,  r); }
  void neg(Reg64 r)   { instrR(instr_neg,  r); }
  void negb(Reg8 r)   { instrR(instr_negb, r); }
  void ret()          { emit(instr_ret); }
  void ret(Immed i)   { emitI(instr_ret, i.w(), sz::word); }
  void cqo()          { emit(instr_cqo); }
  void nop()          { emit(instr_nop); }
  void int3()         { emit(instr_int3); }
  void ud2()          { byte(0x0f); byte(0x0b); }
  void pushf()        { byte(0x9c); }
  void popf()         { byte(0x9d); }
  void lock()         { byte(0xF0); }

  void push(MemoryRef m) { instrM(instr_push, m); }
  void pop (MemoryRef m) { instrM(instr_pop,  m); }
  void prefetch(MemoryRef m) { instrM(instr_prefetch, m); }
  void incq(MemoryRef m) { instrM(instr_inc,  m); }
  void incl(MemoryRef m) { instrM32(instr_inc, m); }
  void incw(MemoryRef m) { instrM16(instr_inc, m); }
  void decqlock(MemoryRef m) { lock(); decq(m); }
  void decq(MemoryRef m) { instrM(instr_dec,  m); }
  void decl(MemoryRef m) { instrM32(instr_dec, m); }
  void decw(MemoryRef m) { instrM16(instr_dec, m); }

  void push(Immed64 i) { emitI(instr_push, i.q()); }

  void movups(RegXMM x, MemoryRef m)        { instrRM(instr_movups, x, m); }
  void movups(MemoryRef m, RegXMM x)        { instrMR(instr_movups, m, x); }
  void movdqu(RegXMM x, MemoryRef m)        { instrRM(instr_movdqu, x, m); }
  void movdqu(MemoryRef m, RegXMM x)        { instrMR(instr_movdqu, m, x); }
  void movdqa(RegXMM x, RegXMM y)           { instrRR(instr_movdqa, x, y); }
  void movdqa(RegXMM x, MemoryRef m)        { instrRM(instr_movdqa, x, m); }
  void movdqa(MemoryRef m, RegXMM x)        { instrMR(instr_movdqa, m, x); }
  void movsd (RegXMM x, RegXMM y)           { instrRR(instr_movsd,  x, y); }
  void movsd (RegXMM x, MemoryRef m)        { instrRM(instr_movsd,  x, m); }
  void movsd (MemoryRef m, RegXMM x)        { instrMR(instr_movsd,  m, x); }
  void movsd (RIPRelativeRef m, RegXMM x)   { instrMR(instr_movsd,  m, x); }
  void lddqu (MemoryRef m, RegXMM x)        { instrMR(instr_lddqu, m, x); }
  void unpcklpd(RegXMM s, RegXMM d)         { instrRR(instr_unpcklpd, d, s); }

  void rorq  (Immed i, Reg64 r) { instrIR(instr_ror, i, r); }
  void shlq  (Immed i, Reg64 r) { instrIR(instr_shl, i, r); }
  void shrq  (Immed i, Reg64 r) { instrIR(instr_shr, i, r); }
  void sarq  (Immed i, Reg64 r) { instrIR(instr_sar, i, r); }
  void shll  (Immed i, Reg32 r) { instrIR(instr_shl, i, r); }
  void shrl  (Immed i, Reg32 r) { instrIR(instr_shr, i, r); }
  void shlw  (Immed i, Reg16 r) { instrIR(instr_shl, i, r); }
  void shrw  (Immed i, Reg16 r) { instrIR(instr_shr, i, r); }

  void shlq (Reg64 r) { instrR(instr_shl, r); }
  void shrq (Reg64 r) { instrR(instr_shr, r); }
  void sarq (Reg64 r) { instrR(instr_sar, r); }

  void roundsd (RoundDirection d, RegXMM src, RegXMM dst) {
    emitIRR(instr_roundsd, rn(dst), rn(src), ssize_t(d));
  }

  void cmpsd(RegXMM src, RegXMM dst, ComparisonPred pred) {
    emitIRR(instr_cmpsd, rn(dst), rn(src), ssize_t(pred));
  }

  /*
   * Control-flow directives.  Primitive labeling/patching facilities
   * are available, as well as slightly higher-level ones via the
   * Label class.
   */

  void jmp(Reg64 r)            { instrR(instr_jmp, r); }
  void jmp(MemoryRef m)        { instrM(instr_jmp, m); }
  void jmp(RIPRelativeRef m)   { instrM(instr_jmp, m); }
  void call(Reg64 r)           { instrR(instr_call, r); }
  void call(MemoryRef m)       { instrM(instr_call, m); }
  void call(RIPRelativeRef m)  { instrM(instr_call, m); }

  void jmp8(CodeAddress dest)  { emitJ8(instr_jmp, ssize_t(dest)); }

  void jmp(CodeAddress dest) {
    always_assert_flog(dest && jmpDeltaFits(dest), "Bad Jmp: {}", dest);
    emitJ32(instr_jmp, ssize_t(dest));
  }

  void call(CodeAddress dest) {
    always_assert(dest && jmpDeltaFits(dest));
    emitJ32(instr_call, ssize_t(dest));
  }

  void jcc(ConditionCode cond, CodeAddress dest) {
    emitCJ32(instr_jcc, cond, (ssize_t)dest);
  }

  void jcc8(ConditionCode cond, CodeAddress dest) {
    emitCJ8(instr_jcc, cond, (ssize_t)dest);
  }

  using X64AssemblerBase::call;
  using X64AssemblerBase::jmp;
  using X64AssemblerBase::jmp8;
  using X64AssemblerBase::jcc;
  using X64AssemblerBase::jcc8;

  void setcc(int cc, Reg8 byteReg) {
    emitCR(instr_setcc, cc, rn(byteReg), sz::byte);
  }

  void psllq(Immed i, RegXMM r) { emitIR(instr_psllq, rn(r), i.b()); }
  void psrlq(Immed i, RegXMM r) { emitIR(instr_psrlq, rn(r), i.b()); }

  void movq_rx(Reg64 rSrc, RegXMM rdest) {
    emitRR(instr_gpr2xmm, rn(rdest), rn(rSrc));
  }
  void movq_xr(RegXMM rSrc, Reg64 rdest) {
    emitRR(instr_xmm2gpr, rn(rSrc), rn(rdest));
  }

  void addsd(RegXMM src, RegXMM srcdest) {
    emitRR(instr_xmmadd, rn(srcdest), rn(src));
  }
  void mulsd(RegXMM src, RegXMM srcdest) {
    emitRR(instr_xmmmul, rn(srcdest), rn(src));
  }
  void subsd(RegXMM src, RegXMM srcdest) {
    emitRR(instr_xmmsub, rn(srcdest), rn(src));
  }
  void pxor(RegXMM src, RegXMM srcdest) {
    emitRR(instr_pxor, rn(srcdest), rn(src));
  }
  void cvtsi2sd(Reg64 src, RegXMM dest) {
    emitRR(instr_cvtsi2sd, rn(dest), rn(src));
  }
  void cvtsi2sd(MemoryRef m, RegXMM dest) {
    instrMR(instr_cvtsi2sd, m, dest);
  }
  void ucomisd(RegXMM l, RegXMM r) {
    emitRR(instr_ucomisd, rn(l), rn(r));
  }
  void sqrtsd(RegXMM src, RegXMM dest) {
    emitRR(instr_xmmsqrt, rn(dest), rn(src));
  }

  void divsd(RegXMM src, RegXMM srcdest) {
    emitRR(instr_divsd, rn(srcdest), rn(src));
  }
  void cvttsd2siq(RegXMM src, Reg64 dest) {
    emitRR(instr_cvttsd2si, rn(dest), rn(src));
  }

  void emitInt3s(int n) {
    for (auto i = 0; i < n; ++i) {
      byte(0xcc);
    }
  }

  void emitNop(int n) {
    if (n == 0) return;
    static const uint8_t nops[][9] = {
      { },
      { 0x90 },
      { 0x66, 0x90 },
      { 0x0f, 0x1f, 0x00 },
      { 0x0f, 0x1f, 0x40, 0x00 },
      { 0x0f, 0x1f, 0x44, 0x00, 0x00 },
      { 0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00 },
      { 0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00 },
      { 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 },
      { 0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 },
    };
    // While n >= 9, emit 9 byte NOPs
    while (n >= 9) {
      bytes(9, nops[9]);
      n -= 9;
    }
    bytes(n, nops[n]);
  }

  void pad() {
    while (available() >= 2) ud2();
    if (available() > 0) int3();
    assertx(available() == 0);
  }

  ALWAYS_INLINE
  X64Assembler& prefix(const MemoryRef& mr) {
    static const uint8_t prefixes[] = {
      0xFF,   // unused
      0x64,   // Segment::FS prefix
      0x65    // Segment::GS prefix
    };
    if (mr.segment != Segment::DS) {
      byte(prefixes[int(mr.segment)]);
    }
    return *this;
  }
  /*
   * Low-level emitter functions.
   *
   * These functions are the core of the assembler, and can also be
   * used directly.
   */

  // op %r
  // ------
  // Restrictions:
  //     r cannot be set to 'none'
  ALWAYS_INLINE
  void emitCR(X64Instr op, int jcond, RegNumber regN, int opSz = sz::qword) {
    assert(regN != noreg);
    int r = int(regN);

    // Opsize prefix
    if (opSz == sz::word) {
      byte(kOpsizePrefix);
    }

    // REX
    unsigned char rex = 0;
    bool highByteReg = false;
    if (opSz == sz::byte) {
      if (byteRegNeedsRex(r)) {
        rex |= 0x40;
      }
      r = byteRegEncodeNumber(r, highByteReg);
    }
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
    if (r & 8) rex |= 1;
    if (rex) {
      byte(0x40 | rex);
      if (highByteReg) byteRegMisuse();
    }
    // If the instruction supports compact-R mode, use that
    if (op.flags & IF_COMPACTR) {
      byte(op.table[5] | (r & 7));
      return;
    }
    char opcode = (op.flags & IF_REVERSE) ? op.table[1] : op.table[0];
    char rval = op.table[3];
    // Handle two byte opcodes
    if (op.flags & IF_TWOBYTEOP) byte(0x0F);
    byte(opcode | jcond);
    emitModrm(3, rval, r);
  }

  ALWAYS_INLINE
  void emitR(X64Instr op, RegNumber r, int opSz = sz::qword) {
    emitCR(op, 0, r, opSz);
  }

  ALWAYS_INLINE
  void emitR32(X64Instr op, RegNumber r) {
    emitCR(op, 0, r, sz::dword);
  }

  ALWAYS_INLINE
  void emitR16(X64Instr op, RegNumber r) {
    emitCR(op, 0, r, sz::word);
  }

  // op %r2, %r1
  // -----------
  // Restrictions:
  //     r1 cannot be set to noreg
  //     r2 cannot be set to noreg
  ALWAYS_INLINE
  void emitCRR(X64Instr op, int jcond, RegNumber rn1, RegNumber rn2,
               int opSz = sz::qword) {
    assert(rn1 != noreg && rn2 != noreg);
    int r1 = int(rn1);
    int r2 = int(rn2);
    bool reverse = ((op.flags & IF_REVERSE) != 0);
    prefixBytes(op.flags, opSz);
    // The xchg instruction is special; we have compact encodings for
    // exchanging with rax or eax.
    if (op.flags & IF_XCHG) {
      if (r1 == int(reg::rax)) {
        // REX
        unsigned char rex = 0;
        if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
        assert(!(op.flags & IF_BYTEREG));
        if (r2 & 8) rex |= (reverse ? 4 : 1);
        if (rex) byte(0x40 | rex);
        // If the second register is rax, emit opcode with the first
        // register id embedded
        byte(op.table[5] | (r2 & 7));
        return;
      } else if (r2 == int(reg::rax)) {
        reverse = !reverse;
        // REX
        unsigned char rex = 0;
        if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) {
          rex |= 8;
        }
        if (r1 & 8) rex |= (reverse ? 1 : 4);
        if (rex) byte(0x40 | rex);
        // If the first register is rax, emit opcode with the second
        // register id embedded
        byte(op.table[5] | (r1 & 7));
        return;
      }
    }
    // REX
    unsigned char rex = 0;
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
    bool highByteReg = false;
    // movzbx's first operand is a bytereg regardless of operand size
    if (opSz == sz::byte || (op.flags & IF_BYTEREG)) {
      if (byteRegNeedsRex(r1) ||
          (!(op.flags & IF_BYTEREG) && byteRegNeedsRex(r2))) {
        rex |= 0x40;
      }
      r1 = byteRegEncodeNumber(r1, highByteReg);
      r2 = byteRegEncodeNumber(r2, highByteReg);
    }
    if (r1 & 8) rex |= (reverse ? 1 : 4);
    if (r2 & 8) rex |= (reverse ? 4 : 1);
    if (rex) {
      byte(0x40 | rex);
      if (highByteReg) byteRegMisuse();
    }
    // For two/three byte opcodes
    if ((op.flags & (IF_TWOBYTEOP | IF_IMUL | IF_THREEBYTEOP)) != 0) byte(0x0F);
    if ((op.flags & IF_THREEBYTEOP) != 0) byte(0x38);
    byte(op.table[0] | jcond);
    if (reverse) {
      emitModrm(3, r2, r1);
    } else {
      emitModrm(3, r1, r2);
    }
  }

  ALWAYS_INLINE
  void emitCRR32(X64Instr op, int jcond, RegNumber r1, RegNumber r2) {
    emitCRR(op, jcond, r1, r2, sz::dword);
  }

  ALWAYS_INLINE
  void emitRR(X64Instr op, RegNumber r1, RegNumber r2, int opSz = sz::qword) {
    emitCRR(op, 0, r1, r2, opSz);
  }

  ALWAYS_INLINE
  void emitRR32(X64Instr op, RegNumber r1, RegNumber r2) {
    emitCRR(op, 0, r1, r2, sz::dword);
  }

  ALWAYS_INLINE
  void emitRR16(X64Instr op, RegNumber r1, RegNumber r2) {
    emitCRR(op, 0, r1, r2, sz::word);
  }

  ALWAYS_INLINE
  void emitRR8(X64Instr op, RegNumber r1, RegNumber r2) {
    emitCRR(op, 0, r1, r2, sz::byte);
  }

  // op $imm, %r
  // -----------
  // Restrictions:
  //     r cannot be set to noreg
  ALWAYS_INLINE
  void emitIR(X64Instr op, RegNumber rname, ssize_t imm,
              int opSz = sz::qword) {
    assert(rname != noreg);
    int r = int(rname);
    // Opsize prefix
    prefixBytes(op.flags, opSz);
    // Determine the size of the immediate.  This might change opSz so
    // do it first.
    int immSize;
    if ((op.flags & IF_MOV) && opSz == sz::qword) {
      immSize = computeImmediateSizeForMovRI64(op, imm, opSz);
    } else {
      immSize = computeImmediateSize(op, imm, opSz);
    }
    // REX
    unsigned char rex = 0;
    bool highByteReg = false;
    if (opSz == sz::byte) {
      if (byteRegNeedsRex(r)) {
        rex |= 0x40;
      }
      r = byteRegEncodeNumber(r, highByteReg);
    }
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
    if (r & 8) rex |= 1;
    if (rex) {
      byte(0x40 | rex);
      if (highByteReg) byteRegMisuse();
    }
    // Use the special rax encoding if the instruction supports it
    if (r == int(reg::rax) && immSize == sz::dword &&
        (op.flags & IF_RAX)) {
      byte(op.table[4]);
      emitImmediate(op, imm, immSize);
      return;
    }
    // Use the compact-R encoding if the operand size and the immediate
    // size are the same
    if ((op.flags & IF_COMPACTR) && immSize == opSz) {
      byte(op.table[5] | (r & 7));
      emitImmediate(op, imm, immSize);
      return;
    }
    // For two byte opcodes
    if ((op.flags & (IF_TWOBYTEOP | IF_IMUL)) != 0) byte(0x0F);
    int rval = op.table[3];
    // shift/rotate instructions have special opcode when
    // immediate is 1
    if ((op.flags & IF_SHIFT) != 0 && imm == 1) {
      byte(0xd1);
      emitModrm(3, rval, r);
      // don't emit immediate
      return;
    }
    int opcode = (immSize == sz::byte && opSz != sz::byte) ?
      (op.table[2] | 2) : op.table[2];
    byte(opcode);
    emitModrm(3, rval, r);
    emitImmediate(op, imm, immSize);
  }

  ALWAYS_INLINE
  void emitIR32(X64Instr op, RegNumber r, ssize_t imm) {
    emitIR(op, r, imm, sz::dword);
  }

  ALWAYS_INLINE
  void emitIR16(X64Instr op, RegNumber r, ssize_t imm) {
    emitIR(op, r, safe_cast<int16_t>(imm), sz::word);
  }

  ALWAYS_INLINE
  void emitIR8(X64Instr op, RegNumber r, ssize_t imm) {
    emitIR(op, r, safe_cast<int8_t>(imm), sz::byte);
  }

  // op $imm, %r2, %r1
  // -----------------
  // Restrictions:
  //     r1 cannot be set to noreg
  //     r2 cannot be set to noreg
  ALWAYS_INLINE
  void emitIRR(X64Instr op, RegNumber rn1, RegNumber rn2, ssize_t imm,
               int opSz = sz::qword) {
    assert(rn1 != noreg && rn2 != noreg);
    int r1 = int(rn1);
    int r2 = int(rn2);
    bool reverse = ((op.flags & IF_REVERSE) != 0);
    // Opsize prefix
    prefixBytes(op.flags, opSz);
    // REX
    unsigned char rex = 0;
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
    bool highByteReg = false;
    if (opSz == sz::byte || (op.flags & IF_BYTEREG)) {
      if (byteRegNeedsRex(r1) ||
          (!(op.flags & IF_BYTEREG) && byteRegNeedsRex(r2))) {
        rex |= 0x40;
      }
      r1 = byteRegEncodeNumber(r1, highByteReg);
      r2 = byteRegEncodeNumber(r2, highByteReg);
    }
    if (r1 & 8) rex |= (reverse ? 1 : 4);
    if (r2 & 8) rex |= (reverse ? 4 : 1);
    if (rex) {
      byte(0x40 | rex);
      if (highByteReg) byteRegMisuse();
    }
    // Determine the size of the immediate
    int immSize = computeImmediateSize(op, imm, opSz);
    if (op.flags & IF_TWOBYTEOP || op.flags & IF_THREEBYTEOP) byte(0x0F);
    if (op.flags & IF_THREEBYTEOP) byte(0x3a);
    int opcode = (immSize == sz::byte && opSz != sz::byte &&
                  (op.flags & IF_ROUND) == 0) ?
      (op.table[2] | 2) : op.table[2];
    byte(opcode);
    if (reverse) {
      emitModrm(3, r2, r1);
    } else {
      emitModrm(3, r1, r2);
    }
    emitImmediate(op, imm, immSize);
  }

  ALWAYS_INLINE
  void emitCI(X64Instr op, int jcond, ssize_t imm, int opSz = sz::qword) {
    // Opsize prefix
    prefixBytes(op.flags, opSz);
    // REX
    if ((op.flags & IF_NO_REXW) == 0) {
      byte(0x48);
    }
    // Determine the size of the immediate
    int immSize = computeImmediateSize(op, imm, opSz);
    // Emit opcode
    if ((op.flags & IF_JCC) != 0) {
      // jcc is weird so we handle it separately
      if (immSize != sz::byte) {
        byte(0x0F);
        byte(jcond | 0x80);
      } else {
        byte(jcond | 0x70);
      }
    } else {
      int opcode = (immSize == sz::byte && opSz != sz::byte) ?
        (op.table[2] | 2) : op.table[2];
      byte(jcond | opcode);
    }
    emitImmediate(op, imm, immSize);
  }

  ALWAYS_INLINE
  void emitI(X64Instr op, ssize_t imm, int opSz = sz::qword) {
    emitCI(op, 0, imm, opSz);
  }

  ALWAYS_INLINE
  void emitJ8(X64Instr op, ssize_t imm) {
    assert((op.flags & IF_JCC) == 0);
    ssize_t delta = imm - ((ssize_t)codeBlock.frontier() + 2);
    // Emit opcode and 8-bit immediate
    byte(0xEB);
    byte(safe_cast<int8_t>(delta));
  }

  ALWAYS_INLINE
  void emitCJ8(X64Instr op, int jcond, ssize_t imm) {
    // this is for jcc only
    assert(op.flags & IF_JCC);
    ssize_t delta = imm - ((ssize_t)codeBlock.frontier() + 2);
    // Emit opcode
    byte(jcond | 0x70);
    // Emit 8-bit offset
    byte(safe_cast<int8_t>(delta));
  }

  ALWAYS_INLINE
  void emitJ32(X64Instr op, ssize_t imm) {
    // call and jmp are supported, jcc is not supported
    assert((op.flags & IF_JCC) == 0);
    int32_t delta =
      safe_cast<int32_t>(imm - ((ssize_t)codeBlock.frontier() + 5));
    uint8_t *bdelta = (uint8_t*)&delta;
    uint8_t instr[] = { op.table[2],
      bdelta[0], bdelta[1], bdelta[2], bdelta[3] };
    bytes(5, instr);
  }

  ALWAYS_INLINE
  void emitCJ32(X64Instr op, int jcond, ssize_t imm) {
    // jcc is supported, call and jmp are not supported
    assert(op.flags & IF_JCC);
    int32_t delta =
      safe_cast<int32_t>(imm - ((ssize_t)codeBlock.frontier() + 6));
    uint8_t* bdelta = (uint8_t*)&delta;
    uint8_t instr[6] = { 0x0f, uint8_t(0x80 | jcond),
      bdelta[0], bdelta[1], bdelta[2], bdelta[3] };
    bytes(6, instr);
  }

  // op disp(%br,%ir,s)
  //   (for reverse == false, hasImmediate == false, r == noreg)
  // op $imm, disp(%br,%ir,s)
  //   (for reverse == false, hasImmediate == true,  r == noreg)
  // op %r, disp(%br,%ir,s)
  //   (for reverse == false, hasImmediate == false, r != noreg)
  // op $imm, %r, disp(%br,%ir,s)
  //   (for reverse == false, hasImmediate == true,  r != noreg)
  // op disp(%br,%ir,s), %r
  //   (for reverse == true,  hasImmediate == false, r != noreg)
  // op $imm, disp(%br,%ir,s), %r
  //   (for reverse == true,  hasImmediate == true,  r != noreg)
  // -----------------------------------------------------------------
  // Restrictions:
  //     ir cannot be set to 'sp'
  ALWAYS_INLINE
  void emitCMX(X64Instr op, int jcond, RegNumber brName, RegNumber irName,
               int s, int64_t disp,
               RegNumber rName,
               bool reverse = false,
               ssize_t imm = 0,
               bool hasImmediate = false,
               int opSz = sz::qword,
               bool ripRelative = false) {
    assert(irName != rn(reg::rsp));

    int ir = int(irName);
    int r = int(rName);
    int br = int(brName);

    // The opsize prefix can be placed here, if the instruction
    // deals with words.
    // When an instruction has a manditory prefix, it goes before the
    // REX byte if we end up needing one.
    prefixBytes(op.flags, opSz);

    // Determine immSize from the 'hasImmediate' flag
    int immSize = sz::nosize;
    if (hasImmediate) {
      immSize = computeImmediateSize(op, imm, opSz);
    }
    if ((op.flags & IF_REVERSE) != 0) reverse = !reverse;
    // Determine if we need to use a two byte opcode;
    // imul is weird so we have a special case for it
    bool twoByteOpcode = ((op.flags & IF_TWOBYTEOP) != 0) ||
      ((op.flags & IF_IMUL) != 0 && rName != noreg &&
      immSize == sz::nosize);
    // Again, imul is weird
    if ((op.flags & IF_IMUL) != 0 && rName != noreg) {
      reverse = !reverse;
    }
    // The wily rex byte, a multipurpose extension to the opcode space for x64
    unsigned char rex = 0;
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;

    bool highByteReg = false;
    // XXX: This IF_BYTEREG check is a special case for movzbl: we currently
    // encode it using an opSz of sz::byte but it doesn't actually have a
    // byte-sized operand like other instructions can.
    if (!(op.flags & IF_BYTEREG) && opSz == sz::byte && rName != noreg) {
      if (byteRegNeedsRex(r)) {
        rex |= 0x40;
      }
      r = byteRegEncodeNumber(r, highByteReg);
    }

    if (rName != noreg && (r & 8)) rex |= 4;
    if (irName != noreg && (ir & 8)) rex |= 2;
    if (brName != noreg && (br & 8)) rex |= 1;
    if (rex) {
      byte(0x40 | rex);
      if (highByteReg) byteRegMisuse();
    }
    // Emit the opcode
    if (immSize != sz::nosize) {
      if (twoByteOpcode) byte(0x0F);
      if (immSize == sz::byte && opSz != sz::byte) {
        byte(op.table[2] | 2 | jcond);
      } else {
        byte(op.table[2] | jcond);
      }
    } else {
      if (twoByteOpcode) byte(0x0F);
      int opcode;
      if ((op.flags & IF_IMUL) != 0) {
        opcode = (rName == noreg) ? op.table[1] : op.table[0];
      } else {
        opcode = reverse ? op.table[1] : op.table[0];
      }
      byte(opcode | jcond);
    }
    // SIB byte if:
    //   1. We're using an index register.
    //   2. The base register is rsp-like.
    //   3. We're doing a baseless disp access and it is not rip-relative.
    bool sibIsNeeded =
      ir != int(noreg) ||                      /* 1 */
      br == int(reg::rsp) || br == int(reg::r12) || /* 2 */
      (br == int(noreg) && !ripRelative);
    // If there is no register and no immediate, use the /r value
    if (r == int(noreg)) r = op.table[3];
    // If noreg was specified for 'ir', we use
    // the encoding for the sp register
    if (ir == int(noreg)) ir = 4;
    int dispSize = sz::nosize;
    if (disp != 0) {
      if (!ripRelative && disp <= 127 && disp >= -128) {
        dispSize = sz::byte;
      } else {
        dispSize = sz::dword;
      }
    }
    // Set 'mod' based on the size of the displacement
    int mod;
    switch (dispSize) {
      case sz::nosize: mod = 0; break;
      case sz::byte: mod = 1; break;
      default: mod = 2; break;
    }
    // Handle special cases for 'br'
    if (br == int(noreg)) {
      // If noreg was specified for 'br', we use the encoding
      // for the rbp register (or rip, if we're emitting a
      // rip-relative instruction), and we must set mod=0 and
      // "upgrade" to a DWORD-sized displacement
      br = 5;
      mod = 0;
      dispSize = sz::dword;
    } else if ((br & 7) == 5 && dispSize == sz::nosize) {
      // If br == rbp and no displacement was specified, we
      // must "upgrade" to using a 1-byte displacement value
      dispSize = sz::byte;
      mod = 1;
    }
    // Emit modr/m and the sib
    if (sibIsNeeded) {
      // s:                               0  1  2   3  4   5   6   7  8
      static const int scaleLookup[] = { -1, 0, 1, -1, 2, -1, -1, -1, 3 };
      assert(s > 0 && s <= 8);
      int scale = scaleLookup[s];
      assert(scale != -1);
      emitModrm(mod, r, 4);
      byte((scale << 6) | ((ir & 7) << 3) | (br & 7));
    } else {
      emitModrm(mod, r, br);
    }
    // Emit displacement if needed
    if (dispSize == sz::dword) {
      if (ripRelative) {
        disp -= (int64_t)codeBlock.frontier() + immSize + dispSize;
        always_assert(deltaFits(disp, sz::dword));
      }
      dword(disp);
    } else if (dispSize == sz::byte) {
      byte(disp & 0xff);
    }
    // Emit immediate if needed
    if (immSize != sz::nosize) {
      emitImmediate(op, imm, immSize);
    }
  }

  ALWAYS_INLINE
  void emitIM(X64Instr op, RegNumber br, RegNumber ir, int s, int disp,
              ssize_t imm, int opSz = sz::qword) {
    emitCMX(op, 0, br, ir, s, disp, noreg, false, imm, true, opSz);
  }

  ALWAYS_INLINE
  void emitIM8(X64Instr op, RegNumber br, RegNumber ir, int s, int disp,
               ssize_t imm) {
    emitCMX(op, 0, br, ir, s, disp, noreg, false, imm, true,
            sz::byte);
  }

  ALWAYS_INLINE
  void emitIM16(X64Instr op, RegNumber br, RegNumber ir, int s, int disp,
                ssize_t imm) {
    emitCMX(op, 0, br, ir, s, disp, noreg, false, imm, true,
            sz::word);
  }

  ALWAYS_INLINE
  void emitIM32(X64Instr op, RegNumber br, RegNumber ir, int s, int disp,
                ssize_t imm) {
    emitCMX(op, 0, br, ir, s, disp, noreg, false, imm, true, sz::dword);
  }

  ALWAYS_INLINE
  void emitRM(X64Instr op, RegNumber br, RegNumber ir, int s, int disp,
              RegNumber r, int opSz = sz::qword) {
    emitCMX(op, 0, br, ir, s, disp, r, false, 0, false, opSz);
  }

  ALWAYS_INLINE
  void emitRM32(X64Instr op, RegNumber br, RegNumber ir, int s, int disp,
                RegNumber r) {
    emitCMX(op, 0, br, ir, s, disp, r, false, 0, false, sz::dword);
  }

  ALWAYS_INLINE
  void emitRM16(X64Instr op, RegNumber br, RegNumber ir, int s, int disp,
                RegNumber r) {
    emitCMX(op, 0, br, ir, s, disp, r, false, 0, false, sz::word);
  }

  ALWAYS_INLINE
  void emitRM8(X64Instr op, RegNumber br, RegNumber ir, int s, int disp,
               RegNumber r) {
    emitCMX(op, 0, br, ir, s, disp, r, false, 0, false, sz::byte);
  }

  ALWAYS_INLINE
  void emitCMR(X64Instr op, int jcond, RegNumber br, RegNumber ir,
               int s, int disp, RegNumber r, int opSz = sz::qword) {
    emitCMX(op, jcond, br, ir, s, disp, r, true, 0, false, opSz);
  }

  ALWAYS_INLINE
  void emitMR(X64Instr op, RegNumber br, RegNumber ir, int s, int64_t disp,
              RegNumber r, int opSz = sz::qword, bool ripRelative = false) {
    emitCMX(op, 0, br, ir, s, disp, r, true, 0, false, opSz, ripRelative);
  }

  ALWAYS_INLINE
  void emitMR32(X64Instr op, RegNumber br, RegNumber ir, int s, int disp,
                RegNumber r) {
    emitCMX(op, 0, br, ir, s, disp, r, true, 0, false, sz::dword);
  }

  ALWAYS_INLINE
  void emitMR16(X64Instr op, RegNumber br, RegNumber ir, int s, int disp,
                RegNumber r) {
    emitCMX(op, 0, br, ir, s, disp, r, true, 0, false, sz::word);
  }

  ALWAYS_INLINE
  void emitMR8(X64Instr op, RegNumber br, RegNumber ir, int s, int disp,
               RegNumber r) {
    emitCMX(op, 0, br, ir, s, disp, r, true, 0, false, sz::byte);
  }

  ALWAYS_INLINE
  void emitIRM(X64Instr op, RegNumber br, RegNumber ir, int s, int disp,
               RegNumber r, ssize_t imm, int opSz = sz::qword) {
    emitCMX(op, 0, br, ir, s, disp, r, false, imm, true, opSz);
  }

  ALWAYS_INLINE
  void emitIMR(X64Instr op, RegNumber br, RegNumber ir, int s, int disp,
               RegNumber r, ssize_t imm, int opSz = sz::qword) {
    emitCMX(op, 0, br, ir, s, disp, r, true, imm, true, opSz);
  }

  ALWAYS_INLINE
  void emitM(X64Instr op, RegNumber br, RegNumber ir, int s, int64_t disp,
             int opSz = sz::qword, bool ripRelative = false) {
    emitCMX(op, 0, br, ir, s, disp, noreg, false, 0, false, opSz,
            ripRelative);
  }

  ALWAYS_INLINE
  void emitM32(X64Instr op, RegNumber br, RegNumber ir, int s, int disp) {
    emitCMX(op, 0, br, ir, s, disp, noreg, false, 0, false, sz::dword);
  }

  ALWAYS_INLINE
  void emitM16(X64Instr op, RegNumber br, RegNumber ir, int s, int disp) {
    emitCMX(op, 0, br, ir, s, disp, noreg, false, 0, false, sz::word);
  }

  ALWAYS_INLINE
  void emitCM(X64Instr op, int jcond, RegNumber br,
              RegNumber ir, int s, int disp, int opSz = sz::qword) {
    emitCMX(op, jcond, br, ir, s, disp, noreg, false, 0, false, opSz);
  }

  // emit (with no arguments)
  ALWAYS_INLINE
  void emit(X64Instr op) {
    if ((op.flags & IF_NO_REXW) == 0) {
      byte(0x48);
    }
    byte(op.table[5]);
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
    emitCMX(instr_cmovcc, cc, rn(rbase), noreg, sz::byte, off, rn(rdest),
            false /*reverse*/);

  }
  inline void cload_reg64_disp_reg32(ConditionCode cc, Reg64 rbase,
                                     int off, Reg32 rdest) {
    emitCMX(instr_cmovcc, cc, rn(rbase), noreg, sz::byte, off, rn(rdest),
            false /*reverse*/,
            0 /*imm*/,
            false /*hasImmediate*/,
            sz::dword /*opSz*/);
  }
  inline void cmov_reg64_reg64(ConditionCode cc, Reg64 rsrc, Reg64 rdest) {
    emitCRR(instr_cmovcc, cc, rn(rsrc), rn(rdest));
  }

private:
  bool byteRegNeedsRex(int rn) const {
    // Without a rex, 4 through 7 mean the high 8-bit byte registers.
    return rn >= 4 && rn <= 7;
  }
  int byteRegEncodeNumber(int rn, bool& seenHigh) const {
    // We flag a bit in ah, ch, dh, bh so byteRegNeedsRex doesn't
    // trigger.
    if (rn & 0x80) seenHigh = true;
    return rn & ~0x80;
  }
  // In 64-bit mode, you can't mix accesses to high byte registers
  // with low byte registers other than al,cl,bl,dl.  We assert this.
  void byteRegMisuse() const {
    assert(!"High byte registers can't be used with new x64 registers, or"
            " anything requiring a REX prefix");
  }

  int computeImmediateSize(X64Instr op,
                           ssize_t imm,
                           int opsize = sz::dword) {
    // Most instructions take a 32-bit or 16-bit immediate,
    // depending on the presence of the opsize prefix (0x66).
    int immSize = opsize == sz::word ? sz::word : sz::dword;
    // ret always takes a 16-bit immediate.
    if (op.flags & IF_RET) {
      immSize = sz::word;
    }
    // Use an 8-bit immediate if the instruction supports it and if
    // the immediate value fits in a byte
    if (deltaFits(imm, sz::byte) && (op.flags & IF_HAS_IMM8) != 0) {
      immSize = sz::byte;
    }
    return immSize;
  }

  void emitModrm(int x, int y, int z) {
    byte((x << 6) | ((y & 7) << 3) | (z & 7));
  }

  /*
   * The mov instruction supports an 8 byte immediate for the RI
   * address mode when opSz is qword.  It also supports a 4-byte
   * immediate with opSz qword (the immediate is sign-extended).
   *
   * On the other hand, if it fits in 32-bits as an unsigned, we can
   * change opSz to dword, which will zero the top 4 bytes instead of
   * sign-extending.
   */
  int computeImmediateSizeForMovRI64(X64Instr op, ssize_t imm, int& opSz) {
    assert(opSz == sz::qword);
    if (deltaFits(imm, sz::dword)) {
      return computeImmediateSize(op, imm);
    }
    if (magFits(imm, sz::dword)) {
      opSz = sz::dword;
      return sz::dword;
    }
    return sz::qword;
  }

  void emitImmediate(X64Instr op, ssize_t imm, int immSize) {
    if (immSize == sz::nosize) {
      return;
    }
    if ((op.flags & (IF_SHIFT | IF_SHIFTD)) == 0) {
      if (immSize == sz::dword) {
        dword(imm);
      } else if (immSize == sz::byte) {
        byte(imm);
      } else if (immSize == sz::word) {
        word(imm);
      } else {
        qword(imm);
      }
    } else {
      // we always use a byte-sized immediate for shift instructions
      byte(imm);
    }
  }

  void prefixBytes(unsigned long flags, int opSz) {
    if (opSz == sz::word && !(flags & IF_RET)) byte(kOpsizePrefix);
    if (flags & IF_66PREFIXED) byte(0x66);
    if (flags & IF_F2PREFIXED) byte(0xF2);
    if (flags & IF_F3PREFIXED) byte(0xF3);
  }

private:
  // Wraps a bunch of the emit* functions to make using them with the
  // typed wrappers more terse. We should have these replace
  // the emit functions eventually.

#define UMR(m) rn(m.r.base), rn(m.r.index), m.r.scale, m.r.disp
#define URIP(m) noreg, noreg, sz::byte, m.r.disp

  void instrR(X64Instr   op, Reg64  r)           { emitR(op,    rn(r));        }
  void instrR(X64Instr   op, Reg32  r)           { emitR32(op,  rn(r));        }
  void instrR(X64Instr   op, Reg16  r)           { emitR16(op,  rn(r));        }
  void instrR(X64Instr   op, Reg8   r)           { emitR(op, rn(r), sz::byte); }
  void instrRR(X64Instr  op, Reg64  x, Reg64  y) { emitRR(op,   rn(x), rn(y)); }
  void instrRR(X64Instr  op, Reg32  x, Reg32  y) { emitRR32(op, rn(x), rn(y)); }
  void instrRR(X64Instr  op, Reg16  x, Reg16  y) { emitRR16(op, rn(x), rn(y)); }
  void instrRR(X64Instr  op, Reg8   x, Reg8   y) { emitRR8(op,  rn(x), rn(y)); }
  void instrRR(X64Instr  op, RegXMM x, RegXMM y) { emitRR(op,   rn(x), rn(y)); }
  void instrM(X64Instr   op, MemoryRef m)        { emitM(op,    UMR(m));       }
  void instrM(X64Instr   op, RIPRelativeRef m)   { emitM(op,    URIP(m),
                                                         sz::qword, true);     }
  void instrM32(X64Instr op, MemoryRef m)        { emitM32(op,  UMR(m));       }
  void instrM16(X64Instr op, MemoryRef m)        { emitM16(op,  UMR(m));       }

  void instrRM(X64Instr op,
               Reg64 r,
               MemoryRef m)        { emitRM(op, UMR(m), rn(r)); }
  void instrRM(X64Instr op,
               Reg32 r,
               MemoryRef m)        { emitRM32(op, UMR(m), rn(r)); }
  void instrRM(X64Instr op,
               Reg16 r,
               MemoryRef m)        { emitRM16(op, UMR(m), rn(r)); }
  void instrRM(X64Instr op,
               Reg8 r,
               MemoryRef m)        { emitRM8(op, UMR(m), rn(r)); }
  void instrRM(X64Instr op,
               RegXMM x,
               MemoryRef m)        { emitRM(op, UMR(m), rn(x)); }

  void instrMR(X64Instr op,
               MemoryRef m,
               Reg64 r)            { emitMR(op, UMR(m), rn(r)); }
  void instrMR(X64Instr op,
               MemoryRef m,
               Reg32 r)            { emitMR32(op, UMR(m), rn(r)); }
  void instrMR(X64Instr op,
               MemoryRef m,
               Reg16 r)            { emitMR16(op, UMR(m), rn(r)); }
  void instrMR(X64Instr op,
               MemoryRef m,
               Reg8 r)             { emitMR8(op, UMR(m), rn(r)); }
  void instrMR(X64Instr op,
               MemoryRef m,
               RegXMM x)           { emitMR(op, UMR(m), rn(x)); }
  void instrMR(X64Instr op,
               RIPRelativeRef m,
               Reg64 r)            { emitMR(op, URIP(m), rn(r),
                                            sz::qword, true); }
  void instrMR(X64Instr op,
               RIPRelativeRef m,
               RegXMM r)           { emitMR(op, URIP(m), rn(r),
                                            sz::qword, true); }

  void instrIR(X64Instr op, Immed64 i, Reg64 r) {
    emitIR(op, rn(r), i.q());
  }
  void instrIR(X64Instr op, Immed i, Reg64 r) {
    emitIR(op, rn(r), i.q());
  }
  void instrIR(X64Instr op, Immed i, Reg32 r) {
    emitIR32(op, rn(r), i.l());
  }
  void instrIR(X64Instr op, Immed i, Reg16 r) {
    emitIR16(op, rn(r), i.w());
  }
  void instrIR(X64Instr op, Immed i, Reg8 r) {
    emitIR8(op, rn(r), i.b());
  }

  void instrIM(X64Instr op, Immed i, MemoryRef m) {
    emitIM(op, UMR(m), i.q());
  }
  void instrIM32(X64Instr op, Immed i, MemoryRef m) {
    emitIM32(op, UMR(m), i.l());
  }
  void instrIM16(X64Instr op, Immed i, MemoryRef m) {
    emitIM16(op, UMR(m), i.w());
  }
  void instrIM8(X64Instr op, Immed i, MemoryRef m) {
    emitIM8(op, UMR(m), i.b());
  }

#undef UMR
#undef URIP
};

}}

