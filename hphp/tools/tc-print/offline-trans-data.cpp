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
#include "hphp/tools/tc-print/offline-trans-data.h"

#include "hphp/tools/tc-print/tc-print.h"
#include "hphp/tools/tc-print/offline-code.h"
#include "hphp/util/build-info.h"
#include "hphp/runtime/vm/repo.h"
#include <folly/Math.h>

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
    SHA1Str   sha1Str;
    uint32_t  kind;
    uint64_t  srcKeyInt;
    uint64_t  annotationsCount;
    size_t    numBCMappings = 0;
    size_t    numBlocks = 0;
    size_t    numGuards = 0;

    READ("Translation %u {", &tRec.id);
    if (tRec.id == kInvalidTransID) {
      addTrans(tRec);
      READ_EMPTY();
      READ_EMPTY();
      continue;
    }
    READ(" src.sha1 = %s", sha1Str);
    tRec.sha1 = SHA1(sha1Str);
    READ(" src.funcName = %s", funcName);
    tRec.funcName = funcName;
    READ(" src.key = %" PRIu64 "", &srcKeyInt);
    tRec.src = SrcKey::fromAtomicInt(srcKeyInt);

    READ(" src.blocks = %lu", &numBlocks);
    for (size_t i = 0; i < numBlocks; ++i) {
      SHA1Str sha1Tmp;
      Offset start = kInvalidOffset;
      Offset past = kInvalidOffset;

      if (gzgets(file, buf, BUFLEN) == Z_NULL ||
          sscanf(buf, "%s %d %d", sha1Tmp, &start, &past) != 3) {
        snprintf(buf, BUFLEN,
                 "Error reading bytecode block #%lu at translation %u\n",
                 i, tRec.id);
        error(buf);
      }

      tRec.blocks.emplace_back(TransRec::Block{SHA1(sha1Tmp), start, past});
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

    READ(" bcMapping = %lu", &numBCMappings);
    for (size_t i = 0; i < numBCMappings; i++) {
      TransBCMapping bcMap;
      SHA1Str sha1Tmp;
      uint64_t srcKeyIntTmp;

      if (gzgets(file, buf, BUFLEN) == Z_NULL ||
          sscanf(buf, "%s %" PRIu64 " %p %p %p",
                 sha1Tmp, &srcKeyIntTmp,
                 (void**)&bcMap.aStart,
                 (void**)&bcMap.acoldStart,
                 (void**)&bcMap.afrozenStart) != 5) {

        snprintf(buf, BUFLEN,
                 "Error reading bytecode mapping #%lu at translation %u\n",
                 i, tRec.id);

        error(buf);
      }

      bcMap.sha1 = SHA1(sha1Tmp);
      bcMap.sk = SrcKey::fromAtomicInt(srcKeyIntTmp);
      tRec.bcMapping.push_back(bcMap);
    }

    // push a sentinel bcMapping so that we can figure out stop offsets later on
    const TransBCMapping sentinel { tRec.sha1, SrcKey {},
                                    tRec.aStart + tRec.aLen,
                                    tRec.acoldStart + tRec.acoldLen,
                                    tRec.afrozenStart + tRec.afrozenLen };
    tRec.bcMapping.push_back(sentinel);

    READ_EMPTY();
    READ_EMPTY();
    tRec.kind = (TransKind)kind;
    always_assert_flog(tid == tRec.id,
                       "Translation {} has id {}", tid, tRec.id);
    addTrans(tRec);

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
    const auto mid = folly::midpoint(first, last);
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
void OfflineTransData::findFuncTrans(uint32_t selectedFuncId,
                                     vector<TransID> *inodes) {
  for (uint32_t tid = 0; tid < nTranslations; tid++) {
    if (!translations[tid].isValid() ||
        translations[tid].kind == TransKind::Anchor ||
        translations[tid].src.funcID().toInt() != selectedFuncId) continue;
    inodes->push_back(tid);
  }
}


void OfflineTransData::addControlArcs(uint32_t selectedFuncId,
                                      OfflineCode *transCode) {
  vector<TransID> funcTrans;
  findFuncTrans(selectedFuncId, &funcTrans);

  for (uint32_t i = 0; i < funcTrans.size(); i++) {
    TransID transId = funcTrans[i];

    std::vector<TCA> jmpTargets;
    transCode->getTransJmpTargets(getTransRec(transId), &jmpTargets);

    auto const srcFuncId = getTransRec(transId)->src.funcID();

    for (size_t i2 = 0; i2 < jmpTargets.size(); i2++) {
      TransID targetId = getTransStartingAt(jmpTargets[i2]);
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
  if (!tRec->isValid()) {
    std::cout << "Translation -1 {\n}\n";
    return;
  }

  std::cout << folly::format(
    "Translation {} {{\n"
    "  src.sha1 = {}\n"
    "  src.funcId = {}\n"
    "  src.funcName = {}\n"
    "  src.resumeMode = {}\n"
    "  src.prologue = {}\n"
    "  src.bcStartOffset = {}\n"
    "  src.guards = {}\n",
    tRec->id,
    tRec->sha1,
    tRec->src.funcID(),
    tRec->funcName,
    static_cast<int32_t>(tRec->src.resumeMode()),
    tRec->src.prologue(),
    tRec->src.printableOffset(),
    tRec->guards.size());

  for (auto& guard : tRec->guards) {
    std::cout << "    " << guard << '\n';
  }

  std::cout << folly::format(
    "  kind = {}\n"
    "  hasLoop = {:d}\n"
    "  aStart = {}\n"
    "  aLen = {:#x}\n"
    "  coldStart = {}\n"
    "  coldLen = {:#x}\n"
    "  frozenStart = {}\n"
    "  frozenLen = {:#x}\n",
    show(tRec->kind),
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
      auto const& annotationValue = annotation.second;
      // Either read annotation from file or print inline.
      if (annotationsVerbosity > 1 && annotationValue.substr(0, 5) == "file:") {
        std::cout << '\n';
        auto const maybeFileInfo = g_annotations->getFileInfo(annotationValue);

        if (!maybeFileInfo) {
          std::cout << annotationValue << '\n';
          continue;
        }
        auto const& fileInfo = *maybeFileInfo;

        auto const& maybeValue = g_annotations->getValue(fileInfo);
        if (maybeValue) {
          std::cout << '\n' << (folly::gen::split(*maybeValue, '\n') |
                                folly::gen::unsplit("\n    "));
        }
        std::cout << '\n';
      } else {
        std::cout << folly::format(" = {}\n", annotationValue);
      }
    }
  }

  transStats.printEventsHeader(transId);

  std::cout << "}\n\n";
}

} } // HPHP::jit
