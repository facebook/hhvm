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

#include "hphp/tools/tc-print/tc-print.h"

#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <folly/Format.h>
#include <folly/dynamic.h>
#include <folly/DynamicConverter.h>
#include <folly/json.h>
#include <folly/Singleton.h>

#include "hphp/util/build-info.h"

#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/program-functions.h"

#include "hphp/tools/tc-print/perf-events.h"
#include "hphp/tools/tc-print/printir-annotation.h"
#include "hphp/tools/tc-print/offline-trans-data.h"
#include "hphp/tools/tc-print/offline-code.h"
#include "hphp/tools/tc-print/mappers.h"
#include "hphp/tools/tc-print/repo-wrapper.h"
#include "hphp/tools/tc-print/std-logger.h"
#include "hphp/tools/tc-print/tc-print-logger.h"
#ifdef FACEBOOK
#include "hphp/facebook/extensions/scribe/ext_scribe.h"
#include "hphp/tools/tc-print/facebook/db-logger.h"
#endif

using namespace HPHP;
using namespace HPHP::jit;

#define MAX_SYM_LEN       10240

std::string     dumpDir("/tmp");
std::string     configFile;
std::string     profFileName;
uint32_t        nTopTrans       = 0;
uint32_t        nTopFuncs       = 0;
bool            useJSON         = false;
bool            creationOrder   = false;
bool            transCFG        = false;
bool            collectBCStats  = false;
bool            inclusiveStats  = false;
bool            verboseStats    = false;
bool            hostOpcodes     = false;
folly::Optional<SHA1> sha1Filter;
PerfEventType   sortBy          = EVENT_CYCLES;
bool            sortByDensity   = false;
bool            sortBySize      = false;
double          helpersMinPercentage = 0;
ExtOpcode       filterByOpcode  = 0;
std::string     kindFilter      = "all";
uint32_t        selectedFuncId  = INVALID_ID;
TCA             minAddr         = 0;
TCA             maxAddr         = (TCA)-1;
uint32_t        annotationsVerbosity = 2;
#ifdef FACEBOOK
bool            printToDB       = false;
std::string     hiveTable;
#endif

std::vector<uint32_t> transPrintOrder;

RepoWrapper*      g_repo;
OfflineTransData* g_transData;
OfflineCode*   transCode;

std::unique_ptr<AnnotationCache> g_annotations;

const char* kListKeyword = "list";
TCPrintLogger* g_logger;

PerfEventsMap<TCA>     tcaPerfEvents;
PerfEventsMap<TransID> transPerfEvents;

#define NTRANS        (g_transData->getNumTrans())
#define NFUNCS        (g_transData->getNumFuncs())
#define TREC(TID)     (g_transData->getTransRec(TID))

void warnTooFew(const std::string& name,
                uint32_t requested,
                uint32_t available) {
  fprintf(stderr,
          "Requested top %u %s, but there are only %u available.\n",
          requested,
          name.c_str(),
          available);
}

