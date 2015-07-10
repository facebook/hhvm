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

#include "hphp/tools/tc-print/tc-print.h"

#include <stdio.h>
#include <assert.h>

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>

#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/base/thread-init-fini.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/program-functions.h"

#include "hphp/tools/tc-print/perf-events.h"
#include "hphp/tools/tc-print/offline-trans-data.h"
#include "hphp/tools/tc-print/offline-x86-code.h"
#include "hphp/tools/tc-print/mappers.h"
#include "hphp/tools/tc-print/repo-wrapper.h"

using namespace HPHP;
using namespace HPHP::jit;

#define MAX_SYM_LEN       10240

std::string     dumpDir("/tmp");
std::string     configFile;
std::string     profFileName;
uint32_t        nTopTrans       = 0;
uint32_t        nTopFuncs       = 0;
bool            srcOrder        = false;
bool            transCFG        = false;
bool            collectBCStats  = false;
bool            inclusiveStats  = false;
bool            verboseStats    = false;
folly::Optional<MD5> md5Filter;
PerfEventType   sortBy          = SPECIAL_PROF_COUNTERS;
double          helpersMinPercentage = 0;
ExtOpcode       filterByOpcode  = 0;
std::string     kindFilter      = "all";
uint32_t        selectedFuncId  = INVALID_ID;
TCA             minAddr         = 0;
TCA             maxAddr         = (TCA)-1;
uint32_t        annotationsVerbosity = 2;

std::vector<uint32_t>    transSortSrc;

RepoWrapper*      g_repo;
OfflineTransData* g_transData;
OfflineX86Code*   transCode;

char errMsgBuff[MAX_SYM_LEN];
const char* kListKeyword = "list";

PerfEventsMap<TCA>     tcaPerfEvents;
PerfEventsMap<TransID> transPerfEvents;

#define NTRANS        (g_transData->getNumTrans())
#define NFUNCS        (g_transData->getNumFuncs())
#define TREC(TID)     (g_transData->getTransRec(TID))

void error(const std::string& msg) {
  fprintf(stderr, "Error: %s\n", msg.c_str());
  exit(1);
}

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
         "    -d <DIRECTORY>  : looks for dump file in <DIRECTORY> "
         "(default: /tmp)\n"
         "    -f <FUNC_ID>    : prints the translations for the given "
         "<FUNC_ID>, sorted by start offset\n"
         "    -g <FUNC_ID>    : prints the CFG among the translations for the "
         "given <FUNC_ID>\n"
         "    -p <FILE>       : uses raw profile data from <FILE>\n"
         "    -s              : prints all translations sorted by source key "
         "(md5, startOffset)\n"
         "    -u <MD5>        : prints all translations from the specified "
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
         "    -e <EVENT_TYPE> : sorts by the specified perf event. Pass '%s' "
         "to get a list of valid event types.\n"
         "    -b              : prints bytecode stats\n"
         "    -B <OPCODE>     : used in conjunction with -e, prints the top "
         "bytecode translationc event type. Pass '%s' to get a "
         "list of valid opcodes.\n"
         "    -i              : reports inclusive stats by including helpers "
         "(perf data must include call graph information)\n"
         "    -n <level>      : level of verbosity for annotations. Use 0 for "
         "no annotations, 1 - for inline, 2 - to print all annotations "
         "including from a file (default: 2).\n"
         "    -v <PERCENTAGE> : sets the minimum percentage to <PERCENTAGE> "
         "when printing the top helpers (implies -i). The lower the percentage,"
         " the more helpers that will show up.\n"
         "    -h              : prints help message\n",
         kListKeyword,
         kListKeyword);
}

void printValidBytecodes() {
  printf("<OPCODE>:\n");
  auto validOpcodes = getValidOpcodeNames();
  for (size_t i = 0; i < validOpcodes.size(); i++) {
    printf("  * %s\n", validOpcodes[i].first.c_str());
  }
}

void printValidEventTypes() {
  printf("<EVENT_TYPE>:\n");
  for (size_t i = 0; i < NUM_EVENT_TYPES; i++) {
    printf("  * %s\n", eventTypeToCommandLineArgument((PerfEventType)i));
  }
}

