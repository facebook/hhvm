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


#define MAX_SYM_LEN       10240

using std::string;

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

void OfflineCode::printDisasm(std::ostream& os,
                              TCA startAddr,
                              uint32_t len,
                              const vector<TransBCMapping>& bcMap,
                              const PerfEventsMap<TCA>& perfEvents,
                              bool hostOpcodes) {
  TCRegion tcr = findTCRegionContaining(startAddr);
  disasm(os, tcRegions[tcr].file, tcRegions[tcr].baseAddr, startAddr, len,
         perfEvents, BCMappingInfo(tcr, bcMap), true, hostOpcodes);
}

void OfflineCode::setAnnotationRanges(BCMappingInfo& bc, printir::Unit unit) {
  vector<printir::TCRange> annotations;

  for (auto const& block : unit.blocks) {
    for (auto const& instr: block.second.instrs) {
      for (auto const& tcr: instr.tcRanges) {
        if (tcr.start != nullptr &&
            tcr.end != nullptr &&
            tcr.start != tcr.end) {
          annotations.push_back(tcr);
        }
      }
    }
  }

  std::sort(annotations.begin(),
            annotations.end(),
            [](const printir::TCRange& a, const printir::TCRange& b) {
              return a.start < b.start;
            });

  bc.annotations = annotations;
}

