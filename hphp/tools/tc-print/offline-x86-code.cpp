/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/tools/tc-print/offline-x86-code.h"

#include <stdio.h>
#include <cxxabi.h>
#include <vector>
#include <assert.h>
#include <iomanip>
#include <sys/stat.h>

#include "hphp/tools/tc-print/tc-print.h"
#include "hphp/tools/tc-print/offline-trans-data.h"

#define MAX_INSTR_ASM_LEN 128
#define MAX_SYM_LEN       10240

using std::string;
using std::vector;
using std::pair;

namespace HPHP { namespace jit {

string TCRegionString[] = {
  "hot", "main", "profile", "cold", "frozen"
};

static string helpersMapFileName("/tc_dump_helpers_addrs.txt");
static string nmMapFileName("/hhvm.nm");
static string tcRegionFileNames[TCRCount] = { "/tc_dump_ahot",
                                              "/tc_dump_a",
                                              "/tc_dump_aprof",
                                              "/tc_dump_acold",
                                              "/tc_dump_afrozen" };

static size_t fileSize(FILE* f) {
  auto fd = fileno(f);
  struct stat st;
  fstat(fd, &st);
  return st.st_size;
}

void OfflineX86Code::openFiles(TCA tcRegionBases[TCRCount]) {

  for (size_t i = 0; i < TCRCount; i++) {
    string fileName = dumpDir + tcRegionFileNames[i];
    tcRegions[i].file = fopen(fileName.c_str(), "rb");
    if (!tcRegions[i].file) {
      for (size_t o = 0; o < i; o++) {
        fclose(tcRegions[o].file);
      }
      error("Error opening file " + fileName);
    }
    tcRegions[i].baseAddr = tcRegionBases[i];
    tcRegions[i].len = fileSize(tcRegions[i].file);
  }
}

void OfflineX86Code::closeFiles() {
  for (size_t i = 0; i < TCRCount; i++) {
    fclose(tcRegions[i].file);
  }
}

bool OfflineX86Code::tcRegionContains(TCRegion tcr, TCA addr) const {
  assert(tcr >= 0 && tcr < TCRCount);
  return (addr >= tcRegions[tcr].baseAddr &&
          addr <  tcRegions[tcr].baseAddr + tcRegions[tcr].len);
}

// Returns TCRegion containing addr if any, TCRCount otherwise.
TCRegion OfflineX86Code::findTCRegionContaining(TCA addr) const {
  for (int tcr = 0; tcr < TCRCount; tcr++) {
    if (tcRegionContains((TCRegion)tcr, addr)) return (TCRegion)tcr;
  }
  return TCRCount;
}

void OfflineX86Code::xedInit() {
  xed_state_init(&xed_state, XED_MACHINE_MODE_LONG_64,
                 XED_ADDRESS_WIDTH_64b, XED_ADDRESS_WIDTH_64b);
  xed_tables_init();
  xed_syntax = getenv("HHVM_ATT_DISAS") ? XED_SYNTAX_ATT : XED_SYNTAX_INTEL;
}


TCA OfflineX86Code::getTransJmpTargets(const TransRec *transRec,
                                       vector<TCA> *jmpTargets) {

  TCRegion tcrMain = findTCRegionContaining(transRec->aStart);

  assert(tcrMain == TCRHot || tcrMain == TCRMain || tcrMain == TCRProfile);

  TCA aFallThru = collectJmpTargets(tcRegions[tcrMain].file,
                                    tcRegions[tcrMain].baseAddr,
                                    transRec->aStart, transRec->aLen,
                                    jmpTargets);

  collectJmpTargets(tcRegions[TCRCold].file,
                    tcRegions[TCRCold].baseAddr,
                    transRec->acoldStart, transRec->acoldLen, jmpTargets);

  collectJmpTargets(tcRegions[TCRFrozen].file,
                    tcRegions[TCRFrozen].baseAddr,
                    transRec->afrozenStart, transRec->afrozenLen, jmpTargets);

  return aFallThru;
}

TCA OfflineX86Code::collectJmpTargets(FILE  *file,
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

void OfflineX86Code::printDisasm(TCA startAddr, uint32_t len,
                                 const vector<TransBCMapping>& bcMap,
                                 const PerfEventsMap<TCA>& perfEvents) {
  TCRegion tcr = findTCRegionContaining(startAddr);
  disasm(tcRegions[tcr].file, tcRegions[tcr].baseAddr, startAddr, len,
         perfEvents, BCMappingInfo(tcr, bcMap));
}

// Disassemble the code from the given raw file, whose initial address is given
// by fileStartAddr, for the address range given by
// [codeStartAddr, codeStartAddr + codeLen)

void OfflineX86Code::disasm(FILE* file,
                            TCA   fileStartAddr,
                            TCA   codeStartAddr,
                            uint64_t codeLen,
                            const PerfEventsMap<TCA>& perfEvents,
                            BCMappingInfo bcMappingInfo,
                            bool printAddr /* =true */,
                            bool printBinary /* =false */) {

  char codeStr[MAX_INSTR_ASM_LEN];
  xed_uint8_t* code = (xed_uint8_t*) alloca(codeLen);
  xed_uint8_t* frontier;
  TCA          ip;
  TCA          r10val = 0;
  size_t       currBC = 0;

  if (codeLen == 0) return;

  if (fseek(file, codeStartAddr - fileStartAddr, SEEK_SET)) {
    error("disasm error: seeking file");
  }

  size_t readLen = fread(code, codeLen, 1, file);
  if (readLen != 1) error("disasm error: reading file");

  xed_decoded_inst_t xedd;

  // Decode and print each instruction
  for (frontier = code, ip = codeStartAddr; frontier < code + codeLen; ) {

    xed_decoded_inst_zero_set_mode(&xedd, &xed_state);
    xed_decoded_inst_set_input_chip(&xedd, XED_CHIP_INVALID);
    xed_error_enum_t xed_error = xed_decode(&xedd, frontier, 15);

    if (xed_error != XED_ERROR_NONE) break;

    // Get disassembled instruction in codeStr
    if (!xed_format_context(xed_syntax, &xedd, codeStr,
                            MAX_INSTR_ASM_LEN, (uint64_t)ip, nullptr)) {
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

void OfflineX86Code::loadSymbolsMap() {
  loadSymbolsMapNm();
  loadSymbolsMapTramp();
}


void OfflineX86Code::loadSymbolsMapTramp() {
  FILE* helpersMapFile;

  string helpersFileName = dumpDir + helpersMapFileName;
  helpersMapFile = fopen(helpersFileName.c_str(), "rt");

  if (!helpersMapFile) return;

  TCA trampAddr, helperAddr;
  char symName[MAX_SYM_LEN];
  uint32_t count=0;

  while (fscanf(helpersMapFile, "%p %p ", (void**)&trampAddr,
                (void**)&helperAddr) == 2) {
    if (fgets(symName, MAX_SYM_LEN, helpersMapFile) == nullptr) break;

    // remove trailing '\n'
    size_t symLen = strlen(symName);
    if (symLen && symName[symLen - 1] == '\n') {
      symName[symLen - 1] = 0;
    }
    string strSymName = symName;
    addr2SymMap[trampAddr] = strSymName;
    addr2SymMap[helperAddr] = strSymName;
    count++;
  }

  printf("# Read %u symbols from file %s\n", count, helpersFileName.c_str());

  fclose(helpersMapFile);
}


void OfflineX86Code::loadSymbolsMapNm() {
  FILE* nmMapFile;

  string nmFileName = dumpDir + nmMapFileName;
  nmMapFile = fopen(nmFileName.c_str(), "rt");

  if (!nmMapFile) return;

  TCA symAddr;
  char symName[MAX_SYM_LEN], line[2*MAX_SYM_LEN];
  uint32_t count=0;

  while (fgets(line, 2*MAX_SYM_LEN, nmMapFile) != nullptr) {
    if (sscanf(line, "%p %*s %s", &symAddr, symName) == 2) {

      int status;
      char* demangledName = abi::__cxa_demangle(symName, 0, 0, &status);
      if (demangledName) {
        addr2SymMap[symAddr] = string(demangledName);
        free(demangledName);
      } else {
        addr2SymMap[symAddr] = string(symName);
      }
    }
    count++;
  }
  printf("# Read %u symbols from file %s\n", count, nmFileName.c_str());

  fclose(nmMapFile);
}


// Returns the name of the symbol of the given address if available, otherwise
// just returns the address
string OfflineX86Code::getSymbolName(TCA addr) {
  string sym;
  auto it = addr2SymMap.find(addr);
  if (it != addr2SymMap.end()) {
    sym = "  # " + it->second;
  } else {
    char addrStr[50];
    sprintf(addrStr, "%p", addr);
    sym = "  # SYMBOL @ " + string(addrStr);
  }
  return sym;
}

size_t OfflineX86Code::printBCMapping(BCMappingInfo bcMappingInfo,
                                      size_t currBC,
                                      TCA ip) {

  TransBCMapping curr, next;
  TCA x86Start, x86Stop;
  auto const& bcMap = bcMappingInfo.bcMapping;

  curr = next = TransBCMapping { MD5(), 0, 0, 0, 0 };
  x86Start = x86Stop = 0;

  // Account for the sentinel.
  size_t mappingSize = bcMap.size() - 1;

  // Starting from currBC, find the next bytecode with a non-empty x86 range
  // that could potentially correspond to instruction ip.
  for (; currBC < mappingSize; ++currBC) {
    curr = bcMap[currBC];
    next = bcMap[currBC + 1];

    switch (bcMappingInfo.tcRegion) {
      case TCRHot:
      case TCRMain:
      case TCRProfile:
        x86Start = curr.aStart;
        x86Stop  = next.aStart;
        break;
      case TCRCold:
        x86Start = curr.acoldStart;
        x86Stop  = next.acoldStart;
        break;
      case TCRFrozen:
        x86Start = curr.afrozenStart;
        x86Stop  = next.afrozenStart;
        break;
      default:
        error("printBCMapping: unexpected TCRegion");
    }

    always_assert(x86Start <= x86Stop);
    if (x86Start >= ip && x86Start < x86Stop) break;
  }

  if (currBC < mappingSize && x86Start == ip) {
    if (auto currUnit = g_repo->getUnit(curr.md5)) {
      auto bcPast = curr.bcStart + instrLen(
        reinterpret_cast<const Op*>(currUnit->at(curr.bcStart)));

      currUnit->prettyPrint(std::cout,
                            Unit::PrintOpts().range(curr.bcStart,
                                                    bcPast));
    } else {
      std::cout << folly::format(
        "<<< couldn't find unit {} to print bytecode at offset {} >>>\n",
        curr.md5, curr.bcStart);
    }

    currBC++;
  }

  return currBC;
}

void OfflineX86Code::printEventStats(TCA address,
                                     uint32_t instrLen,
                                     const PerfEventsMap<TCA>& perfEvents) {
  static const PerfEventType AnnotatedEvents[] = {
    EVENT_CYCLES,
    EVENT_BRANCH_MISSES,
    EVENT_ICACHE_MISSES,
    EVENT_DCACHE_MISSES,
    EVENT_LLC_MISSES,
    EVENT_ITLB_MISSES,
    EVENT_DTLB_MISSES,
  };

  const size_t NumAnnotatedEvents =
    sizeof(AnnotatedEvents) / sizeof(AnnotatedEvents[0]);

  static const char* SmallCaptions[] = {"cy", "bm", "ic", "dc", "lc", "it",
                                        "dt"};

  assert(sizeof(SmallCaptions)/sizeof(SmallCaptions[0]) == NumAnnotatedEvents);

  for (size_t i = 0; i < NumAnnotatedEvents; i++) {
    uint64_t eventCount = perfEvents.getEventCount(address,
                                                   address + instrLen - 1,
                                                   AnnotatedEvents[i]);
    std::string eventStr;
    if (eventCount) {
      eventStr = folly::format("{:>3}:{:>4}",
                               SmallCaptions[i], eventCount).str();
    }
    std::cout << folly::format("{:<10} ", eventStr);
  }
}

} } // HPHP::jit
