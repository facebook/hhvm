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

#include <vector>
#include <iomanip>



#define MAX_INSTR_ASM_LEN 128


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

TCRegionInfo OfflineCode::getRegionInfo(FILE* file,
                                         TCA fileStartAddr,
                                         TCA codeStartAddr,
                                         uint64_t codeLen,
                                         const PerfEventsMap<TCA>& perfEvents,
                                         BCMappingInfo bcMappingInfo) {
  if (codeLen == 0) return TCRegionInfo{bcMappingInfo.tcRegion};

  auto const codeEndAddr = codeStartAddr + codeLen;
  TCRegionInfo regionInfo{bcMappingInfo.tcRegion,
                           getRanges(bcMappingInfo,
                                     codeStartAddr,
                                     codeEndAddr)};
  auto& ranges = regionInfo.ranges;

  char codeStr[MAX_INSTR_ASM_LEN];
  Instruction* code = (Instruction*) alloca(codeLen);
  Instruction* frontier;
  TCA ip;
  size_t  currBC = 0;

  auto const offset = codeStartAddr - fileStartAddr;
  readDisasmFile(file, offset, codeLen, code);

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

    auto const binaryStr = [&] {
      std::ostringstream binary_os;
      binary_os << folly::format("{:08" PRIx32 "}",
                                 *reinterpret_cast<int32_t*>(frontier));
      binary_os << string(10, ' ');
      return binary_os.str();
    }();

    while (currBC < ranges.size() - 1 && ip >= ranges[currBC].end) currBC++;

    auto const disasmInfo = getDisasmInfo(ip, kInstructionSize, perfEvents,
                                          binaryStr, callDest, codeStr);
    ranges[currBC].disasm.push_back(disasmInfo);

    frontier += kInstructionSize;
    ip += kInstructionSize;
  }
  return regionInfo;
}

#endif

} } // HPHP::jit