template<typename T>
std::string toString(T value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

void usage() {
  printf("Usage: tc-print [OPTIONS]\n"
    "  Options:\n"
    "    -c <FILE>       : uses the given config file\n"
    "    -D              : used along with -t, this option sorts the top "
    "translations by density (count / size) of the selected perf event\n"
    "    -d <DIRECTORY>  : looks for dump file in <DIRECTORY> "
    "(default: /tmp)\n"
    "    -f <FUNC_ID>    : prints the translations for the given "
    "<FUNC_ID>, sorted by start offset\n"
    "    -g <FUNC_ID>    : prints the CFG among the translations for the "
    "given <FUNC_ID>\n"
    "    -p <FILE>       : uses raw profile data from <FILE>\n"
    "    -s              : prints all translations sorted by creation "
    "order\n"
    "    -u <SHA1>        : prints all translations from the specified "
    "unit\n"
    "    -t <NUMBER>     : prints top <NUMBER> translations according to "
    "profiling info\n"
    "    -k <TRANS_KIND> : used with -t, filters only translations of the "
    "given kind, e.g. TransLive (default: all)\n"
    "    -a <ADDR>       : used with -t, filters only events at addresses "
    ">= <ADDR>\n"
    "    -A <ADDR>       : used with -t, filters only events at addresses "
    "<= <ADDR>\n"
    "    -T <NUMBER>     : prints top <NUMBER> functions according to "
    "profiling info\n"
    "    -e <EVENT>      : sorts by the specified perf event. Should either "
    "be one of the predefined events or one passed with -E.\n"
    "                      Pass '%s' to get a list of valid predefined event "
    "types.\n"
    "    -E <EVENTS>     : specifies a new replacement set of perf events. "
    "Pass a comma-separated list of events to replace the default set.\n"
    "    -b              : prints bytecode stats\n"
    "    -B <OPCODE>     : used in conjunction with -e, prints the top "
    "bytecode translationc event type. Pass '%s' to get a "
    "list of valid opcodes.\n"
    "    -i              : reports inclusive stats by including helpers "
    "(perf data must include call graph information)\n"
    "    -n <level>      : level of verbosity for annotations. Use 0 for "
    "no annotations, 1 - for inline, 2 - to print all annotations "
    "including from a file (default: 2).\n"
    "    -o              : print host opcodes\n"
    "    -v <PERCENTAGE> : sets the minimum percentage to <PERCENTAGE> "
    "when printing the top helpers (implies -i). The lower the percentage,"
    " the more helpers that will show up.\n"
    "    -j              : outputs tc-dump in JSON format (not compatible with "
    "some other flags).\n"
    // TODO(T52857399) - investigate compatibility with other flags
    #ifdef FACEBOOK
    "    -H <HIVE_TABLE> : used with -j, write the JSON output to Hive in the "
    "table <HIVE_TABLE>\n"
    "    -x              : log translations to database\n"
    #endif
    "    -h              : prints help message\n",
    kListKeyword,
    kListKeyword);
}

void printValidBytecodes() {
  g_logger->printGeneric("<OPCODE>:\n");
  auto validOpcodes = getValidOpcodeNames();
  for (size_t i = 0; i < validOpcodes.size(); i++) {
    g_logger->printGeneric("  * %s\n", validOpcodes[i].first.c_str());
  }
}

void printValidEventTypes() {
  g_logger->printGeneric("<EVENT>:\n");
  // Note: The -e list option is only for printing the predefined (default)
  //       event types; thus ranging from [0,NUM_PREDEFINED_EVENT_TYPES).
  for (size_t i = 0; i < NUM_PREDEFINED_EVENT_TYPES; i++) {
    g_logger->printGeneric("  * %s\n",
                           eventTypeToCommandLineArgument((PerfEventType)i));
  }
}

void parseOptions(int argc, char *argv[]) {
  int c;
  opterr = 0;
  char* sortByArg = nullptr;
  while ((c = getopt(argc, argv, "hc:Dd:f:g:ip:st:u:S:T:o:e:E:bB:v:k:a:A:n:jH:"
                                 "x"))
         != -1) {
    switch (c) {
      case 'A':
        if (sscanf(optarg, "%p", &maxAddr) != 1) {
          usage();
          exit(1);
        }
        break;
      case 'a':
        if (sscanf(optarg, "%p", &minAddr) != 1) {
          usage();
          exit(1);
        }
        break;
      case 'h':
        usage();
        exit(0);
      case 'c':
        configFile = optarg;
        break;
      case 'd':
        dumpDir = optarg;
        break;
      case 'f':
        creationOrder = true;
        if (sscanf(optarg, "%u", &selectedFuncId) != 1) {
          usage();
          exit(1);
        }
        break;
      case 'g':
        transCFG = true;
        if (sscanf(optarg, "%u", &selectedFuncId) != 1) {
          usage();
          exit(1);
        }
        break;
      case 'p':
        profFileName = optarg;
        break;
      case 's':
        creationOrder = true;
        break;
      case 't':
        if (sscanf(optarg, "%u", &nTopTrans) != 1) {
          usage();
          exit(1);
        }
        break;
      case 'u':
        if (strlen(optarg) == 32) {
          sha1Filter = SHA1(optarg);
        } else {
          usage();
          exit(1);
        }
        break;
      case 'S':
        sortBySize = true;
      case 'T':
        if (sscanf(optarg, "%u", &nTopFuncs) != 1) {
          usage();
          exit(1);
        }
        break;
      case 'k':
        kindFilter = optarg;
        break;
      case 'D':
        sortByDensity = true;
        break;
      case 'e':
        if (!strcmp(optarg, kListKeyword)) {
          printValidEventTypes();
          exit(0);
        }
        // Will lookup event type after parsing all args
        sortByArg = optarg;
        break;
      case 'E':
        {
          char* p = strtok(optarg, ",");
          while (p) {
            addEventType(std::string(p));
            p = strtok(nullptr, ",");
          }
        }
        break;
      case 'b':
        collectBCStats = true;
        break;
      case 'B':
        if (!strcmp(optarg, kListKeyword)) {
          printValidBytecodes();
          exit(0);
        }
        filterByOpcode = stringToExtOpcode(optarg);
        if (!filterByOpcode) {
          usage();
          exit(1);
        }
        break;
      case 'i':
        inclusiveStats = true;
        break;
      case 'n':
        if (sscanf(optarg, "%u", &annotationsVerbosity) != 1) {
          usage();
          exit(1);
        }
        break;
      case 'v':
        verboseStats = true;
        // -v implies -i
        inclusiveStats = true;
        if (sscanf(optarg, "%lf", &helpersMinPercentage) != 1) {
          usage();
          exit(1);
        }
        break;
      case '?':
        if (optopt == 'd' || optopt == 'c' || optopt == 'p' || optopt == 't') {
          fprintf (stderr, "Error: -%c expects an argument\n\n", optopt);
        }
      case 'o':
        hostOpcodes = true;
        break;
      case 'j':
        useJSON = true;
        break;
      #ifdef FACEBOOK
      case 'x':
        printToDB = true;
        break;
      case 'H':
        hiveTable = optarg;
        break;
      #endif
      default:
        usage();
        exit(1);
    }
  }

  // Lookup the event type of a event to sort by was specified with -e
  if (sortByArg) {
    sortBy = commandLineArgumentToEventType(sortByArg);
    if (sortBy < getFirstEventType() || sortBy >= getNumEventTypes()) {
      usage();
      exit(1);
    }
  } else {
    // If not specified, then default the sorting to the first event type.
    // If using predefined events, then EVENT_CYCLES.
    sortBy = static_cast<PerfEventType>(getFirstEventType());
  }
}

void sortTrans() {
  for (uint32_t tid = 0; tid < NTRANS; tid++) {
    const auto trec = TREC(tid);
    if (trec->isValid() &&
        (selectedFuncId == INVALID_ID ||
         selectedFuncId == trec->src.funcID()) &&
        (kindFilter == "all" || kindFilter == show(trec->kind).c_str())) {
      transPrintOrder.push_back(tid);
    }
  }
}

void loadPerfEvents() {
  FILE* profFile;

  profFile = fopen(profFileName.c_str(), "rt");

  if (!profFile) {
    error("Error opening file " + profFileName);
  }

  char   program[MAX_SYM_LEN];
  char   eventCaption[MAX_SYM_LEN];
  char   line[2*MAX_SYM_LEN];
  TCA    addr;
  vector<uint32_t> tcSamples(getNumEventTypes(), 0);
  vector<uint32_t> hhvmSamples(getNumEventTypes(), 0);
  size_t numEntries = 0;
  PerfEventType eventType = EVENT_NULL;
  // samplesPerKind[event][kind], samplesPerTCRegion[event][TCRegion]
  vector< vector<uint32_t> > samplesPerKind(getNumEventTypes(),
                                            vector<uint32_t>(NumTransKinds, 0));
  vector< vector<uint32_t> > samplesPerTCRegion(getNumEventTypes(),
                                                vector<uint32_t>(TCRCount, 0));

  while (fgets(line, 2*MAX_SYM_LEN, profFile) != nullptr) {
    always_assert(sscanf(line, "%s %s %lu", program, eventCaption, &numEntries)
                  == 3);
    always_assert(numEntries);

    std::vector<std::pair<TCA,std::string>> entries;

    for (size_t i = 0; i < numEntries; i++) {
      fscanf(profFile, "%p %s\n", &addr, line);
      entries.push_back(std::pair<TCA,std::string>(addr, line));
    }

    // Strip :p+ PEBS suffix.
    if (auto pos = strchr(eventCaption, ':')) {
      *pos = '\0';
    }

    if (strncmp(program, "hhvm", 4) == 0) {
      eventType = perfScriptOutputToEventType(eventCaption);
      if (eventType == EVENT_NULL) {
        error(folly::sformat("loadProfData: invalid event caption {}",
                             eventCaption));
      }

      hhvmSamples[eventType]++;

      size_t selIdx = 0;
      addr = entries[0].first;

      if (inclusiveStats) {
        for (size_t i = 0; i < entries.size(); i++) {
          if (g_transData->isAddrInSomeTrans(entries[i].first)) {
            addr = entries[i].first;
            selIdx = i;
            break;
          }
        }
      }

      if (!(minAddr <= addr && addr <= maxAddr)) continue;
      if (!g_transData->isAddrInSomeTrans(addr)) continue;
      TransID transId = g_transData->getTransContaining(addr);
      always_assert(transId != INVALID_ID);
      tcSamples[eventType]++;

      const TransRec* trec = g_transData->getTransRec(transId);
      TransKind kind = trec->kind;
      samplesPerKind[eventType][static_cast<uint32_t>(kind)]++;
      TCRegion region = transCode->findTCRegionContaining(addr);
      always_assert(region != TCRCount);
      samplesPerTCRegion[eventType][region]++;

      std::vector<std::string> stackTrace;
      if (verboseStats) {
        for (size_t i = 0; i < selIdx; i++) {

          if (!strcmp(entries[i].second.c_str(), "[unknown]")) {

            // Append the address to disambiguate.
            entries[i].second += std::string("@")
                              +  toString((void*)entries[i].first);
          }

          stackTrace.push_back(entries[i].second);
        }
        reverse(stackTrace.begin(), stackTrace.end());
      }

      if (selIdx) addr--;
      tcaPerfEvents.addEvent(addr, (PerfEvent){eventType, 1}, stackTrace);
    }
  }

  AddrToTransMapper transMapper(g_transData);
  transPerfEvents = tcaPerfEvents.mapTo(transMapper);

  g_logger->printGeneric("# Number of hhvm samples read "
                         "(%% in TC) from file %s\n",
                         profFileName.c_str());

  for (size_t i = getFirstEventType(); i < getNumEventTypes(); i++) {
    if (!hhvmSamples[i]) continue;

    g_logger->printGeneric("#  %-19s TOTAL: %10u (%u in TC = %5.2lf%%)\n",
                           eventTypeToCommandLineArgument((PerfEventType)i),
                           hhvmSamples[i],
                           tcSamples[i],
                           100.0 * tcSamples[i] / hhvmSamples[i]);

    for (size_t j = 0; j < NumTransKinds; ++j) {
      auto ct = samplesPerKind[i][j];
      if (!ct) continue;
      std::string kind = show(static_cast<TransKind>(j));
      g_logger->printGeneric("# %26s:             %-8u (%5.2lf%%)\n",
                             kind.c_str(), ct, 100.0 * ct / tcSamples[i]);
    }
    g_logger->printGeneric("#\n");
  }
  g_logger->printGeneric("\n");

  // print per-TCRegion information

  // header
  g_logger->printGeneric("# TCRegion ");
  for (size_t i = getFirstEventType(); i < getNumEventTypes(); i++) {
    g_logger->printGeneric("%17s ",
                           eventTypeToCommandLineArgument((PerfEventType)i));
  }
  g_logger->printGeneric("\n");

  // HW events for each region
  for (size_t i = 0; i < TCRCount; i++) {
    g_logger->printGeneric("# %8s ",
                           tcRegionToString(static_cast<TCRegion>(i)).c_str());
    for (size_t j = getFirstEventType(); j < getNumEventTypes(); j++) {
      auto ct = samplesPerTCRegion[j][i];
      g_logger->printGeneric("%8u (%5.2lf%%) ", ct,
                             ct ? (100.0 * ct / tcSamples[j]) : 0);
    }
    g_logger->printGeneric("\n");
  }
  g_logger->printGeneric("#\n\n");

  fclose(profFile);
}

void loadProfData() {
  if (!profFileName.empty()) {
    loadPerfEvents();
  }
}

const char* getDisasmStr(OfflineCode* code,
                         TCA startAddr,
                         uint32_t len,
                         const std::vector<TransBCMapping>& bcMap,
                         const PerfEventsMap<TCA>& perfEvents,
                         bool hostOpcodes) {
  std::ostringstream os;
  code->printDisasm(os, startAddr, len, bcMap, perfEvents, hostOpcodes);
  return os.str().c_str();
}

namespace get_json {

using folly::dynamic;

std::string show(TCA tca) {
  return folly::sformat("{}", static_cast<void*>(tca));
}

dynamic getAnnotation(const Annotations& annotations) {
  // for JSON outputs, we only care about the annotation "after code gen"
  std::string annotationStr;
  for (auto const& annotation : annotations) {
    if (annotation.first == " after code gen ") {
      annotationStr = annotation.second;
      break;
    }
  }
  if (annotationStr.empty()) return dynamic();

  auto const rawValue = g_annotations->getAnnotation(annotationStr);
  if (rawValue.subpiece(0, 5) != "json:") return rawValue;

  try {
    return folly::parseJson(rawValue.subpiece(5));
  }
  catch (const std::runtime_error& re){
    std::cerr << re.what() << std::endl;
    std::cerr << "Parsing annotation as JSON failed. "
              << "Annotation included as raw value\n";
    return rawValue;
  }
}

dynamic getTransRec(const TransRec* tRec,
                    const PerfEventsMap<TransID>& transPerfEvents) {
  auto const guards = dynamic(tRec->guards.begin(), tRec->guards.end());

  auto const resumeMode = [&] {
    switch (tRec->src.resumeMode()) {
      case ResumeMode::None: return "None";
      case ResumeMode::Async: return "Async";
      case ResumeMode::GenIter: return "GenIter";
    }
    always_assert(false);
  }();

  const dynamic src = dynamic::object("sha1", tRec->sha1.toString())
                                     ("funcId", tRec->src.funcID())
                                     ("funcName", tRec->funcName)
                                     ("resumeMode", resumeMode)
                                     ("hasThis", tRec->src.hasThis())
                                     ("prologue", tRec->src.prologue())
                                     ("bcStartOffset", tRec->src.offset())
                                     ("guards", guards);

  const dynamic result = dynamic::object("id", tRec->id)
                                        ("src", src)
                                        ("kind", show(tRec->kind))
                                        ("hasLoop", tRec->hasLoop)
                                        ("aStart", show(tRec->aStart))
                                        ("aLen", tRec->aLen)
                                        ("coldStart", show(tRec->acoldStart))
                                        ("coldLen", tRec->acoldLen)
                                        ("frozenStart",
                                         show(tRec->afrozenStart))
                                        ("frozenLen", tRec->afrozenLen);

  return result;
}

dynamic getPerfEvents(const PerfEventsMap<TransID>& transPerfEvents,
                      const TransID transId) {
  dynamic eventsObj = dynamic::object();
  auto const events = transPerfEvents.getAllEvents(transId);

  bool hasEvents = false;
  for (auto const c : events) {
    if (c) hasEvents = true;
  }
  if (!hasEvents) return eventsObj;

  auto const numEvents = getNumEventTypes();
  for (int i = 0; i < numEvents; i++) {
    auto const event = static_cast<PerfEventType>(i);
    auto const eventName = eventTypeToCommandLineArgument(event);
    eventsObj[eventName] = events[i];
  }

  return eventsObj;
}

dynamic getTrans(TransID transId) {
  always_assert(transId < NTRANS);

  auto const* tRec = TREC(transId);
  if (!tRec->isValid()) return dynamic();

  auto const transRec = getTransRec(tRec, transPerfEvents);
  auto const perfEvents = getPerfEvents(transPerfEvents, transId);

  dynamic blocks = dynamic::array;
  for (auto const& block : tRec->blocks) {
    std::stringstream byteInfo; // TODO(T52857125) - translate to actual data

    auto const unit = g_repo->getUnit(block.sha1);
    if (unit) {
      auto const newFunc = unit->getFunc(block.bcStart);
      always_assert(newFunc);
      newFunc->prettyPrint(byteInfo, HPHP::Func::PrintOpts().noMetadata());
      unit->prettyPrint(byteInfo, HPHP::Unit::PrintOpts().range(block.bcStart,
                                                                block.bcPast)
                                                         .noFuncs());
    }

    blocks.push_back(dynamic::object("sha1", block.sha1.toString())
                                    ("start", block.bcStart)
                                    ("end", block.bcPast)
                                    ("unit", unit ?
                                             byteInfo.str() :
                                             dynamic()));
  }

  auto const annotationDynamic = getAnnotation(tRec->annotations);

  auto const maybeUnit = [&]() -> folly::Optional<printir::Unit> {
    if (!annotationDynamic.isObject()) return folly::none;
    try {
      return folly::convertTo<printir::Unit>(annotationDynamic);
    }
    catch (const printir::ParseError& pe) {
      std::cerr << pe.what() << std::endl;
      return folly::none;
    }
  }();

  const dynamic mainDisasm = tRec->aLen ?
                             transCode->getDisasm(tRec->aStart,
                                                  tRec->aLen,
                                                  tRec->bcMapping,
                                                  tcaPerfEvents,
                                                  hostOpcodes,
                                                  maybeUnit) :
                             dynamic();

  auto const coldIsFrozen = tRec->acoldStart == tRec->afrozenStart;
  const dynamic coldDisasm = !coldIsFrozen && tRec->acoldLen ?
                             transCode->getDisasm(tRec->acoldStart,
                                                  tRec->acoldLen,
                                                  tRec->bcMapping,
                                                  tcaPerfEvents,
                                                  hostOpcodes,
                                                  maybeUnit) :
                             dynamic();
  const dynamic frozenDisasm = tRec -> afrozenLen ?
                               transCode->getDisasm(tRec->afrozenStart,
                                                    tRec->afrozenLen,
                                                    tRec->bcMapping,
                                                    tcaPerfEvents,
                                                    hostOpcodes,
                                                    maybeUnit) :
                               dynamic();
  const dynamic disasmObj = dynamic::object("main", mainDisasm)
                                           ("cold", coldDisasm)
                                           ("frozen", frozenDisasm);

  return dynamic::object("transRec", transRec)
                        ("blocks", blocks)
                        ("archName", transCode->getArchName())
                        ("regions", disasmObj)
                        ("perfEvents", perfEvents)
                        ("transId", transId)
                        ("ir_annotation", maybeUnit ?
                          folly::toDynamic(*maybeUnit) :
                          annotationDynamic);
  // if annotation fails to parse, give raw to the user in annotation field
}

dynamic getTC() {
  dynamic translations = dynamic::array;
  for (uint32_t t = 0; t < NTRANS; t++) {
    translations.push_back(getTrans(t));
  }

  return dynamic::object("configFile", configFile)
                        ("repoSchema", repoSchemaId().begin())
                        ("translations", translations);
}
}

// Prints the metadata, bytecode, and disassembly for the given translation
void printTrans(TransID transId) {
  always_assert(transId < NTRANS);

  g_logger->printGeneric("\n====================\n");
  g_transData->printTransRec(transId, transPerfEvents);

  const TransRec* tRec = TREC(transId);
  if (!tRec->isValid()) return;

  if (!tRec->blocks.empty()) {
    g_logger->printGeneric("----------\nbytecode:\n----------\n");
    const Func* curFunc = nullptr;
    for (auto& block : tRec->blocks) {
      std::stringstream byteInfo;
      auto unit = g_repo->getUnit(block.sha1);
      if (!unit) {
        byteInfo << folly::format(
          "<<< couldn't find unit {} to print bytecode range [{},{}) >>>\n",
          block.sha1, block.bcStart, block.bcPast);
        continue;
      }

      auto newFunc = unit->getFunc(block.bcStart);
      always_assert(newFunc);
      if (newFunc != curFunc) {
        byteInfo << '\n';
        newFunc->prettyPrint(byteInfo, Func::PrintOpts().noMetadata());
      }
      curFunc = newFunc;

      unit->prettyPrint(
        byteInfo,
        Unit::PrintOpts().range(block.bcStart, block.bcPast).noFuncs());
        g_logger->printBytecode(byteInfo.str());
    }
  }

  g_logger->printGeneric("----------\n%s: main\n----------\n",
                         transCode->getArchName());
  g_logger->printAsm("%s", getDisasmStr(transCode,
                                        tRec->aStart,
                                        tRec->aLen,
                                        tRec->bcMapping,
                                        tcaPerfEvents,
                                        hostOpcodes));

  g_logger->printGeneric("----------\n%s: cold\n----------\n",
                         transCode->getArchName());
  // Sometimes acoldStart is the same as afrozenStart.  Avoid printing the code
  // twice in such cases.
  if (tRec->acoldStart != tRec->afrozenStart) {
    g_logger->printAsm("%s", getDisasmStr(transCode,
                                          tRec->acoldStart,
                                          tRec->acoldLen,
                                          tRec->bcMapping,
                                          tcaPerfEvents,
                                          hostOpcodes));
  }

  g_logger->printGeneric("----------\n%s: frozen\n----------\n",
                         transCode->getArchName());
  g_logger->printAsm("%s", getDisasmStr(transCode,
                                        tRec->afrozenStart,
                                        tRec->afrozenLen,
                                        tRec->bcMapping,
                                        tcaPerfEvents,
                                        hostOpcodes));
  g_logger->printGeneric("----------\n");

}


void printCFGOutArcs(TransID transId) {
  std::vector<TCA> jmpTargets;

  TCA fallThru = transCode->getTransJmpTargets(
    g_transData->getTransRec(transId), &jmpTargets);

  auto const srcFuncId = TREC(transId)->src.funcID();

  for (size_t i = 0; i < jmpTargets.size(); i++) {
    TransID targetId = g_transData->getTransStartingAt(jmpTargets[i]);
    if (targetId != INVALID_ID &&
        // filter jumps to prologues of other funcs
        TREC(targetId)->src.funcID() == srcFuncId &&
        TREC(targetId)->kind != TransKind::Anchor) {

      bool retrans = (TREC(transId)->src.offset() ==
                      TREC(targetId)->src.offset());
      const char* color;
      if (retrans) {
        color = "darkorange";
      } else if (jmpTargets[i] == fallThru) {
        color = "brown";
      } else {
        color = "green4";
      }
      g_logger->printGeneric("t%u -> t%u [color=%s] ;\n", transId, targetId,
                             color);
    }
  }
}


void printCFG() {
  std::vector<TransID> inodes;

  g_logger->printGeneric("digraph CFG {\n");

  g_transData->findFuncTrans(selectedFuncId, &inodes);

  // Print nodes
  for (uint32_t i = 0; i < inodes.size(); i++) {
    auto tid = inodes[i];
    uint32_t bcStart   = TREC(tid)->src.offset();
    uint32_t bcStop    = TREC(tid)->bcPast();
    const auto kind = TREC(tid)->kind;
    bool isPrologue = kind == TransKind::LivePrologue ||
                      kind == TransKind::OptPrologue;
    const char* shape = "box";
    switch (TREC(tid)->kind) {
      case TransKind::Optimize:     shape = "oval";         break;
      case TransKind::Profile:      shape = "hexagon";      break;
      case TransKind::LivePrologue:
      case TransKind::ProfPrologue:
      case TransKind::OptPrologue : shape = "invtrapezium"; break;
      default:                      shape = "box";
    }
    g_logger->printGeneric("t%u [shape=%s,label=\"T: %u\\nbc: [0x%x-0x%x)\","
                           "style=filled%s];\n", tid, shape, tid, bcStart,
                           bcStop, (isPrologue ? ",color=blue" : ""));
  }

  // Print arcs
  for (uint32_t i = 0; i < inodes.size(); i++) {
    uint32_t tid = inodes[i];
    printCFGOutArcs(tid);
  }

  g_logger->printGeneric("}\n");
}

void printTopFuncs() {
  if (!nTopFuncs) return;
  std::map<FuncId,std::string> funcStrs;
  for (uint32_t t = 0; t < NTRANS; t++) {
    auto tRec = TREC(t);
    if (!tRec->isValid()) continue;
    auto funcId = tRec->src.funcID();
    if (funcStrs.count(funcId)) continue;
    auto funcName = tRec->funcName;
    funcStrs[funcId] = folly::sformat("{:<6}: {}", funcId, funcName);
  }
  TransToFuncMapper funcMapper(g_transData);
  PerfEventsMap<FuncId> funcPerfEvents = transPerfEvents.mapTo(funcMapper);
  funcPerfEvents.printEventsSummary(sortBy,
                                    "Function",
                                    72,
                                    nTopFuncs,
                                    verboseStats,
                                    helpersMinPercentage,
                                    funcStrs);
}

void printTopFuncsBySize() {
  std::unordered_map<FuncId,size_t> funcSize;
  FuncId maxFuncId = 0;
  for (TransID t = 0; t < NTRANS; t++) {
    const auto trec = TREC(t);
    if (trec->isValid()) {
      const auto funcId = trec->src.funcID();
      funcSize[funcId] += trec->aLen;
      if (funcId > maxFuncId) {
        maxFuncId = funcId;
      }
    }
  }
  std::vector<FuncId> funcIds(maxFuncId+1);
  for (FuncId fid = 0; fid <= maxFuncId; fid++) {
    funcIds[fid] = fid;
  }
  std::sort(funcIds.begin(), funcIds.end(),
            [&](FuncId fid1, FuncId fid2) {
    return funcSize[fid1] > funcSize[fid2];
  });
  g_logger->printGeneric("FuncID:   \tSize (total aLen in bytes):\n");
  for (size_t i = 0; i < nTopFuncs; i++) {
    const auto fid = funcIds[i];
    g_logger->printGeneric("%10u\t%10lu\n", fid, funcSize[funcIds[i]]);
  }
}

struct CompTrans {
  private:
  const PerfEventsMap<TransID>& transPerfEvents;
  const PerfEventType           etype;

  public:
  CompTrans(const PerfEventsMap<TransID>& _transPerfEvents,
            PerfEventType _etype) :
    transPerfEvents(_transPerfEvents), etype(_etype) {}

  bool operator()(TransID t1, TransID t2) const {
    const auto count1 = transPerfEvents.getEventCount(t1, etype);
    const auto count2 = transPerfEvents.getEventCount(t2, etype);
    if (sortByDensity) {
      const auto size1 = TREC(t1)->aLen;
      const auto size2 = TREC(t2)->aLen;
      return count1 * size2 > count2 * size1;
    }
    return count1 > count2;
  }
};

void printTopTrans() {
  if (!nTopTrans) return;

  // The summary currently includes all translations, so it's misleading
  // if we're filtering a specific kind of translations or address range.
  // It also doesn't sort by density, so do print it if sortByDensity is set.
  if (kindFilter == "all" && minAddr == 0 && maxAddr == (TCA)-1 &&
      !sortByDensity) {
    transPerfEvents.printEventsSummary(sortBy,
                                       "TransId",
                                       12,
                                       nTopTrans,
                                       verboseStats,
                                       helpersMinPercentage);
  }

  // Sort and print the top translations.
  std::vector<TransID> transIds;

  for (TransID t = 0; t < NTRANS; t++) {
    if (TREC(t)->isValid() &&
        (kindFilter == "all" || kindFilter == show(TREC(t)->kind).c_str()) &&
        ((minAddr <= TREC(t)->aStart     && TREC(t)->aStart      <= maxAddr) ||
         (minAddr <= TREC(t)->acoldStart && TREC(t)->acoldStart <= maxAddr))) {
      transIds.push_back(t);
    }
  }

  CompTrans compTrans(transPerfEvents, sortBy);
  sort(transIds.begin(), transIds.end(), compTrans);

  size_t nPrint = nTopTrans;
  if (transIds.size() < nTopTrans) {
    fprintf(stderr, "Warning: too few translations selected (%lu)\n",
            transIds.size());
    nPrint = transIds.size();
  }
  for (size_t i = 0; i < nPrint; i++) printTrans(transIds[i]);
}

void printBytecodeStats(const OfflineTransData* tdata,
                        const PerfEventsMap<TCA>& events,
                        PerfEventType etype) {

  if (!g_repo) error("printBytecodeStats: null repo");
  if (!tdata)  error("printBytecodeStats: null g_transData");

  AddrToBcMapper bcMapper(tdata);
  PerfEventsMap<ExtOpcode> bcPerfEvents = events.mapTo(bcMapper);

  std::map<ExtOpcode,std::string> opcodeToName;
  PerfEventsMap<ExtOpcode>::const_iterator it;

  for (it = bcPerfEvents.begin(); it != bcPerfEvents.end(); it++) {
    opcodeToName[it->first] = extOpcodeToString(it->first);
  }

  bcPerfEvents.printEventsSummary(etype,
                                  "Opcode",
                                  16,
                                  PerfEventsMap<ExtOpcode>::kAllEntries,
                                  verboseStats,
                                  helpersMinPercentage,
                                  opcodeToName);
}

void printTopBytecodes(const OfflineTransData* tdata,
                       OfflineCode* olCode,
                       const PerfEventsMap<TCA>& samples,
                       PerfEventType etype,
                       ExtOpcode filterBy) {

  always_assert(etype >= getFirstEventType() && etype < getNumEventTypes());

  AddrToTransFragmentMapper mapper(tdata, filterBy);
  PerfEventsMap<TransFragment> tfragPerfEvents = samples.mapTo(mapper);

  std::vector<std::pair<uint64_t, TransFragment> > ranking;
  PerfEventsMap<TransFragment>::const_iterator it;

  for (it = tfragPerfEvents.begin(); it != tfragPerfEvents.end(); it++) {
    ranking.push_back(std::make_pair(it->second[etype], it->first));
  }

  sort(ranking.rbegin(), ranking.rend());

  for (size_t i = 0; i < ranking.size(); i++) {
    const TransFragment& tfrag = ranking[i].second;
    const TransRec* trec = tdata->getTransRec(tfrag.tid);

    Unit* unit = g_repo->getUnit(trec->sha1);
    always_assert(unit);

    g_logger->printGeneric("\n====================\n");
    g_logger->printGeneric("{\n");
    g_logger->printGeneric("  FuncID  = %u\n", trec->src.funcID());
    g_logger->printGeneric("  TransID = %u\n", tfrag.tid);
    tfragPerfEvents.printEventsHeader(tfrag);
    g_logger->printGeneric("}\n\n");

    g_logger->printGeneric("----------\n%s: main\n----------\n",
                           olCode->getArchName());
    g_logger->printAsm("%s", getDisasmStr(olCode,
                                          tfrag.aStart,
                                          tfrag.aLen,
                                          trec->bcMapping,
                                          samples,
                                          hostOpcodes));

    g_logger->printGeneric("----------\n%s: cold\n----------\n",
                           olCode->getArchName());
    g_logger->printAsm("%s", getDisasmStr(olCode,
                                          tfrag.acoldStart,
                                          tfrag.acoldLen,
                                          trec->bcMapping,
                                          samples,
                                          hostOpcodes));

    g_logger->printGeneric("----------\n%s: frozen\n----------\n",
                           olCode->getArchName());
    g_logger->printAsm("%s", getDisasmStr(olCode,
                                          tfrag.afrozenStart,
                                          tfrag.afrozenLen,
                                          trec->bcMapping,
                                          samples,
                                          hostOpcodes));
  }
}

int main(int argc, char *argv[]) {
  folly::SingletonVault::singleton()->registrationComplete();

  pcre_init();

  // Initially use stdout logger while parsing arguments. Optionally switch
  // to DBLogger below if the printToDb argument was passed.
  StdLogger stdoutlogger{};
  g_logger = &stdoutlogger;

  parseOptions(argc, argv);

  #ifdef FACEBOOK
  folly::Optional<DBLogger> dblogger = folly::none;
  if (printToDB) {
    dblogger = DBLogger{};
    g_logger = &dblogger.value();
    g_logger->printGeneric("Printing to database");
  }
  #endif

  g_transData = new OfflineTransData(dumpDir);
  transCode = new OfflineCode(dumpDir,
                              g_transData->getHotBase(),
                              g_transData->getMainBase(),
                              g_transData->getProfBase(),
                              g_transData->getColdBase(),
                              g_transData->getFrozenBase());
  g_repo = new RepoWrapper(g_transData->getRepoSchema(), configFile, !useJSON);
  g_annotations = std::make_unique<AnnotationCache>(dumpDir);

  loadProfData();

  g_transData->setAnnotationsVerbosity(annotationsVerbosity);

  if (nTopFuncs) {
    if (nTopFuncs > NFUNCS) {
      warnTooFew("functions", nTopFuncs, NFUNCS);
      nTopFuncs = NFUNCS;
    }
    if (sortBySize) {
      printTopFuncsBySize();
    } else {
      printTopFuncs();
    }
  } else if (nTopTrans) {
    if (nTopTrans > NTRANS) {
      warnTooFew("translations", nTopTrans, NTRANS);
      nTopTrans = NTRANS;
    }
    printTopTrans();
  } else if (transCFG) {
    printCFG();
  } else if (creationOrder) {
    // Print translations (all or for a given funcId) in the order
    // they were created.
    sortTrans();
    for (uint32_t i=0; i < transPrintOrder.size(); i++) {
      printTrans(transPrintOrder[i]);
    }
  } else if (collectBCStats) {
    printBytecodeStats(g_transData, tcaPerfEvents, sortBy);
  } else if (filterByOpcode) {
    printTopBytecodes(g_transData,
                      transCode,
                      tcaPerfEvents,
                      sortBy,
                      filterByOpcode);
  } else if (useJSON) {
    auto const tcObj = get_json::getTC();

    auto const jsonStr = folly::toJson(tcObj);
    std::cout << jsonStr << std::endl;

    #ifdef FACEBOOK
    if (!hiveTable.empty()) {
      auto const uuid = boost::uuids::random_generator()();
      auto const uuidStr = boost::uuids::to_string(uuid);

      for (auto const& translation : tcObj["translations"]) {
        StructuredLogEntry entry;
        entry.force_init = true;

        entry.setStr("translation_json", folly::toJson(translation));
        entry.setInt("trans_id", translation["transId"].asInt());
        entry.setStr("repoSchema", repoSchemaId().begin());
        entry.setStr("func_name",
                     translation["transRec"]["src"]["funcName"].asString());
        entry.setStr("username" , std::string(getlogin()));
        entry.setStr("uuid", uuidStr);

        writeStructLogLazyInit(hiveTable, entry);
      }
    }
    #endif
  } else {
    // Print all translations in original order, filtered by unit if desired.
    for (uint32_t t = 0; t < NTRANS; t++) {
      auto tRec = TREC(t);
      if (!tRec->isValid()) continue;
      if (tRec->kind == TransKind::Anchor) continue;
      if (sha1Filter && tRec->sha1 != *sha1Filter) continue;

      printTrans(t);
      bool opt = (tRec->kind == TransKind::OptPrologue
                        || tRec->kind == TransKind::Optimize);
      g_logger->flushTranslation(tRec->funcName, opt);
  }
  }

  delete g_transData;
  delete transCode;
  delete g_repo;

  return 0;
}
