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
#include "hphp/tools/tc-print/offline-trans-data.h"

#include "hphp/tools/tc-print/tc-print.h"
#include "hphp/tools/tc-print/offline-x86-code.h"
#include "hphp/util/repo-schema.h"
#include "hphp/runtime/vm/repo.h"

using std::string;

#define BUFLEN 1000

#define READ(FMT, ELEM)                                                 \
  do {                                                                  \
    if (!gzgets(file, buf, BUFLEN) || (sscanf(buf, FMT, ELEM) != 1)) {  \
      error("Error reading " + string(FMT));                            \
    }                                                                   \
  } while(0)

#define READ_EMPTY() gzgets(file, buf, BUFLEN);

static string tcDataFileName("/tc_data.txt.gz");

namespace HPHP { namespace jit {

void OfflineTransData::loadTCData(string dumpDir) {
  string fileName = dumpDir + tcDataFileName;
  char buf[BUFLEN+1];
  char funcName[BUFLEN+1];

  gzFile file = gzopen(fileName.c_str(), "r");
  if (!file) {
    error("Error opening file " + fileName);
  }

  // read header info
  READ("repo_schema = %s", repoSchema);
  READ("ahot.base = %p", &ahotBase);
  READ("ahot.frontier = %p", &ahotFrontier);
  READ("a.base = %p", &aBase);
  READ("a.frontier = %p", &aFrontier);
  READ("aprof.base = %p", &aprofBase);
  READ("aprof.frontier = %p", &aprofFrontier);
  READ("acold.base = %p", &coldBase);
  READ("acold.frontier = %p", &coldFrontier);
  READ("afrozen.base = %p", &frozenBase);
  READ("afrozen.frontier = %p", &frozenFrontier);
  READ_EMPTY();
  READ("total_translations = %u", &nTranslations);
  READ_EMPTY();

  // Read translations
  for (uint32_t tid = 0; tid < nTranslations; tid++) {
    TransRec  tRec;
    MD5Str    md5Str;
    uint32_t  kind;
    FuncId    funcId;
    int32_t   resumed;
    uint64_t  profCount;
    size_t    numBCMappings = 0;
    size_t    numBlocks = 0;
    size_t    numGuards = 0;

    READ("Translation %u {", &tRec.id);
    READ(" src.md5 = %s", md5Str);
    tRec.md5 = MD5(md5Str);
    READ(" src.funcId = %u", &funcId);
    READ(" src.funcName = %s", funcName);
    tRec.funcName = funcName;
    READ(" src.resumed = %d", &resumed);
    READ(" src.bcStart = %d", &tRec.bcStart);

    READ(" src.blocks = %lu", &numBlocks);
    for (size_t i = 0; i < numBlocks; ++i) {
      MD5Str md5Tmp;
      Offset start = kInvalidOffset;
      Offset past = kInvalidOffset;

      if (gzgets(file, buf, BUFLEN) == Z_NULL ||
          sscanf(buf, "%s %d %d", md5Tmp, &start, &past) != 3) {
        snprintf(buf, BUFLEN,
                 "Error reading bytecode block #%lu at translation %u\n",
                 i, tRec.id);
        error(buf);
      }

      tRec.blocks.emplace_back(TransRec::Block{MD5(md5Tmp), start, past});
    }

    READ(" src.guards = %lu", &numGuards);
    for (size_t i = 0; i < numGuards; ++i) {
      char location[BUFLEN+1];
      char type[BUFLEN+1];
      if (gzgets(file, buf, BUFLEN) == Z_NULL ||
          sscanf(buf, "%s :: %s", location, type) != 2) {
        snprintf(buf, BUFLEN,
                 "Error reading guard #%lu at translation %u\n",
                 i, tRec.id);
        error(buf);
      }

      tRec.guards.emplace_back(folly::to<std::string>(
                                 location, " :: ", type));
    }

    READ(" kind = %u %*s", &kind);
    int isLLVM;
    READ(" isLLVM = %d", &isLLVM);
    tRec.isLLVM = isLLVM;
    READ(" aStart = %p", (void**)&tRec.aStart);
    READ(" aLen = %x", &tRec.aLen);
    READ(" coldStart = %p", (void**)&tRec.acoldStart);
    READ(" coldLen = %x", &tRec.acoldLen);
    READ(" frozenStart = %p", (void**)&tRec.afrozenStart);
    READ(" frozenLen = %x", &tRec.afrozenLen);
    READ(" profCount = %" PRIu64 "", &profCount);

    READ(" bcMapping = %lu", &numBCMappings);
    for (size_t i = 0; i < numBCMappings; i++) {
      TransBCMapping bcMap;
      MD5Str md5Tmp;

      if (gzgets(file, buf, BUFLEN) == Z_NULL ||
          sscanf(buf, "%s %d %p %p %p",
                 md5Tmp, &bcMap.bcStart,
                 (void**)&bcMap.aStart,
                 (void**)&bcMap.acoldStart,
                 (void**)&bcMap.afrozenStart) != 5) {

        snprintf(buf, BUFLEN,
                 "Error reading bytecode mapping #%lu at translation %u\n",
                 i, tRec.id);

        error(buf);
      }

      bcMap.md5 = MD5(md5Tmp);
      tRec.bcMapping.push_back(bcMap);
    }

    // push a sentinel bcMapping so that we can figure out stop offsets later on
    const TransBCMapping sentinel { tRec.md5, 0,
                                    tRec.aStart + tRec.aLen,
                                    tRec.acoldStart + tRec.acoldLen,
                                    tRec.afrozenStart + tRec.afrozenLen };
    tRec.bcMapping.push_back(sentinel);

    READ_EMPTY();
    READ_EMPTY();
    tRec.src = SrcKey { funcId, tRec.bcStart, static_cast<bool>(resumed) };
    tRec.kind = (TransKind)kind;
    always_assert(tid == tRec.id);
    addTrans(tRec, profCount);

    funcIds.insert(tRec.src.getFuncId());

    if (tRec.aStart) {
      transAddrRanges.push_back(
        TransAddrRange(tRec.aStart,
                       tRec.aStart + tRec.aLen - 1,
                       tid));
      addr2TransMap[tRec.aStart] = tid;
    }
    if (tRec.acoldStart) {
      transAddrRanges.push_back(
        TransAddrRange(tRec.acoldStart,
                       tRec.acoldStart + tRec.acoldLen - 1,
                       tid));
      // If there's no code in 'a', then the entry must be in 'aCold'
      if (!tRec.aStart) {
        addr2TransMap[tRec.acoldStart] = tid;
      }
    }
    if (tRec.afrozenStart) {
      transAddrRanges.push_back(
        TransAddrRange(tRec.afrozenStart,
                       tRec.afrozenStart + tRec.afrozenLen - 1,
                       tid));
    }
  }
  always_assert(nTranslations == translations.size());

  sort(transAddrRanges.begin(), transAddrRanges.end());

  gzclose(file);
}


// Returns the id of the translation containing the given address,
// or INVALID_ID if none.
TransID OfflineTransData::getTransContaining(TCA addr) const {
  int32_t first = 0;
  int32_t last  = transAddrRanges.size() - 1;
  while (first <= last) {
    int32_t mid = (first + last) / 2;
    if (transAddrRanges[mid].start > addr) {
      last = mid - 1;
    } else if (transAddrRanges[mid].end < addr) {
      first = mid + 1;
    } else {
      return transAddrRanges[mid].transId;
    }
  }
  return INVALID_ID;
}


// Find translations that belong to the selectedFuncId
// Also returns the max prof count among them
uint64_t OfflineTransData::findFuncTrans(uint32_t selectedFuncId,
                                         vector<TransID> *inodes) {
  uint64_t maxProfCount = 1; // Init w/ 1 to avoid div by 0 when all counts are 0

  for (uint32_t tid = 0; tid < nTranslations; tid++) {
    if (translations[tid].kind != TransKind::Anchor &&
        translations[tid].src.getFuncId() == selectedFuncId) {

      inodes->push_back(tid);

      uint64_t profCount = transCounters[tid];
      if (profCount > maxProfCount) {
        maxProfCount = profCount;
      }
    }
  }

  return maxProfCount;
}


void OfflineTransData::addControlArcs(uint32_t selectedFuncId,
                                      OfflineX86Code *transCode) {
  vector<TransID> funcTrans;
  findFuncTrans(selectedFuncId, &funcTrans);

  for (uint32_t i = 0; i < funcTrans.size(); i++) {
    TransID transId = funcTrans[i];

    std::vector<TCA> jmpTargets;
    transCode->getTransJmpTargets(getTransRec(transId), &jmpTargets);

    auto const srcFuncId = getTransRec(transId)->src.getFuncId();

    for (size_t i = 0; i < jmpTargets.size(); i++) {
      TransID targetId = getTransStartingAt(jmpTargets[i]);
      if (targetId != INVALID_ID &&
          // filter jumps to prologues of other funcs for now
          getTransRec(targetId)->src.getFuncId() == srcFuncId &&
          getTransRec(targetId)->kind != TransKind::Anchor) {

        addControlArc(transId, targetId);
      }
    }
  }
}

void OfflineTransData::printTransRec(TransID transId,
                                     const PerfEventsMap<TransID>& transStats) {

  const TransRec* tRec = getTransRec(transId);

  std::cout << folly::format(
    "Translation {} {{\n"
    "  src.md5 = {}\n"
    "  src.funcId = {}\n"
    "  src.funcName = {}\n"
    "  src.resumed = {}\n"
    "  src.bcStartOffset = {}\n"
    "  src.guards = {}\n",
    tRec->id,
    tRec->md5,
    tRec->src.getFuncId(),
    tRec->funcName,
    tRec->src.resumed(),
    tRec->src.offset(),
    tRec->guards.size());

  for (auto& guard : tRec->guards) {
    std::cout << "    " << guard << '\n';
  }

  std::cout << folly::format(
    "  kind = {}\n"
    "  isLLVM = {:d}\n"
    "  aStart = {}\n"
    "  aLen = {:#x}\n"
    "  coldStart = {}\n"
    "  coldLen = {:#x}\n"
    "  frozenStart = {}\n"
    "  frozenLen = {:#x}\n",
    show(tRec->kind),
    tRec->isLLVM,
    tRec->aStart,
    tRec->aLen,
    tRec->acoldStart,
    tRec->acoldLen,
    tRec->afrozenStart,
    tRec->afrozenLen);

  if (transCounters[transId]) {
    std::cout << folly::format(
      "  prof-counters = {}\n",
      transCounters[transId]);
  }

  transStats.printEventsHeader(transId);

  std::cout << "}\n\n";
}

} } // HPHP::jit
