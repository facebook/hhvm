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
#include "hphp/vixl/a64/disasm-a64.h"
#include "hphp/vixl/a64/instructions-a64.h"

#define MAX_INSTR_ASM_LEN 128

using std::string;
using std::vector;

namespace HPHP { namespace jit {

#if defined(__aarch64__)

using namespace vixl;

const char* OfflineCode::getArchName() { return "A64"; }


TCA OfflineCode::collectJmpTargets(FILE *file,
                                   TCA fileStartAddr,
                                   TCA codeStartAddr,
                                   uint64_t codeLen,
                                   vector<TCA> *jmpTargets) {

  if (codeLen == 0) return 0;

  Instruction* code = (Instruction*) alloca(codeLen);
  Instruction* frontier;
  TCA ip;

  if (fseek(file, codeStartAddr - fileStartAddr, SEEK_SET)) {
    error("collectJmpTargets error: seeking file");
  }

  size_t readLen = fread(code, codeLen, 1, file);
  if (readLen != 1) error("collectJmpTargets error: reading file");

  for (frontier = code, ip = codeStartAddr; frontier < code + codeLen; ) {
    TCA addr = 0;

    if (frontier->IsCondBranchImm()) {
      addr = ip + kInstructionSize +  (frontier->ImmCondBranch() << 2);
      jmpTargets->push_back(addr);
    } else if (frontier->IsCompareBranch()) {
      addr = ip + kInstructionSize +  frontier->ImmCmpBranch();
      jmpTargets->push_back(addr);
    } else if (frontier->IsTestBranch()) {
      addr = ip + kInstructionSize +  frontier->ImmTestBranch();
      jmpTargets->push_back(addr);
    }

    frontier += kInstructionSize;
    ip += kInstructionSize;
  }

  // if the code sequence falls thru, then the next instruction
  // is a possible target.
  if (frontier->IsCondBranchImm()) {
    jmpTargets->push_back(ip);
    return ip;
  }

  return 0;
}


void OfflineCode::disasm(FILE* file,
                         TCA fileStartAddr,
                         TCA codeStartAddr,
                         uint64_t codeLen,
                         const PerfEventsMap<TCA>& perfEvents,
                         BCMappingInfo bcMappingInfo,
                         bool printAddr,
                         bool printBinary) {

  if (codeLen == 0) return;

  char codeStr[MAX_INSTR_ASM_LEN];
  Instruction* code = (Instruction*) alloca(codeLen);
  Instruction* frontier;
  TCA ip;
  size_t  currBC = 0;

  auto const offset = codeStartAddr - fileStartAddr;
  if (fseek(file, offset, SEEK_SET)) {
    error("disasm error: seeking file");
  }

  size_t readLen = fread(code, codeLen, 1, file);
  if (readLen != 1) {
    error("Failed to read {} bytes at offset {} from code file due to {}",
          codeLen, offset, feof(file) ? "EOF" : "read error");
  }

  Decoder dec;
  Disassembler dis(codeStr, MAX_INSTR_ASM_LEN);
  dis.MapCodeAddress((int64_t)codeStartAddr, code);
  dec.AppendVisitor(&dis);
  int64_t callAddr = 0;
  Instr insn;

  const Instr MOVZ_x18 = MOVZ_x | 0x12;            // movz x18, #0x...
  const Instr MOVK_x18 = MOVK_x | 0x12;            // movk x18, #0x... {, lsl #N}
  const Instr MoveWideImmX18Mask = 0xFF800012;

  const Instr LDR_x_litx18 = LDR_x_lit | 0xffffd2; // ldr  x18, pc-8
  const Instr BLR_x18 = BLR | 0x240;               // blr  x18

  for (frontier = code, ip = codeStartAddr; frontier < code + codeLen; ) {
    dec.Decode(frontier);

    currBC = printBCMapping(bcMappingInfo, currBC, (TCA)ip);
    if (printAddr) printf("%14p: ", ip);

    if (printBinary) {
      printf("%08" PRIx32 , *reinterpret_cast<int32_t *>(frontier));
      printf("%10s","");
    }

    // Shadow potential call destinations based on fixed sequence.
    // This needs to match code generation in JIT.
    //
    //   movz x18, #...        ldr x18, pc-8
    //   movk x18, #...        blr x18
    //   blr x18

    insn = frontier->InstructionBits();
    if ((insn & MoveWideImmX18Mask) == MOVZ_x18) {
      callAddr = static_cast<uint32_t>((insn & 0x1fffd0) >> ImmMoveWide_offset);

    } else if ((insn & MoveWideImmX18Mask) == MOVK_x18) {
      callAddr |= ((insn & 0x1fffe0) >> 5) << (16 * ((insn & 0x600000) >> ShiftMoveWide_offset));

    } else if (insn == LDR_x_litx18) {
      callAddr = static_cast<int64_t>((frontier-4)->InstructionBits()) << 32;
      callAddr |= (frontier-8)->InstructionBits();
      callAddr = callAddr + (int64_t)ip;
      callAddr = callAddr + (int64_t)kInstructionSize;
    }

    string callDest="";
    if ((insn & BLR_x18) == BLR_x18) {
      callDest = getSymbolName((TCA)callAddr);
      callAddr = 0;
    }

    if (! perfEvents.empty()) {
      printEventStats((TCA)ip, kInstructionSize, perfEvents);
    } else {
      printf("%48s", "");
    }

    printf("%s%s\n", codeStr, callDest.c_str());

    frontier += kInstructionSize;
    ip += kInstructionSize;
  }
}

#endif

} } // HPHP::jit
