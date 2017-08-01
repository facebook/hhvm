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
#include "hphp/tools/tc-print/offline-code.h"

#include <stdio.h>
#include <cxxabi.h>
#include <vector>
#include <assert.h>
#include <iomanip>
#include <sys/stat.h>

#include "hphp/tools/tc-print/tc-print.h"
#include "hphp/tools/tc-print/offline-trans-data.h"

#include "hphp/util/disasm.h"

#define MAX_INSTR_ASM_LEN 128

using std::string;
using std::vector;

namespace HPHP { namespace jit {

#if defined(__x86_64__)
#if defined(HAVE_LIBXED)

const char* OfflineCode::getArchName() { return "X64"; }

void OfflineCode::xedInit() {
  xed_state_init(&xed_state, XED_MACHINE_MODE_LONG_64,
                 XED_ADDRESS_WIDTH_64b, XED_ADDRESS_WIDTH_64b);
  xed_tables_init();
  xed_syntax = getenv("HHVM_INTEL_DISAS") ? XED_SYNTAX_INTEL : XED_SYNTAX_ATT;
}

TCA OfflineCode::collectJmpTargets(FILE  *file,
                                      TCA    fileStartAddr,
                                      TCA    codeStartAddr,
                                      uint64_t codeLen,
                                      vector<TCA> *jmpTargets) {

  xed_uint8_t* code = (xed_uint8_t*) alloca(codeLen);
  xed_uint8_t* frontier;
  TCA          ip;

  if (codeLen == 0) return 0;

  if (fseek(file, codeStartAddr - fileStartAddr, SEEK_SET)) {
    error("collectJmpTargets error: seeking file");
  }

  size_t readLen = fread(code, codeLen, 1, file);
  if (readLen != 1) error("collectJmpTargets error: reading file");

  xed_decoded_inst_t xedd;
  xed_iclass_enum_t iclass = XED_ICLASS_NOP;

  // Decode each instruction
  for (frontier = code, ip = codeStartAddr; frontier < code + codeLen; ) {

    xed_decoded_inst_zero_set_mode(&xedd, &xed_state);
    xed_decoded_inst_set_input_chip(&xedd, XED_CHIP_INVALID);
    xed_error_enum_t xed_error = xed_decode(&xedd, frontier, 15);

    if (xed_error != XED_ERROR_NONE) break;

    uint32_t instrLen = xed_decoded_inst_get_length(&xedd);

    iclass = xed_decoded_inst_get_iclass(&xedd);

    if (iclass >= XED_ICLASS_JB && iclass <= XED_ICLASS_JZ) {
      const xed_inst_t    *xi       = xed_decoded_inst_inst(&xedd);
      always_assert(xed_inst_noperands(xi) >= 1);
      const xed_operand_t *opnd     = xed_inst_operand(xi, 0);
      xed_operand_enum_t   opndName = xed_operand_name(opnd);

      if (opndName == XED_OPERAND_RELBR) {
        always_assert(xed_decoded_inst_get_branch_displacement_width(&xedd));
        xed_int32_t disp = xed_decoded_inst_get_branch_displacement(&xedd);
        TCA         addr = ip + instrLen + disp;
        jmpTargets->push_back(addr);
      }
    }

    frontier += instrLen;
    ip       += instrLen;
  }

  // If the code sequence falls thru, then add the next instruction as a
  // possible target
  bool fallsThru = (iclass != XED_ICLASS_JMP      &&
                    iclass != XED_ICLASS_JMP_FAR  &&
                    iclass != XED_ICLASS_RET_NEAR &&
                    iclass != XED_ICLASS_RET_FAR);
  if (fallsThru) {
    jmpTargets->push_back(ip);
    return ip;
  }
  return 0;
}

// Disassemble the code from the given raw file, whose initial address is given
// by fileStartAddr, for the address range given by
// [codeStartAddr, codeStartAddr + codeLen)

void OfflineCode::disasm(FILE* file,
                            TCA   fileStartAddr,
                            TCA   codeStartAddr,
                            uint64_t codeLen,
                            const PerfEventsMap<TCA>& perfEvents,
                            BCMappingInfo bcMappingInfo,
                            bool printAddr,
                            bool printBinary) {

  char codeStr[MAX_INSTR_ASM_LEN];
  xed_uint8_t* code = (xed_uint8_t*) alloca(codeLen);
  xed_uint8_t* frontier;
  TCA          ip;
  TCA          r10val = 0;
  size_t       currBC = 0;

  if (codeLen == 0) return;

  auto const offset = codeStartAddr - fileStartAddr;
  if (fseek(file, offset, SEEK_SET)) {
    error("disasm error: seeking file");
  }

  size_t readLen = fread(code, codeLen, 1, file);
  if (readLen != 1) {
    error("Failed to read {} bytes at offset {} from code file due to {}",
          codeLen, offset, feof(file) ? "EOF" : "read error");
  }

  xed_decoded_inst_t xedd;

  // Decode and print each instruction
  for (frontier = code, ip = codeStartAddr; frontier < code + codeLen; ) {

    xed_decoded_inst_zero_set_mode(&xedd, &xed_state);
    xed_decoded_inst_set_input_chip(&xedd, XED_CHIP_INVALID);
    xed_error_enum_t xed_error = xed_decode(&xedd, frontier, 15);

    if (xed_error != XED_ERROR_NONE) break;

    // Get disassembled instruction in codeStr
    if (!xed_format_context(xed_syntax, &xedd, codeStr,
                            MAX_INSTR_ASM_LEN, (uint64_t)ip, nullptr
#if XED_ENCODE_ORDER_MAX_ENTRIES != 28 // Newer version of XED library
                            , 0
#endif
                           )) {
      error("disasm error: xed_format_context failed");
    }

    // Annotate the x86 with its bytecode.
    currBC = printBCMapping(bcMappingInfo, currBC, (TCA)ip);

    if (printAddr) printf("%14p: ", ip);

    uint32_t instrLen = xed_decoded_inst_get_length(&xedd);

    if (printBinary) {
      uint32_t i;
      for (i=0; i < instrLen; i++) {
        printf("%02X", frontier[i]);
      }
      for (; i < 16; i++) {
        printf("  ");
      }
    }

    // For calls, we try to figure out the destination symbol name.
    // We look both at relative branches and the pattern:
    //    move r10, IMMEDIATE
    //    call r10
    xed_iclass_enum_t iclass = xed_decoded_inst_get_iclass(&xedd);
    string callDest = "";

    if (iclass == XED_ICLASS_CALL_NEAR || iclass == XED_ICLASS_CALL_FAR) {
      const xed_inst_t    *xi       = xed_decoded_inst_inst(&xedd);
      always_assert(xed_inst_noperands(xi) >= 1);
      const xed_operand_t *opnd     = xed_inst_operand(xi, 0);
      xed_operand_enum_t   opndName = xed_operand_name(opnd);

      if (opndName == XED_OPERAND_RELBR) {
        if (xed_decoded_inst_get_branch_displacement_width(&xedd)) {
          xed_int32_t disp = xed_decoded_inst_get_branch_displacement(&xedd);
          TCA         addr = ip + instrLen + disp;
          callDest = getSymbolName(addr);
        }
      } else if (opndName == XED_OPERAND_REG0) {
        if (xed_decoded_inst_get_reg(&xedd, opndName) == XED_REG_R10) {
          callDest = getSymbolName(r10val);
        }
      }
    } else if (iclass == XED_ICLASS_MOV) {
      // Look for moves into r10 and keep r10val updated
      const xed_inst_t* xi = xed_decoded_inst_inst(&xedd);

      always_assert(xed_inst_noperands(xi) >= 2);

      const xed_operand_t *destOpnd     = xed_inst_operand(xi, 0);
      xed_operand_enum_t   destOpndName = xed_operand_name(destOpnd);

      if (destOpndName == XED_OPERAND_REG0 &&
          xed_decoded_inst_get_reg(&xedd, destOpndName) == XED_REG_R10) {
        const xed_operand_t *srcOpnd     = xed_inst_operand(xi, 1);
        xed_operand_enum_t   srcOpndName = xed_operand_name(srcOpnd);
        if (srcOpndName == XED_OPERAND_IMM0) {
          TCA addr = (TCA)xed_decoded_inst_get_unsigned_immediate(&xedd);
          r10val = addr;
        }
      }
    }

    if (!perfEvents.empty()) {
      printEventStats((TCA)ip, instrLen, perfEvents);
    } else {
      printf("%48s", "");
    }
    printf("%s%s\n", codeStr, callDest.c_str());

    frontier += instrLen;
    ip       += instrLen;
  }
}
#else
  // cmake should prevent this.
#error "tc-print on x86_64 requires libxed"
#endif
#endif

} } // HPHP::jit
