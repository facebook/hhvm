/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <stdint.h>
#include <stdio.h>

#include "util/asm-x64.h"

namespace HPHP {
namespace VM {
namespace Transl {

///////////////////////////////////////////////////////////////////////////////

// List of x64 instructions
const X64Instr instr_list[] = {
  instr_jmp,
  instr_call,
  instr_push,
  instr_pop,
  instr_inc,
  instr_dec,
  instr_not,
  instr_neg,
  instr_add,
  instr_sub,
  instr_and,
  instr_or,
  instr_xor,
  instr_mov,
  instr_test,
  instr_cmp,
  instr_sbb,
  instr_adc,
  instr_lea,
  instr_xchg,
  instr_imul,
  instr_mul,
  instr_div,
  instr_idiv,
  instr_cdq,
  instr_ret,
  instr_jcc,
  instr_cmovcc,
  instr_setcc,
  instr_movswx,
  instr_movzwx,
  instr_movsbx,
  instr_movzbx,
  instr_cwde,
  instr_rol,
  instr_ror,
  instr_rcl,
  instr_rcr,
  instr_shl,
  instr_shr,
  instr_sar,
  instr_xadd,
  instr_cmpxchg,
  instr_nop,
  instr_shld,
  instr_shrd,
  instr_int3
};

// List of x64 instruction names
const char * instr_names[] = {
  "jmp",
  "call",
  "push",
  "pop",
  "inc",
  "dec",
  "not",
  "neg",
  "add",
  "sub",
  "and",
  "or",
  "xor",
  "mov",
  "test",
  "cmp",
  "sbb",
  "adc",
  "lea",
  "xchg",
  "imul",
  "mul",
  "div",
  "idiv",
  "cdq",
  "ret",
  "jcc",
  "cmovcc",
  "setcc",
  "movswx",
  "movzwx",
  "movsbx",
  "movzbx",
  "cwde",
  "rol",
  "ror",
  "rcl",
  "rcr",
  "shl",
  "shr",
  "sar",
  "xadd",
  "cmpxchg",
  "nop",
  "shld",
  "shrd",
  "int3"
};

enum AddressModeMask {
  MASK_none = 0x1,
  MASK_R = 0x2,
  MASK_RR = 0x4,
  MASK_I = 0x8,
  MASK_IR = 0x10,
  MASK_IRR = 0x20,
  MASK_M = 0x40,
  MASK_RM = 0x80,
  MASK_MR = 0x100,
  MASK_IMR = 0x200,
  MASK_IM = 0x400,
  MASK_CI = 0x800,
  MASK_CMR = 0x1000,
  MASK_CR = 0x2000,
  MASK_CM = 0x4000,
  MASK_IRM = 0x8000,
  MASK_CRR = 0x10000,
};

int supported_AM[] = {
  MASK_R | MASK_M | MASK_I, // jmp
  MASK_R | MASK_M | MASK_I, // call
  MASK_R | MASK_M | MASK_I, // push
  MASK_R | MASK_M, // pop
  MASK_R | MASK_M, // inc
  MASK_R | MASK_M, // dec
  MASK_R | MASK_M, // not
  MASK_R | MASK_M, // neg
  MASK_RR | MASK_IR | MASK_RM | MASK_MR | MASK_IM, // add
  MASK_RR | MASK_IR | MASK_RM | MASK_MR | MASK_IM, // sub
  MASK_RR | MASK_IR | MASK_RM | MASK_MR | MASK_IM, // and
  MASK_RR | MASK_IR | MASK_RM | MASK_MR | MASK_IM, // or
  MASK_RR | MASK_IR | MASK_RM | MASK_MR | MASK_IM, // xor
  MASK_RR | MASK_IR | MASK_RM | MASK_MR | MASK_IM, // mov
  MASK_RR | MASK_IR | MASK_RM | MASK_MR | MASK_IM, // test
  MASK_RR | MASK_IR | MASK_RM | MASK_MR | MASK_IM, // cmp
  MASK_RR | MASK_IR | MASK_RM | MASK_MR | MASK_IM, // sbb
  MASK_RR | MASK_IR | MASK_RM | MASK_MR | MASK_IM, // adc
  MASK_MR,   // lea
  MASK_RR | MASK_RM | MASK_MR, // xchg
  MASK_R | MASK_RR | MASK_IRR | MASK_M | MASK_MR | MASK_IMR, // imul
  MASK_R,     // mul
  MASK_R,     // div
  MASK_R,     // idiv
  MASK_none,  // cdq
  MASK_none | MASK_I, // ret
  MASK_CI,   // jcc
  MASK_CRR | MASK_CMR, // cmovcc
  MASK_CR | MASK_CM, // setcc
  MASK_RR | MASK_MR, // movsx
  MASK_RR | MASK_MR, // movzx
  MASK_RR | MASK_MR, // movsx8
  MASK_RR | MASK_MR, // movzx8
  MASK_none,  // cwde
  MASK_R | MASK_IR | MASK_M | MASK_IM, // rol
  MASK_R | MASK_IR | MASK_M | MASK_IM, // ror
  MASK_R | MASK_IR | MASK_M | MASK_IM, // rcl
  MASK_R | MASK_IR | MASK_M | MASK_IM, // rcr
  MASK_R | MASK_IR | MASK_M | MASK_IM, // shl
  MASK_R | MASK_IR | MASK_M | MASK_IM, // shr
  MASK_R | MASK_IR | MASK_M | MASK_IM, // sar
  MASK_RR | MASK_RM, // xadd
  MASK_RR | MASK_RM, // cmpxchg
  MASK_none,  // nop
  MASK_RR | MASK_IRR | MASK_RM | MASK_IRM, // shld
  MASK_RR | MASK_IRR | MASK_RM | MASK_IRM, // shrd
  MASK_none   // int3
};

void callTarget(uint64_t arg0, uint64_t arg1) {
  printf("call target reached: arg0: %lx arg1: %lx\n",
         (long)arg0, (long)arg1);
}

void testEmitMethods() {
  X64Assembler e;
  e.init(10 << 20);

  using namespace HPHP::VM::Transl::reg;

  int n = sizeof(instr_list) / sizeof(instr_list[0]);
  for (int i = 0; i < n; ++i) {

    X64Instr op = instr_list[i];
    printf("%s:\n", instr_names[i]);

    if (supported_AM[i] & MASK_none) {
      printf("  Address mode: none\n");
      e.emit(op);
    }

    if (supported_AM[i] & MASK_R) {
      printf("  Address mode: R\n");
      e.emitR(op, rax);
      e.emitR(op, rsi);
      e.emitR(op, rbp);
      e.emitR(op, rsp);
      e.emitR(op, r8);
      e.emitR(op, r15);
      e.emitR(op, r13);
      e.emitR(op, r12);
    }

    if (supported_AM[i] & MASK_RR) {
      printf("  Address mode: RR\n");
      e.emitRR(op, rsi, rax);
      e.emitRR(op, rax, rdi);
      e.emitRR(op, rsi, rdi);
      e.emitRR(op, rbp, rsp);
      e.emitRR(op, rsp, rbp);
      e.emitRR(op, rsi, r8);
      e.emitRR(op, rax, r15);
      e.emitRR(op, rsi, r15);
      e.emitRR(op, rbp, r12);
      e.emitRR(op, rsp, r13);
      e.emitRR(op, r14, rax);
      e.emitRR(op, r8,  rdi);
      e.emitRR(op, r14, rdi);
      e.emitRR(op, r13, rsp);
      e.emitRR(op, r12, rbp);
      e.emitRR(op, r14, r8);
      e.emitRR(op, r8,  r15);
      e.emitRR(op, r14, r15);
      e.emitRR(op, r13, r12);
      e.emitRR(op, r12, r13);
      e.emitRR(op, rax, rax);
      e.emitRR(op, rax, r8);
      e.emitRR(op, r8, rax);
      e.emitRR(op, r8, r8);
    }

    if (supported_AM[i] & MASK_I) {
      printf("  Address mode: I\n");
      e.emitI(op, -128);
      e.emitI(op, 127);
      e.emitI(op, 0xF1);
      e.emitI(op, 1);
    }

    if (supported_AM[i] & MASK_IR) {
      printf("  Address mode: IR\n");
      e.emitIR(op, rbx, 1);
      e.emitIR(op, rax, -128);
      e.emitIR(op, rsi, -128);
      e.emitIR(op, rbp, 127);
      e.emitIR(op, rsp, 0xF1);
      e.emitIR(op, rsp, 1);
      e.emitIR(op, r11, 1);
      e.emitIR(op, r8, -128);
      e.emitIR(op, r14, -128);
      e.emitIR(op, r13, 127);
      e.emitIR(op, r12, 0xF1);
      e.emitIR(op, r12, 1);
      if (i == 13 /*instr_mov*/) {
        e.emitIR(op, rax, (ssize_t)0x1234123412341234);
        e.emitIR(op, r8, (ssize_t)0x1234123412341234);
      }
    }

    if (supported_AM[i] & MASK_IRR) {
      printf("  Address mode: IRR\n");
      e.emitIRR(op, rsi, rax, -128);
      e.emitIRR(op, rax, rdi, -128);
      e.emitIRR(op, rbp, rsp, 127);
      e.emitIRR(op, rsp, rbp, 0xF1);
      e.emitIRR(op, rsp, rbp, 1);
      e.emitIRR(op, r14, rax, -128);
      e.emitIRR(op, r8, rdi, -128);
      e.emitIRR(op, r13, rsp, 127);
      e.emitIRR(op, r12, rbp, 0xF1);
      e.emitIRR(op, r12, rbp, 1);
      e.emitIRR(op, rsi, r8, -128);
      e.emitIRR(op, rax, r15, -128);
      e.emitIRR(op, rbp, r12, 127);
      e.emitIRR(op, rsp, r13, 0xF1);
      e.emitIRR(op, rsp, r13, 1);
      e.emitIRR(op, r14, r8, -128);
      e.emitIRR(op, r8, r15, -128);
      e.emitIRR(op, r13, r12, 127);
      e.emitIRR(op, r12, r13, 0xF1);
      e.emitIRR(op, r12, r13, 1);
      e.emitIRR(op, rax, rax, 1);
      e.emitIRR(op, rax, r8, 1);
      e.emitIRR(op, r8, rax, 1);
      e.emitIRR(op, r8, r8, 1);
    }

    if (supported_AM[i] & MASK_M) {
      printf("  Address mode: M\n");
      e.emitM(op, rsi, rdi, sz::word, -128);
      e.emitM(op, rbp, noreg, sz::dword, 127);
      e.emitM(op, noreg, rbp, sz::byte, 127);
      e.emitM(op, rsp, rbp, sz::qword, 0xF1);
      e.emitM(op, rsi, r15, sz::word, -128);
      e.emitM(op, rbp, noreg, sz::dword, 127);
      e.emitM(op, noreg, r13, sz::byte, 127);
      e.emitM(op, rsp, r13, sz::qword, 0xF1);
      e.emitM(op, r14, rdi, sz::word, -128);
      e.emitM(op, r13, noreg, sz::dword, 127);
      e.emitM(op, noreg, rbp, sz::byte, 127);
      e.emitM(op, r12, rbp, sz::qword, 0xF1);
      e.emitM(op, r14, r15, sz::word, -128);
      e.emitM(op, r13, noreg, sz::dword, 127);
      e.emitM(op, noreg, r13, sz::byte, 127);
      e.emitM(op, r12, r13, sz::qword, 0xF1);
      // Providing only an immediate with no base register and no index
      // register produces the RIP relative form
      e.emitM(op, noreg, noreg, sz::byte, 0xF1F1);
    }

    if (supported_AM[i] & MASK_RM) {
      printf("  Address mode: RM\n");
      e.emitRM(op, rsi, rdi, sz::word, -128, rax);
      e.emitRM(op, rbp, noreg, sz::dword, 127, rcx);
      e.emitRM(op, noreg, rbp, sz::byte, 127, rcx);
      e.emitRM(op, rsp, rbp, sz::qword, 0xF1, rbp);
      e.emitRM(op, noreg, noreg, sz::byte, 0xF1F1, rsp);
      e.emitRM(op, rsi, rdi, sz::word, -128, r8);
      e.emitRM(op, rbp, noreg, sz::dword, 127, r9);
      e.emitRM(op, noreg, rbp, sz::byte, 127, r9);
      e.emitRM(op, rsp, rbp, sz::qword, 0xF1, r13);
      e.emitRM(op, noreg, noreg, sz::byte, 0xF1F1, r12);
      e.emitRM(op, r14, rdi, sz::word, -128, rax);
      e.emitRM(op, r13, noreg, sz::dword, 127, rcx);
      e.emitRM(op, noreg, rbp, sz::byte, 127, rcx);
      e.emitRM(op, r12, rbp, sz::qword, 0xF1, rbp);
      e.emitRM(op, noreg, noreg, sz::byte, 0xF1F1, rsp);
      e.emitRM(op, r14, rdi, sz::word, -128, r8);
      e.emitRM(op, r13, noreg, sz::dword, 127, r9);
      e.emitRM(op, noreg, rbp, sz::byte, 127, r9);
      e.emitRM(op, r12, rbp, sz::qword, 0xF1, r13);
      e.emitRM(op, noreg, noreg, sz::byte, 0xF1F1, r12);
    }

    if (supported_AM[i] & MASK_MR) {
      printf("  Address mode: MR\n");
      e.emitMR(op, rsi, rdi, sz::word, -128, rax);
      e.emitMR(op, rbp, noreg, sz::dword, 127, rcx);
      e.emitMR(op, noreg, rbp, sz::byte, 127, rcx);
      e.emitMR(op, rsp, rbp, sz::qword, 0xF1, rbp);
      e.emitMR(op, noreg, noreg, sz::byte, 0xF1F1, rsp);
      e.emitMR(op, rsi, rdi, sz::word, -128, r8);
      e.emitMR(op, rbp, noreg, sz::dword, 127, r9);
      e.emitMR(op, noreg, rbp, sz::byte, 127, r9);
      e.emitMR(op, rsp, rbp, sz::qword, 0xF1, r13);
      e.emitMR(op, noreg, noreg, sz::byte, 0xF1F1, r12);
      e.emitMR(op, r14, rdi, sz::word, -128, rax);
      e.emitMR(op, r13, noreg, sz::dword, 127, rcx);
      e.emitMR(op, noreg, rbp, sz::byte, 127, rcx);
      e.emitMR(op, r12, rbp, sz::qword, 0xF1, rbp);
      e.emitMR(op, noreg, noreg, sz::byte, 0xF1F1, rsp);
      e.emitMR(op, r14, rdi, sz::word, -128, r8);
      e.emitMR(op, r13, noreg, sz::dword, 127, r9);
      e.emitMR(op, noreg, rbp, sz::byte, 127, r9);
      e.emitMR(op, r12, rbp, sz::qword, 0xF1, r13);
      e.emitMR(op, noreg, noreg, sz::byte, 0xF1F1, r12);
    }

    if (supported_AM[i] & MASK_IRM) {
      printf("  Address mode: IRM\n");
      e.emitIRM(op, rsi, rdi, sz::word, -128, rax, 0xF1F1F1F1);
      e.emitIRM(op, rbp, noreg, sz::dword, 127, rcx, 0xF1);
      e.emitIRM(op, noreg, rbp, sz::byte, 127, rcx, 0xF1);
      e.emitIRM(op, rsp, rbp, sz::qword, 0xF1, rbp, -128);
      e.emitIRM(op, noreg, noreg, sz::byte, 0xF1F1, rsp, 127);
      e.emitIRM(op, noreg, noreg, sz::byte, 0xF1F1, rsp, 1);
      e.emitIRM(op, rsi, rdi, sz::word, -128, r8, 0xF1F1F1F1);
      e.emitIRM(op, rbp, noreg, sz::dword, 127, r9, 0xF1);
      e.emitIRM(op, noreg, rbp, sz::byte, 127, r9, 0xF1);
      e.emitIRM(op, rsp, rbp, sz::qword, 0xF1, r13, -128);
      e.emitIRM(op, noreg, noreg, sz::byte, 0xF1F1, r12, 127);
      e.emitIRM(op, noreg, noreg, sz::byte, 0xF1F1, r12, 1);
      e.emitIRM(op, r14, rdi, sz::word, -128, rax, 0xF1F1F1F1);
      e.emitIRM(op, r13, noreg, sz::dword, 127, rcx, 0xF1);
      e.emitIRM(op, noreg, rbp, sz::byte, 127, rcx, 0xF1);
      e.emitIRM(op, r12, rbp, sz::qword, 0xF1, rbp, -128);
      e.emitIRM(op, noreg, noreg, sz::byte, 0xF1F1, rsp, 127);
      e.emitIRM(op, noreg, noreg, sz::byte, 0xF1F1, rsp, 1);
      e.emitIRM(op, r14, rdi, sz::word, -128, r8, 0xF1F1F1F1);
      e.emitIRM(op, r13, noreg, sz::dword, 127, r9, 0xF1);
      e.emitIRM(op, noreg, rbp, sz::byte, 127, r9, 0xF1);
      e.emitIRM(op, r12, rbp, sz::qword, 0xF1, r13, -128);
      e.emitIRM(op, noreg, noreg, sz::byte, 0xF1F1, r12, 127);
      e.emitIRM(op, noreg, noreg, sz::byte, 0xF1F1, r12, 1);
    }

    if (supported_AM[i] & MASK_IMR) {
      printf("  Address mode: IMR\n");
      e.emitIMR(op, rsi, rdi, sz::word, -128, rax, 0xF1F1F1F1);
      e.emitIMR(op, rbp, noreg, sz::dword, 127, rax, 0xF1);
      e.emitIMR(op, noreg, rbp, sz::byte, 127, rcx, 0xF1);
      e.emitIMR(op, rsp, rbp, sz::qword, 0xF1, rbp, -128);
      e.emitIMR(op, noreg, noreg, sz::byte, 0xF1F1, rsp, 127);
      e.emitIMR(op, noreg, noreg, sz::byte, 0xF1F1, rsp, 1);
      e.emitIMR(op, rsi, rdi, sz::word, -128, r8, 0xF1F1F1F1);
      e.emitIMR(op, rbp, noreg, sz::dword, 127, r8, 0xF1);
      e.emitIMR(op, noreg, rbp, sz::byte, 127, r9, 0xF1);
      e.emitIMR(op, rsp, rbp, sz::qword, 0xF1, r13, -128);
      e.emitIMR(op, noreg, noreg, sz::byte, 0xF1F1, r12, 127);
      e.emitIMR(op, noreg, noreg, sz::byte, 0xF1F1, r12, 1);
      e.emitIMR(op, r14, rdi, sz::word, -128, rax, 0xF1F1F1F1);
      e.emitIMR(op, r13, noreg, sz::dword, 127, rax, 0xF1);
      e.emitIMR(op, noreg, rbp, sz::byte, 127, rcx, 0xF1);
      e.emitIMR(op, r12, rbp, sz::qword, 0xF1, rbp, -128);
      e.emitIMR(op, noreg, noreg, sz::byte, 0xF1F1, rsp, 127);
      e.emitIMR(op, noreg, noreg, sz::byte, 0xF1F1, rsp, 1);
      e.emitIMR(op, r14, rdi, sz::word, -128, r8, 0xF1F1F1F1);
      e.emitIMR(op, r13, noreg, sz::dword, 127, r8, 0xF1);
      e.emitIMR(op, noreg, rbp, sz::byte, 127, r9, 0xF1);
      e.emitIMR(op, r12, rbp, sz::qword, 0xF1, r13, -128);
      e.emitIMR(op, noreg, noreg, sz::byte, 0xF1F1, r12, 127);
      e.emitIMR(op, noreg, noreg, sz::byte, 0xF1F1, r12, 1);
    }

    if (supported_AM[i] & MASK_IM) {
      printf("  Address mode: IM\n");
      e.emitIM(op, rsi, rdi, sz::word, -128, 0xF1F1F1F1);
      e.emitIM(op, rbp, noreg, sz::dword, 127, 0xF1);
      e.emitIM(op, noreg, rbp, sz::byte, 127, 0xF1);
      e.emitIM(op, rsp, rbp, sz::qword, 0xF1, -128);
      e.emitIM(op, noreg, noreg, sz::byte, 0xF1F1, 127);
      e.emitIM(op, noreg, noreg, sz::byte, 0xF1F1, 1);
      e.emitIM(op, r14, rdi, sz::word, -128, 0xF1F1F1F1);
      e.emitIM(op, r13, noreg, sz::dword, 127, 0xF1);
      e.emitIM(op, noreg, rbp, sz::byte, 127, 0xF1);
      e.emitIM(op, r12, rbp, sz::qword, 0xF1, -128);
      e.emitIM(op, noreg, noreg, sz::byte, 0xF1F1, 127);
      e.emitIM(op, noreg, noreg, sz::byte, 0xF1F1, 1);
      e.emitIM(op, rsi, r15, sz::word, -128, 0xF1F1F1F1);
      e.emitIM(op, rbp, noreg, sz::dword, 127, 0xF1);
      e.emitIM(op, noreg, r13, sz::byte, 127, 0xF1);
      e.emitIM(op, rsp, r13, sz::qword, 0xF1, -128);
      e.emitIM(op, noreg, noreg, sz::byte, 0xF1F1, 127);
      e.emitIM(op, noreg, noreg, sz::byte, 0xF1F1, 1);
      if (i == 13 /*instr_mov*/) {
        e.emitIM(op, rsi, rdi, sz::word, -128, 0x1234123412341234);
        e.emitIM(op, r14, rdi, sz::word, -128, 0x1234123412341234);
        e.emitIM(op, rsi, r15, sz::word, -128, 0x1234123412341234);
      }
    }

    if (supported_AM[i] & MASK_CI) {
      printf("  Address mode: CI\n");
      e.emitCI(op, CC_NE, 0xF1F1F1F1);
      e.emitCI(op, CC_G, 0xF1);
      e.emitCI(op, CC_S, -128);
      e.emitCI(op, CC_O, 127);
      e.emitCI(op, CC_O, 1);
    }

    if (supported_AM[i] & MASK_CMR) {
      printf("  Address mode: CMR\n");
      e.emitCMR(op, CC_NE, rsi, rdi, sz::word, -128, rax);
      e.emitCMR(op, CC_G, rbp, noreg, sz::dword, 127, rcx);
      e.emitCMR(op, CC_G, noreg, rbp, sz::byte, 127, rcx);
      e.emitCMR(op, CC_S, rsp, rbp, sz::qword, 0xF1, rbp);
      e.emitCMR(op, CC_O, noreg, noreg, sz::byte, 0xF1F1, rsp);
      e.emitCMR(op, CC_NE, rsi, rdi, sz::word, -128, r8);
      e.emitCMR(op, CC_G, rbp, noreg, sz::dword, 127, r9);
      e.emitCMR(op, CC_G, noreg, rbp, sz::byte, 127, r9);
      e.emitCMR(op, CC_S, rsp, rbp, sz::qword, 0xF1, r13);
      e.emitCMR(op, CC_O, noreg, noreg, sz::byte, 0xF1F1, r12);
      e.emitCMR(op, CC_NE, r14, rdi, sz::word, -128, rax);
      e.emitCMR(op, CC_G, r13, noreg, sz::dword, 127, rcx);
      e.emitCMR(op, CC_G, noreg, rbp, sz::byte, 127, rcx);
      e.emitCMR(op, CC_S, r12, rbp, sz::qword, 0xF1, rbp);
      e.emitCMR(op, CC_O, noreg, noreg, sz::byte, 0xF1F1, rsp);
      e.emitCMR(op, CC_NE, r14, rdi, sz::word, -128, r8);
      e.emitCMR(op, CC_G, r12, noreg, sz::dword, 127, r9);
      e.emitCMR(op, CC_G, noreg, rbp, sz::byte, 127, r9);
      e.emitCMR(op, CC_S, r12, rbp, sz::qword, 0xF1, r13);
      e.emitCMR(op, CC_O, noreg, noreg, sz::byte, 0xF1F1, r12);
    }

    if (supported_AM[i] & MASK_CR) {
      printf("  Address mode: CR\n");
      e.emitCR(op, CC_NE, rax);
      e.emitCR(op, CC_G, rcx);
      e.emitCR(op, CC_S, rbp);
      e.emitCR(op, CC_O, rsp);
      e.emitCR(op, CC_NE, r8);
      e.emitCR(op, CC_G, r9);
      e.emitCR(op, CC_S, r13);
      e.emitCR(op, CC_O, r12);
    }

    if (supported_AM[i] & MASK_CM) {
      printf("  Address mode: CM\n");
      e.emitCM(op, CC_NE, rsi, rdi, sz::word, -128);
      e.emitCM(op, CC_G, rbp, noreg, sz::dword, 127);
      e.emitCM(op, CC_G, noreg, rbp, sz::byte, 127);
      e.emitCM(op, CC_S, rsp, rbp, sz::qword, 0xF1);
      e.emitCM(op, CC_S, rbp, rbp, sz::qword, 0xF1);
      e.emitCM(op, CC_O, noreg, noreg, sz::byte, 0xF1F1);
      e.emitCM(op, CC_NE, rsi, r15, sz::word, -128);
      e.emitCM(op, CC_G, rbp, noreg, sz::dword, 127);
      e.emitCM(op, CC_G, noreg, r13, sz::byte, 127);
      e.emitCM(op, CC_S, rsp, r13, sz::qword, 0xF1);
      e.emitCM(op, CC_O, noreg, noreg, sz::byte, 0xF1F1);
      e.emitCM(op, CC_NE, r14, rdi, sz::word, -128);
      e.emitCM(op, CC_G, r13, noreg, sz::dword, 127);
      e.emitCM(op, CC_G, noreg, rbp, sz::byte, 127);
      e.emitCM(op, CC_S, r12, rbp, sz::qword, 0xF1);
      e.emitCM(op, CC_O, noreg, noreg, sz::byte, 0xF1F1);
      e.emitCM(op, CC_NE, r14, r15, sz::word, -128);
      e.emitCM(op, CC_G, r13, noreg, sz::dword, 127);
      e.emitCM(op, CC_G, noreg, r13, sz::byte, 127);
      e.emitCM(op, CC_S, r12, r13, sz::qword, 0xF1);
      e.emitCM(op, CC_O, noreg, noreg, sz::byte, 0xF1F1);
    }

    if (supported_AM[i] & MASK_CRR) {
      printf("  Address mode: CRR\n");
      e.emitCRR(op, CC_NE, rsi, rax);
      e.emitCRR(op, CC_G, rax, rdi);
      e.emitCRR(op, CC_S, rsi, rdi);
      e.emitCRR(op, CC_O, rbp, rsp);
      e.emitCRR(op, CC_E, rsp, rbp);
      e.emitCRR(op, CC_NP, rsi, r8);
      e.emitCRR(op, CC_P, rax, r15);
      e.emitCRR(op, CC_L, rsi, r15);
      e.emitCRR(op, CC_LE, rbp, r12);
      e.emitCRR(op, CC_BE, rsp, r13);
      e.emitCRR(op, CC_NO, r14, rax);
      e.emitCRR(op, CC_NS, r8, rdi);
      e.emitCRR(op, CC_NE, r14, rdi);
      e.emitCRR(op, CC_G, r13, rsp);
      e.emitCRR(op, CC_S, r12, rbp);
      e.emitCRR(op, CC_O, r14, r8);
      e.emitCRR(op, CC_E, r8, r15);
      e.emitCRR(op, CC_NP, r14, r15);
      e.emitCRR(op, CC_P, r13, r12);
      e.emitCRR(op, CC_L, r12, r13);
      e.emitCRR(op, CC_LE, rax, rax);
      e.emitCRR(op, CC_BE, rax, r8);
      e.emitCRR(op, CC_NO, r8, rax);
      e.emitCRR(op, CC_NS, r8, r8);
    }
    printf("\n");
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}
}