folly::dynamic OfflineCode::getDisasm(TCA startAddr,
                                      uint32_t len,
                                      const vector<TransBCMapping>& bcMap,
                                      const PerfEventsMap<TCA>& perfEvents,
                                      bool hostOpcodes,
                                      Optional<printir::Unit> unit) {
  auto const tcr = findTCRegionContaining(startAddr);
  auto mappingInfo = BCMappingInfo(tcr, bcMap);

  if (unit) setAnnotationRanges(mappingInfo, *unit);

  auto const regionInfo = getRegionInfo(tcRegions[tcr].file,
                                        tcRegions[tcr].baseAddr,
                                        startAddr,
                                        len,
                                        perfEvents,
                                        mappingInfo);
  return regionInfo.toDynamic();
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

TCA OfflineCode::getRegionStart(TCRegion region, TransBCMapping transBCMap) {
  switch (region) {
    case TCRHot:
    case TCRMain:
    case TCRProfile:
      return transBCMap.aStart;
    case TCRCold:
      return transBCMap.acoldStart;
    case TCRFrozen:
      return transBCMap.afrozenStart;
    case TCRCount:
      error("printBCMapping: unexpected TCRegion");
  }
  always_assert(false);
}

void OfflineCode::printEventStats(std::ostream& os,
                                  EventCounts events) {
  if (events.empty()) {
    os << string(48, ' ');
    return;
  }
  for (int i = 0; i < events.size(); i++) {
    auto const event = static_cast<PerfEventType>(i);
    auto const count = events[i];
    auto const eventStr = count ?
                          folly::sformat("{:>3}:{:>4}",
                                         eventTypeToSmallCaption(event),
                                         count) :
                          "";
    os << folly::format("{:<10} ", eventStr);
  }
}

EventCounts OfflineCode::getEventCounts(TCA address,
                                        uint32_t instrLen,
                                        const PerfEventsMap<TCA>& perfEvents) {
  if (perfEvents.empty()) return EventCounts();

  auto const numEvents = getNumEventTypes();
  EventCounts eventCounts(numEvents);
  for (int i = 0; i < numEvents; i++) {
    auto const event = static_cast<PerfEventType>(i);
    auto const eventCount = perfEvents.getEventCount(address,
                                                     address + instrLen - 1,
                                                     event);
    eventCounts[i] = eventCount;
  }
  return eventCounts;
}

void OfflineCode::disasm(std::ostream& os,
                         FILE* file,
                         TCA fileStartAddr,
                         TCA codeStartAddr,
                         uint64_t codeLen,
                         const PerfEventsMap<TCA>& perfEvents,
                         OfflineCode::BCMappingInfo bcMappingInfo,
                         bool printAddr,
                         bool printBinary) {

  auto const regionInfo = getRegionInfo(file,
                                        fileStartAddr,
                                        codeStartAddr,
                                        codeLen,
                                        perfEvents,
                                        bcMappingInfo);

  for (auto const& rangeInfo : regionInfo.ranges) {
    printRangeInfo(os, rangeInfo, printAddr, printBinary);
  }
}

void OfflineCode::printRangeInfo(std::ostream& os,
                                 const TCRangeInfo& rangeInfo,
                                 const bool printAddr,
                                 const bool printBinary) {
  if (rangeInfo.disasm.empty()) return;
  if (rangeInfo.sk && rangeInfo.disasm[0].ip == rangeInfo.start) {
    auto const sk = *rangeInfo.sk;
    if (rangeInfo.instrStr) {
      os << std::setw(4) << sk.printableOffset() << ": "
         << *rangeInfo.instrStr << std::endl;
    } else {
      auto const currSha1 = rangeInfo.sha1
        ? rangeInfo.sha1->toString() : "\"missing SHA1\"";
      os << folly::format(
            "<<< couldn't find unit {} to print bytecode at offset {} >>>\n",
            currSha1, sk.printableOffset());
    }
  }
  for (auto const& disasmInfo : rangeInfo.disasm) {
    printDisasmInfo(os, disasmInfo, printAddr, printBinary);
  }
}

void OfflineCode::printDisasmInfo(std::ostream& os,
                                  const TCDisasmInfo& disasmInfo,
                                  const bool printAddr,
                                  const bool printBinary) {
    if (printAddr) {
      os << folly::format("{:>#14x}: ",
                          reinterpret_cast<uintptr_t>(disasmInfo.ip));
    }
    if (printBinary) os << disasmInfo.binaryStr;
    printEventStats(os, disasmInfo.eventCounts);
    os << folly::format("{}{}\n", disasmInfo.codeStr, disasmInfo.callDest);
}

vector<TCRangeInfo> annotateRanges(const vector<TCRangeInfo>& ranges,
                                   const vector<printir::TCRange>& annotations){
  if (ranges.empty() || annotations.empty()) return ranges;

  vector<TCRangeInfo> annotatedRanges;

  auto currRangeItr = ranges.begin();
  TCRangeInfo lastRange = *currRangeItr;
  auto const progressRangeItr = [&]() {
    if (currRangeItr != ranges.end()) ++currRangeItr;
    if (currRangeItr != ranges.end()) lastRange = *currRangeItr;
  };

  auto currAnnotItr = annotations.begin();
  auto const progressAnnotItr = [&](const TCA tcStart) {
    while (tcStart >= currAnnotItr->end) {
      if (currAnnotItr == annotations.end()) return;
      ++currAnnotItr;
    }
  };

  while (currRangeItr != ranges.end()) {
    progressAnnotItr(lastRange.start);
    if (currAnnotItr == annotations.end()) break;

    auto const tcStart = lastRange.start;
    auto const tcEnd = lastRange.end;
    auto const annotStart = currAnnotItr->start;
    auto const annotEnd = currAnnotItr->end;

    if (tcStart < annotStart) {
      if (tcEnd <= annotStart) {
        // this range both starts and ends before our next annotation, so this
        // range gets added annotationless
        annotatedRanges.push_back(lastRange);
        progressRangeItr();
      } else {
        // the first part of this range happened before our next annotation, so
        // split that part off and add it annotationless, then reprocess the
        // second part
        auto const splitTCRange = lastRange.split(annotStart);
        annotatedRanges.push_back(splitTCRange.first);
        lastRange = splitTCRange.second;
      }
    } else {
      if (tcEnd <= annotEnd) {
        // this range ends before our next annotation, so this
        // range gets added with the current annotation
        lastRange.annotation = *currAnnotItr;
        annotatedRanges.push_back(lastRange);
        progressRangeItr();
      } else {
        // the range is split among multiple annotations, so split this first
        // part off and annotate it, then process the rest
        auto splitTCRange = lastRange.split(annotEnd);
        splitTCRange.first.annotation = *currAnnotItr;
        annotatedRanges.push_back(splitTCRange.first);
        lastRange = splitTCRange.second;
      }
    }
  }

  // Whatever ranges might be left after we're done with our annotations, make
  // sure that we add those as well
  while (currRangeItr != ranges.end()) {
    annotatedRanges.push_back(lastRange);
    progressRangeItr();
  }

  return annotatedRanges;
}

vector<TCRangeInfo>
OfflineCode::getRanges(const BCMappingInfo& bcMappingInfo,
                       const TCA start,
                       const TCA end) {
  auto const& bcMap = bcMappingInfo.bcMapping;
  auto const region = bcMappingInfo.tcRegion;
  auto const numRanges = bcMap.size();
  vector<TCRangeInfo> ranges;

  // For "prologue" translations, we need another range in front of where the
  // bcMapping starts.
  auto const actualStart = getRegionStart(region, bcMap[0]);
  if (start != actualStart) {
    ranges.push_back(TCRangeInfo{start, actualStart});
  }

  for (int i = 0; i < numRanges; i++) {
    auto const& curr = bcMap[i];

    auto const tcaStart = getRegionStart(bcMappingInfo.tcRegion, curr);
    auto const tcaEnd = (i < numRanges - 1) ?
                        getRegionStart(bcMappingInfo.tcRegion, bcMap[i + 1]) :
                        end; // use the provided end for the last element
    if (tcaStart != tcaEnd) {
      ranges.push_back(getRangeInfo(curr, tcaStart, tcaEnd));
    }
  }

  if (ranges.empty() || bcMappingInfo.annotations.empty()) {
    return ranges;
  }

  return annotateRanges(ranges, bcMappingInfo.annotations);
}

TCRangeInfo OfflineCode::getRangeInfo(const TransBCMapping& transBCMap,
                                      const TCA start,
                                      const TCA end) {
  TCRangeInfo rangeInfo{start, end, transBCMap.sk, transBCMap.sha1};
  auto const sk = transBCMap.sk;
  if (sk.valid()) {
    rangeInfo.unit = sk.func()->unit();
    rangeInfo.func = sk.func();
    rangeInfo.instrStr = sk.showInst();
    auto const lineNum = sk.lineNumber();
    if (lineNum != -1) rangeInfo.lineNum = lineNum;
  }

  return rangeInfo;
}

TCDisasmInfo OfflineCode::getDisasmInfo(const TCA ip,
                                         const uint32_t instrLen,
                                         const PerfEventsMap<TCA>& perfEvents,
                                         const std::string& binaryStr,
                                         const std::string& callDest,
                                         const std::string& codeStr) {
  auto const eventCounts = getEventCounts(ip, instrLen, perfEvents);
  return TCDisasmInfo{binaryStr,
                       callDest,
                       codeStr,
                       eventCounts,
                       ip,
                       instrLen};
}

void OfflineCode::readDisasmFile(FILE* file,
                                 const Offset offset,
                                 const uint64_t codeLen,
                                 void* code) {
  if (fseek(file, offset, SEEK_SET)) {
    error("disasm error: seeking file");
  }

  size_t readLen = fread(code, codeLen, 1, file);
  if (readLen != 1) {
    error("Failed to read {} bytes at offset {} from code file due to {}",
          codeLen, offset, feof(file) ? "EOF" : "read error");
  }
}

} } // HPHP::jit