void parseOptions(int argc, char *argv[]) {
  int c;
  opterr = 0;
  while ((c = getopt (argc, argv, "hc:d:f:g:ip:st:u:T:o:e:bB:v:k:a:A:n:"))
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
        srcOrder = true;
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
        srcOrder = true;
        break;
      case 't':
        if (sscanf(optarg, "%u", &nTopTrans) != 1) {
          usage();
          exit(1);
        }
        break;
      case 'u':
        if (strlen(optarg) == 32) {
          md5Filter = MD5(optarg);
        } else {
          usage();
          exit(1);
        }
        break;
      case 'T':
        if (sscanf(optarg, "%u", &nTopFuncs) != 1) {
          usage();
          exit(1);
        }
        break;
      case 'k':
        kindFilter = optarg;
        break;
      case 'e':
        if (!strcmp(optarg, kListKeyword)) {
          printValidEventTypes();
          exit(0);
        }
        sortBy = commandLineArgumentToEventType(optarg);
        if (sortBy == NUM_EVENT_TYPES) {
          usage();
          exit(1);
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
      default:
        usage();
        exit(1);
    }
  }
}

int compTransSrc(int i, int j) {
  int cmp = strcmp(TREC(i)->md5.toString().c_str(),
                   TREC(j)->md5.toString().c_str());
  if (cmp) return cmp < 0;
  cmp = TREC(i)->src.offset() - TREC(j)->src.offset();
  if (cmp) return cmp < 0;
  return TREC(i)->id < TREC(j)->id;
}

