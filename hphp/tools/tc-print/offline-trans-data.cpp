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
    uint64_t  annotationsCount;
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
    int hasLoop;
    READ(" hasLoop = %d", &hasLoop);
    tRec.hasLoop = hasLoop;
    READ(" aStart = %p", (void**)&tRec.aStart);
    READ(" aLen = %x", &tRec.aLen);
    READ(" coldStart = %p", (void**)&tRec.acoldStart);
    READ(" coldLen = %x", &tRec.acoldLen);
    READ(" frozenStart = %p", (void**)&tRec.afrozenStart);
    READ(" frozenLen = %x", &tRec.afrozenLen);
    READ(" annotations = %" PRIu64 "", &annotationsCount);
    for (size_t i = 0; i < annotationsCount; ++i) {
      if (gzgets(file, buf, BUFLEN) == Z_NULL) {
        snprintf(buf, BUFLEN,
                 "Error reading annotation #%lu at translation %u\n",
                 i, tRec.id);
        error(buf);
      }
      char* title = strstr(buf, "[\"");
      char* annotation = nullptr;
      if (title) {
        title += 2;
        annotation = strstr(buf, "\"] = ");
        if (annotation) {
          *annotation = '\0';
          annotation += 5;
          if (auto eol = strchr(annotation, '\n')) {
            *eol = '\0';
          }
        }
      }
      if (!title || !annotation) {
        title = "<unknown>";
        annotation = buf;
      }
      tRec.annotations.emplace_back(title, annotation);
    }

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

    funcIds.insert(tRec.src.funcID());

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
        translations[tid].src.funcID() == selectedFuncId) {

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

    auto const srcFuncId = getTransRec(transId)->src.funcID();

    for (size_t i = 0; i < jmpTargets.size(); i++) {
      TransID targetId = getTransStartingAt(jmpTargets[i]);
      if (targetId != INVALID_ID &&
          // filter jumps to prologues of other funcs for now
          getTransRec(targetId)->src.funcID() == srcFuncId &&
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
    tRec->src.funcID(),
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
    "  hasLoop = {:d}\n"
    "  aStart = {}\n"
    "  aLen = {:#x}\n"
    "  coldStart = {}\n"
    "  coldLen = {:#x}\n"
    "  frozenStart = {}\n"
    "  frozenLen = {:#x}\n",
    show(tRec->kind),
    tRec->isLLVM,
    tRec->hasLoop,
    tRec->aStart,
    tRec->aLen,
    tRec->acoldStart,
    tRec->acoldLen,
    tRec->afrozenStart,
    tRec->afrozenLen);

  if (annotationsVerbosity > 0) {
    for (auto& annotation : tRec->annotations) {
      std::cout << folly::format(
        "  annotation[\"{}\"]",
        annotation.first);
      // Either read annotation from file or print inline.
      if (annotationsVerbosity > 1 &&
          annotation.second.substr(0, 5) == "file:") {
        std::cout << '\n';
        string fileName = annotation.second.substr(5);
        // The actual file should be located in dumpDir.
        size_t pos = fileName.find_last_of('/');
        if (pos != std::string::npos) {
          fileName = fileName.substr(pos+1);
        }
        uint64_t offset = 0;
        uint64_t length = std::numeric_limits<decltype(length)>::max();
        pos = fileName.find_first_of(':');
        if (pos != std::string::npos) {
          auto filePos = fileName.substr(pos+1);
          fileName.resize(pos);
          if (sscanf(filePos.c_str(), "%ld:%ld", &offset, &length) != 2) {
            std::cout << annotation.second << '\n';
            continue;
          }
        }
        fileName = folly::sformat("{}/{}", dumpDir, fileName);
        FILE *file = fopen(fileName.c_str(), "r");
        if (!file) {
          std::cout << folly::format("<Error opening file {}>\n",
                                     fileName);
          continue;
        }
        if (fseeko(file, offset, SEEK_SET) != 0) {
          std::cout << folly::format("<Error positioning file {} at {}>\n",
                                     fileName, offset);
          fclose(file);
          continue;
        }
        // zlib can read uncompressed files too.
        gzFile compressedFile = gzdopen(fileno(file), "r");
        if (!compressedFile) {
          std::cout << folly::format("<Error opening file {} with gzdopen>\n",
                                     fileName);
          fclose(file);
          continue;
        }
        SCOPE_EXIT{ gzclose(compressedFile); };
        std::cout << '\n';
        char buf[BUFLEN];
        uint64_t bytesRead = 0;
        while (gzgets(compressedFile, buf, BUFLEN) != Z_NULL &&
               bytesRead < length) {
          std::cout << folly::format("    {}", buf);
          bytesRead += std::strlen(buf) + 1;
        }
        std::cout << '\n';
      } else {
        std::cout << folly::format(" = {}\n", annotation.second);
      }
    }
  }

  if (transCounters[transId]) {
    std::cout << folly::format(
      "  prof-counters = {}\n",
      transCounters[transId]);
  }

  transStats.printEventsHeader(transId);

  std::cout << "}\n\n";
}

} } // HPHP::jit
