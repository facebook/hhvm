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

#define MAX_SYM_LEN       10240

using std::string;
using std::vector;

namespace HPHP { namespace jit {

string TCRegionString[] = {
  "hot", "main", "profile", "cold", "frozen"
};

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

void OfflineCode::openFiles(TCA tcRegionBases[TCRCount]) {

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

void OfflineCode::closeFiles() {
  for (size_t i = 0; i < TCRCount; i++) {
    fclose(tcRegions[i].file);
  }
}

bool OfflineCode::tcRegionContains(TCRegion tcr, TCA addr) const {
  assert(tcr >= 0 && tcr < TCRCount);
  return (addr >= tcRegions[tcr].baseAddr &&
          addr <  tcRegions[tcr].baseAddr + tcRegions[tcr].len);
}

// Returns TCRegion containing addr if any, TCRCount otherwise.
TCRegion OfflineCode::findTCRegionContaining(TCA addr) const {
  for (int tcr = 0; tcr < TCRCount; tcr++) {
    if (tcRegionContains((TCRegion)tcr, addr)) return (TCRegion)tcr;
  }
  return TCRCount;
}


TCA OfflineCode::getTransJmpTargets(const TransRec *transRec,
                                       vector<TCA> *jmpTargets) {

  TCRegion tcrMain = findTCRegionContaining(transRec->aStart);

  assert(tcrMain == TCRHot || tcrMain == TCRMain || tcrMain == TCRProfile);

  TCA aFallThru = collectJmpTargets(tcRegions[tcrMain].file,
                                    tcRegions[tcrMain].baseAddr,
                                    transRec->aStart, transRec->aLen,
                                    jmpTargets);

  // Sometimes acoldStart is the same as afrozenStart.  In these cases, don't
  // look up the address range in the "cold" file, since it the range isn't
  // there.
  if (transRec->acoldStart != transRec->afrozenStart) {
    collectJmpTargets(tcRegions[TCRCold].file,
                      tcRegions[TCRCold].baseAddr,
                      transRec->acoldStart, transRec->acoldLen, jmpTargets);
  }

  collectJmpTargets(tcRegions[TCRFrozen].file,
                    tcRegions[TCRFrozen].baseAddr,
                    transRec->afrozenStart, transRec->afrozenLen, jmpTargets);

  return aFallThru;
}

void OfflineCode::printDisasm(TCA startAddr, uint32_t len,
                                 const vector<TransBCMapping>& bcMap,
                                 const PerfEventsMap<TCA>& perfEvents,
                                 bool hostOpcodes) {
  TCRegion tcr = findTCRegionContaining(startAddr);
  disasm(tcRegions[tcr].file, tcRegions[tcr].baseAddr, startAddr, len,
         perfEvents, BCMappingInfo(tcr, bcMap), true, hostOpcodes);
}

void OfflineCode::loadSymbolsMap() {
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
string OfflineCode::getSymbolName(TCA addr) {
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

size_t OfflineCode::printBCMapping(BCMappingInfo bcMappingInfo,
                                      size_t currBC,
                                      TCA ip) {

  TransBCMapping curr, next;
  TCA tcaStart, tcaStop;
  auto const& bcMap = bcMappingInfo.bcMapping;

  curr = next = TransBCMapping { MD5(), 0, 0, 0, 0 };
  tcaStart = tcaStop = 0;

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
        tcaStart = curr.aStart;
        tcaStop  = next.aStart;
        break;
      case TCRCold:
        tcaStart = curr.acoldStart;
        tcaStop  = next.acoldStart;
        break;
      case TCRFrozen:
        tcaStart = curr.afrozenStart;
        tcaStop  = next.afrozenStart;
        break;
      default:
        error("printBCMapping: unexpected TCRegion");
    }

    always_assert(tcaStart <= tcaStop);
    if (tcaStart >= ip && tcaStart < tcaStop) break;
  }

  if (currBC < mappingSize && tcaStart == ip) {
    if (auto currUnit = g_repo->getUnit(curr.md5)) {
      auto bcPast = curr.bcStart + instrLen(currUnit->at(curr.bcStart));

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

void OfflineCode::printEventStats(TCA address,
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