void sortSrc() {
  for (uint32_t tid = 0; tid < NTRANS; tid++) {
    if (selectedFuncId == INVALID_ID ||
        (selectedFuncId == TREC(tid)->src.funcID())) {
      transSortSrc.push_back(tid);
    }
  }
  sort(transSortSrc.begin(), transSortSrc.end(), compTransSrc);
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
  uint32_t tcSamples[NUM_EVENT_TYPES];
  uint32_t hhvmSamples[NUM_EVENT_TYPES];
  size_t numEntries = 0;
  PerfEventType eventType = NUM_EVENT_TYPES;
  // samplesPerKind[event][kind][isLLVM]
  uint32_t samplesPerKind[NUM_EVENT_TYPES][NumTransKinds][2];
  uint32_t samplesPerTCRegion[NUM_EVENT_TYPES][TCRCount];

  memset(tcSamples  , 0, sizeof(tcSamples));
  memset(hhvmSamples, 0, sizeof(hhvmSamples));
  memset(samplesPerKind, 0, sizeof(samplesPerKind));
  memset(samplesPerTCRegion, 0, sizeof(samplesPerTCRegion));

  while (fgets(line, 2*MAX_SYM_LEN, profFile) != nullptr) {
    always_assert(sscanf(line, "%s %s %lu", program, eventCaption, &numEntries)
                  == 3);
    always_assert(numEntries);

    std::vector<std::pair<TCA,std::string>> entries;

    for (size_t i = 0; i < numEntries; i++) {
      fscanf(profFile, "%p %s\n", &addr, line);
      entries.push_back(std::pair<TCA,std::string>(addr, line));
    }

    if (strncmp(program, "hhvm", 4) == 0) {
      eventType = perfScriptOutputToEventType(eventCaption);
      if (eventType == NUM_EVENT_TYPES) {
        snprintf(errMsgBuff,
                 MAX_SYM_LEN,
                 "loadProfData: invalid event caption '%s'",
                 eventCaption);
        error(errMsgBuff);
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
      samplesPerKind[eventType][static_cast<uint32_t>(kind)][trec->isLLVM]++;
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

  printf("# Number of hhvm samples read (%% in TC) from file %s\n",
         profFileName.c_str());

  for (size_t i = 0; i < NUM_EVENT_TYPES; i++) {
    if (!hhvmSamples[i]) continue;

    printf("#  %-19s TOTAL: %10u (%u in TC = %5.2lf%%)\n",
           eventTypeToCommandLineArgument((PerfEventType)i),
           hhvmSamples[i],
           tcSamples[i],
           100.0 * tcSamples[i] / hhvmSamples[i]);

    for (size_t j = 0; j < NumTransKinds; ++j) {
      for (size_t llvm = 0; llvm <= 1; ++llvm) {
        auto ct = samplesPerKind[i][j][llvm];
        if (!ct) continue;
        std::string kind = show(static_cast<TransKind>(j));
        if (llvm) kind = "LLVM " + kind;
        printf("# %26s:             %-8u (%5.2lf%%)\n",
               kind.c_str(), ct, 100.0 * ct / tcSamples[i]);
      }
    }
    printf("#\n");
  }
  printf("\n");

  // print per-TCRegion information

  // header
  printf("# TCRegion ");
  for (size_t i = 0; i < NUM_EVENT_TYPES; i++) {
    printf("%17s ", eventTypeToCommandLineArgument((PerfEventType)i));
  }
  printf("\n");

  // HW events for each region
  for (size_t i = 0 ; i < TCRCount ; i++) {
    printf("# %8s ", tcRegionToString(static_cast<TCRegion>(i)).c_str());
    for (size_t j = 0; j < NUM_EVENT_TYPES; j++) {
      auto ct = samplesPerTCRegion[j][i];
      printf("%8u (%5.2lf%%) ", ct, ct ? (100.0 * ct / tcSamples[j]) : 0);
    }
    printf("\n");
  }
  printf("#\n\n");

  fclose(profFile);
}

void loadProfData() {
  if (!profFileName.empty()) {
    loadPerfEvents();
  }

  // The prof-counters are collected independently.
  for (TransID tid = 0; tid < NTRANS; tid++) {
    PerfEvent profCounters;
    profCounters.type  = SPECIAL_PROF_COUNTERS;
    profCounters.count = g_transData->getTransCounter(tid);
    transPerfEvents.addEvent(tid, profCounters);
  }
}

// Prints the metadata, bytecode, and disassembly for the given translation
void printTrans(TransID transId) {
  always_assert(transId < NTRANS);

  printf("\n====================\n");
  g_transData->printTransRec(transId, transPerfEvents);

  const TransRec* tRec = TREC(transId);

  if (!tRec->blocks.empty()) {
    printf("----------\nbytecode:\n----------\n");
    const Func* curFunc = nullptr;
    for (auto& block : tRec->blocks) {
      auto unit = g_repo->getUnit(block.md5);
      if (!unit) {
        std::cout << folly::format(
          "<<< couldn't find unit {} to print bytecode range [{},{}) >>>\n",
          block.md5, block.bcStart, block.bcPast);
        continue;
      }

      auto newFunc = unit->getFunc(block.bcStart);
      always_assert(newFunc);
      if (newFunc != curFunc) {
        std::cout << '\n';
        newFunc->prettyPrint(std::cout, Func::PrintOpts().noFpi().noMetadata());
      }
      curFunc = newFunc;

      unit->prettyPrint(
        std::cout, Unit::PrintOpts().range(block.bcStart, block.bcPast)
                               .noFuncs());
    }
  }

  printf("----------\nx64: main\n----------\n");
  transCode->printDisasm(tRec->aStart, tRec->aLen,
                         tRec->bcMapping, tcaPerfEvents);

  printf("----------\nx64: cold\n----------\n");
  transCode->printDisasm(tRec->acoldStart, tRec->acoldLen,
                         tRec->bcMapping, tcaPerfEvents);

  printf("----------\nx64: frozen\n----------\n");
  transCode->printDisasm(tRec->afrozenStart, tRec->afrozenLen,
                         tRec->bcMapping, tcaPerfEvents);

  printf("----------\n");
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
      printf("t%u -> t%u [color=%s] ;\n", transId, targetId, color);
    }
  }
}


void printCFG() {
  std::vector<TransID> inodes;

  printf("digraph CFG {\n");

  uint64_t maxProfCount = g_transData->findFuncTrans(selectedFuncId, &inodes);

  // Print nodes
  for (uint32_t i = 0; i < inodes.size(); i++) {
    auto tid = inodes[i];
    uint64_t profCount = g_transData->getTransCounter(tid);
    uint32_t bcStart   = TREC(tid)->src.offset();
    uint32_t bcStop    = TREC(tid)->bcPast();
    uint32_t coldness  = 255 - (255 * profCount / maxProfCount);
    bool isPrologue = TREC(tid)->kind == TransKind::Prologue;
    const char* shape = "box";
    switch (TREC(tid)->kind) {
      case TransKind::Optimize: shape = "oval";         break;
      case TransKind::Profile:  shape = "hexagon";      break;
      case TransKind::Proflogue:
      case TransKind::Prologue: shape = "invtrapezium"; break;
      default:                  shape = "box";
    }
    printf("t%u [shape=%s,label=\"T: %u\\np: %" PRIu64 "\\nbc: [0x%x-0x%x)\","
           "style=filled,fillcolor=\"#ff%02x%02x\"%s];\n", tid, shape, tid,
           profCount, bcStart, bcStop, coldness, coldness,
           (isPrologue ? ",color=blue" : ""));
  }

  // Print arcs
  for (uint32_t i = 0; i < inodes.size(); i++) {
    uint32_t tid = inodes[i];
    printCFGOutArcs(tid);
  }

  printf("}\n");
}

void printTopFuncs() {
  if (!nTopFuncs) return;
  TransToFuncMapper funcMapper(g_transData);
  PerfEventsMap<FuncId> funcPerfEvents = transPerfEvents.mapTo(funcMapper);
  funcPerfEvents.printEventsSummary(sortBy,
                                    "FuncId",
                                    nTopFuncs,
                                    verboseStats,
                                    helpersMinPercentage);
}

class CompTrans {
  const PerfEventsMap<TransID>& transPerfEvents;
  const PerfEventType           etype;

public:
  CompTrans(const PerfEventsMap<TransID>& _transPerfEvents,
            PerfEventType _etype) :
    transPerfEvents(_transPerfEvents), etype(_etype) {}

  bool operator()(TransID t1, TransID t2) const {
    return transPerfEvents.getEventCount(t1, etype) >
           transPerfEvents.getEventCount(t2, etype);
  }
};

void printTopTrans() {
  if (!nTopTrans) return;

  // The summary currently includes all translations, so it's misleading
  // if we're filtering a specific kind of translations or address range.
  if (kindFilter == "all" && minAddr == 0 && maxAddr == (TCA)-1) {
    transPerfEvents.printEventsSummary(sortBy,
                                       "TransId",
                                       nTopTrans,
                                       verboseStats,
                                       helpersMinPercentage);
  }

  // Sort and print the top translations.
  std::vector<TransID> transIds;

  for (TransID t = 0; t < NTRANS; t++) {
    if ((kindFilter == "all" || kindFilter == show(TREC(t)->kind).c_str())
        &&
        ((minAddr <= TREC(t)->aStart      && TREC(t)->aStart      <= maxAddr) ||
         (minAddr <= TREC(t)->acoldStart && TREC(t)->acoldStart <= maxAddr)))
    {
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
                                  PerfEventsMap<ExtOpcode>::kAllEntries,
                                  verboseStats,
                                  helpersMinPercentage,
                                  opcodeToName);
}

void printTopBytecodes(const OfflineTransData* tdata,
                       OfflineX86Code* x86code,
                       const PerfEventsMap<TCA>& samples,
                       PerfEventType etype,
                       ExtOpcode filterBy) {

  always_assert(etype < NUM_EVENT_TYPES);

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

    Unit* unit = g_repo->getUnit(trec->md5);
    always_assert(unit);

    printf("\n====================\n");
    printf("{\n");
    printf("  FuncID  = %u\n", trec->src.funcID());
    printf("  TransID = %u\n", tfrag.tid);
    tfragPerfEvents.printEventsHeader(tfrag);
    printf("}\n\n");

    printf("----------\nx64: main\n----------\n");
    x86code->printDisasm(tfrag.aStart,
                         tfrag.aLen,
                         trec->bcMapping,
                         samples);

    printf("----------\nx64: cold\n----------\n");
    x86code->printDisasm(tfrag.acoldStart,
                         tfrag.acoldLen,
                         trec->bcMapping,
                         samples);

    printf("----------\nx64: frozen\n----------\n");
    x86code->printDisasm(tfrag.afrozenStart,
                         tfrag.afrozenLen,
                         trec->bcMapping,
                         samples);
  }
}

int main(int argc, char *argv[]) {
  pcre_init();

  parseOptions(argc, argv);

  g_transData = new OfflineTransData(dumpDir);
  transCode = new OfflineX86Code(dumpDir,
                                 g_transData->getHotBase(),
                                 g_transData->getMainBase(),
                                 g_transData->getProfBase(),
                                 g_transData->getColdBase(),
                                 g_transData->getFrozenBase());
  g_repo = new RepoWrapper(g_transData->getRepoSchema(), configFile);

  loadProfData();

  g_transData->setAnnotationsVerbosity(annotationsVerbosity);

  if (nTopFuncs) {
    if (nTopFuncs > NFUNCS) {
      warnTooFew("functions", nTopFuncs, NFUNCS);
      nTopFuncs = NFUNCS;
    }
    printTopFuncs();
  } else if (nTopTrans) {
    if (nTopTrans > NTRANS) {
      warnTooFew("translations", nTopTrans, NTRANS);
      nTopTrans = NTRANS;
    }
    printTopTrans();
  } else if (transCFG) {
    printCFG();
  } else if (srcOrder) {
    // Print translations (all or for a given funcId) in source order
    sortSrc();
    for (uint32_t i=0; i < transSortSrc.size(); i++) {
      printTrans(transSortSrc[i]);
    }
  } else if (collectBCStats) {
    printBytecodeStats(g_transData, tcaPerfEvents, sortBy);
  } else if (filterByOpcode) {
    printTopBytecodes(g_transData,
                      transCode,
                      tcaPerfEvents,
                      sortBy,
                      filterByOpcode);
  } else {
    // Print all translations in original order, filtered by unit if desired.
    for (uint32_t t = 0; t < NTRANS; t++) {
      auto tRec = TREC(t);
      if (tRec->kind == TransKind::Anchor) continue;
      if (md5Filter && tRec->md5 != *md5Filter) continue;

      printTrans(t);
    }
  }

  delete g_transData;
  delete transCode;
  delete g_repo;

  return 0;
}
