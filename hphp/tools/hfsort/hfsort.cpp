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

#include "hphp/tools/hfsort/hfutil.h"

#include <ctype.h>
#include <cxxabi.h>
#include <stdio.h>
#include <zlib.h>

#include <fstream>
#include <unordered_map>

#include <folly/Format.h>
#include <folly/String.h>

#include "hphp/util/text-util.h"

namespace HPHP { namespace hfsort {

constexpr uint32_t BUFLEN = 1000;
constexpr uint32_t kPageSize = 2 << 20;

void error(const char* msg) {
  printf("ERROR: %s\n", msg);
  exit(1);
}

void readSymbols(CallGraph& cg, FILE* file) {
  char     line[BUFLEN];
  char     name[BUFLEN];
  uint64_t addr;
  uint32_t size;
  char     kind;
  while (fgets(line, BUFLEN, file)) {
    if (sscanf(line, "%lx %x %c %s", &addr, &size, &kind, name) == 4) {
      int status;
      char* demangledName = abi::__cxa_demangle(name, 0, 0, &status);
      if (demangledName) {
        free(demangledName);
      } else {
        if (!std::isalpha((uint8_t)*name) && *name != '_') continue;
        for (const char* n = name; *++n; ) {
          if (!std::isalnum((uint8_t)*n) && *n != '_' && *n != '.') {
            continue;
          }
        }
      }
      cg.addFunc(name, addr, size, 0);
    }
  }
}

uint64_t getAddr(char* perfLine) {
  uint64_t addr;
  int ret = sscanf(perfLine, "%" SCNx64, &addr);
  if (ret != 1) return InvalidAddr;
  return addr;
}

TargetId getTargetId(const CallGraph& cg, uint64_t addr) {
  if (addr == InvalidAddr) return InvalidId;
  return cg.addrToTargetId(addr);
}

void readPerfData(CallGraph& cg, gzFile file, bool computeArcWeight) {
  char line[BUFLEN];

  while (gzgets(file, line, BUFLEN) != Z_NULL) {
    HFTRACE(2, "readPerfData: line: %s\n", line);
    if (line[0] == '#') continue;
    if (isspace(line[0])) continue;

    // process one sample
    if (gzgets(file, line, BUFLEN) == Z_NULL) error("reading perf data");
    auto addrTop = getAddr(line);
    TargetId idTop = getTargetId(cg, addrTop);
    if (idTop == InvalidId) continue;
    cg.targets[idTop].samples++;
    HFTRACE(2, "readPerfData: idTop: %u %s\n", idTop,
            cg.funcs[idTop].mangledNames[0].c_str());
    if (gzgets(file, line, BUFLEN) == Z_NULL) error("reading perf data");
    auto addrCaller = getAddr(line);
    TargetId idCaller = getTargetId(cg, addrCaller);
    if (idCaller != InvalidId) {
      auto& arc = cg.incArcWeight(idCaller, idTop, computeArcWeight ? 1 : 0);
      if (computeArcWeight) {
        arc.avgCallOffset += addrCaller - cg.funcs[idCaller].addr;
      }
      HFTRACE(2, "readPerfData: idCaller: %u %s\n", idCaller,
              cg.funcs[idCaller].mangledNames[0].c_str());
    }
  }

  if (!computeArcWeight) return;

  cg.normalizeArcWeights();
}

void readSymbolCntData(CallGraph& cg, std::ifstream& file) {
  std::string line;
  while (std::getline(file, line)) {
    std::string caller, top;
    int32_t count;
    folly::split(" ", line, caller, top, count);
    TargetId idTop = cg.funcToTargetId(top);
    if (idTop == InvalidId) continue;
    cg.targets[idTop].samples += count;
    HFTRACE(2, "readSymbolCntData: idTop: %u %s\n", idTop,
            cg.funcs[idTop].mangledNames[0].c_str());

    TargetId idCaller = cg.funcToTargetId(caller);
    if (idCaller == InvalidId) continue;
    cg.incArcWeight(idCaller, idTop, count);
    HFTRACE(2, "readSymbolCntData: idCaller: %u %s\n", idCaller,
            cg.funcs[idCaller].mangledNames[0].c_str());
  }
  cg.normalizeArcWeights();
}

void readEdgcntData(CallGraph& cg, FILE* file) {
  char     line[BUFLEN];
  uint64_t srcAddr;
  uint64_t dstAddr;
  char     kind;
  uint32_t count;

  HFTRACE(1, "=== use edgcnt profile to build callgraph\n\n");

  while (fgets(line, BUFLEN, file)) {
    auto const res =
    sscanf(line, "%lx %lx %c %u %*x", &srcAddr, &dstAddr, &kind, &count);
    if (res == 4) {
      if (kind != 'C') continue;

      // process one sample
      TargetId caller = cg.addrToTargetId(srcAddr);
      TargetId callee = cg.addrToTargetId(dstAddr);
      if (caller != InvalidId && callee != InvalidId) {
        cg.incArcWeight(caller, callee, count);
      }
    }
  }

  // Normalize incoming arc weights for each node.
  for (TargetId f = 0; f < cg.targets.size(); f++) {
    auto& func = cg.targets[f];
    for (auto src : func.preds) {
      auto& arc = *cg.arcs.find(Arc(src, f));
      arc.normalizedWeight = arc.weight / func.samples;
    }
  }
}

std::string getNameWithoutSuffix(std::string str) {
  int suffixStartPosition = str.find(".");
  if (suffixStartPosition == -1) {
    // if no suffix is found, just add the wildcard
    return str + "*";
  } else {
    // replace sufix with wildcard
    return str.substr(0, suffixStartPosition) + "*";
  }
}

void print(CallGraph& cg, const char* filename,
           const std::vector<Cluster>& clusters, bool useWildcards) {
  FILE* outfile = fopen(filename, "wt");
  if (!outfile) {
    error(folly::sformat("opening output file {}", filename).c_str());
  }
  uint32_t totalSize   = 0;
  uint32_t curPage     = 0;
  uint32_t hotfuncs    = 0;
  double totalDistance = 0;
  double totalCalls    = 0;
  double totalCalls64B = 0;
  double totalCalls4KB = 0;
  double totalCalls2MB = 0;
  std::unordered_map<TargetId,uint64_t> newAddr;
  for (auto& cluster : clusters) {
    for (auto fid : cluster.targets) {
      if (cg.targets[fid].samples > 0) {
        newAddr[fid] = totalSize;
        totalSize += cg.targets[fid].size;
      }
    }
  }
  totalSize = 0;
  HFTRACE(1, "============== page 0 ==============\n");
  for (auto& cluster : clusters) {
    HFTRACE(1,
            "-------- density = %.3lf (%u / %u) --------\n",
            (double) cluster.samples / cluster.size,
            cluster.samples, cluster.size);
    for (auto fid : cluster.targets) {
      if (cg.targets[fid].samples > 0) {
        hotfuncs++;
        int space = 0;
        for (const auto& mangledName : cg.funcs[fid].mangledNames) {
          if (useWildcards) {
            fprintf(outfile, "%.*s.text.%s\n",
                    space, " ", getNameWithoutSuffix(mangledName).c_str());
          } else {
            fprintf(outfile, "%.*s.text.%s\n",
                    space, " ", mangledName.c_str());
          }

          space = 1;
        }
        uint64_t dist = 0;
        uint64_t calls = 0;
        for (auto dst : cg.targets[fid].succs) {
          auto& arc = *cg.arcs.find(Arc(fid, dst));
          auto d = std::abs(newAddr[arc.dst] -
                            (newAddr[fid] + arc.avgCallOffset));
          auto w = arc.weight;
          calls += w;
          if (d < 64)      totalCalls64B += w;
          if (d < 4096)    totalCalls4KB += w;
          if (d < 2 << 20) totalCalls2MB += w;
          HFTRACE(
            2,
            "arc: %u [@%lu+%.1lf] -> %u [@%lu]: weight = %.0lf, "
            "callDist = %f\n",
            arc.src, newAddr[arc.src], arc.avgCallOffset,
            arc.dst, newAddr[arc.dst], arc.weight, d);
          dist += arc.weight * d;
        }
        totalCalls += calls;
        totalDistance += dist;
        HFTRACE(1, "start = %6u : avgCallDist = %lu : %s\n",
                totalSize,
                calls ? dist / calls : 0,
                cg.toString(fid).c_str());
        totalSize += cg.targets[fid].size;
        uint32_t newPage = totalSize / kPageSize;
        if (newPage != curPage) {
          curPage = newPage;
          HFTRACE(1, "============== page %u ==============\n", curPage);
        }
      }
    }
  }
  fclose(outfile);
  printf("Output saved in file %s\n", filename);
  printf("  Number of hot functions: %u\n  Number of clusters: %lu\n",
         hotfuncs, clusters.size());
  printf("  Final average call distance = %.1lf (%.0lf / %.0lf)\n",
         totalCalls ? totalDistance / totalCalls : 0,
         totalDistance, totalCalls);
  printf("  Total Calls = %.0lf\n", totalCalls);
  if (totalCalls) {
    printf("  Total Calls within 64B = %.0lf (%.2lf%%)\n",
           totalCalls64B, 100 * totalCalls64B / totalCalls);
    printf("  Total Calls within 4KB = %.0lf (%.2lf%%)\n",
           totalCalls4KB, 100 * totalCalls4KB / totalCalls);
    printf("  Total Calls within 2MB = %.0lf (%.2lf%%)\n",
           totalCalls2MB, 100 * totalCalls2MB / totalCalls);
  }
}

Algorithm checkAlgorithm(const char* algorithm) {
  auto a = HPHP::toLower(algorithm);

  if (a == "hfsort") return Algorithm::Hfsort;
  if (a == "hfsortplus") return Algorithm::HfsortPlus;
  if (a == "pettishansen") return Algorithm::PettisHansen;

  return Algorithm::Invalid;
}

}}

int main(int argc, char* argv[]) {
  using namespace HPHP::hfsort;

  CallGraph cg;
  char* symbFileName = nullptr;
  char* perfFileName = nullptr;
  char* edgcntFileName = nullptr;
  char* symbolCntFileName = nullptr;
  bool useWildcards = false;

  FILE*  symbFile;
  gzFile perfFile = Z_NULL;
  FILE*  edgcntFile;

  Algorithm algorithm = Algorithm::Hfsort;

  // parse commandline arguments

  extern char* optarg;
  extern int optind;
  int c;
  while ((c = getopt(argc, argv, "pae:s:w")) != -1) {
    switch (c) {
      case 'p':
        algorithm = Algorithm::PettisHansen;
        break;
      case 'a':
        algorithm = Algorithm::HfsortPlus;
        break;
      case 'e':
        edgcntFileName = optarg;
        break;
      case 's':
        symbolCntFileName = optarg;
        break;
      case 'w':
        useWildcards = true;
        break;
      case '?':
        error("Unsupported command line argument");
    }
  }

  if (argc < optind + 1 || argc > optind + 2) {
    error(
      "Usage: hfsort [-p] [-e <EDGCNT_FILE>] [-s <SYMBOLCNT_FILE>] [-w]"
      " <SYMBOL_FILE> [<PERF_DATA_FILE>]\n"
      "   -p,               use pettis-hansen algorithm for code layout\n"
      "   -a,               use hfsort-plus algorithm for code layout\n"
      "   -e <EDGCNT_FILE>, use edge profile result to build the call graph\n"
      "   -s <SYMBOL_CNT_FILE>  use a file with 'symbol symbol count'\n"
      "                     triplets to build the call graph \n"
      "   -w                use wildcards instead of suffixes in function names"
    );
  }

  symbFileName = argv[optind];
  if (argc == optind + 2) {
    perfFileName = argv[optind + 1];
  }
  if (!(symbFile = fopen(symbFileName, "rt"))) {
    error("Error opening symbol file\n");
  }
  if (perfFileName && !(perfFile = gzopen(perfFileName, "r"))) {
    error("Error opening perf data file\n");
  }
  if (edgcntFileName && !(edgcntFile = fopen(edgcntFileName, "rt"))) {
    error("Error opening edge count file\n");
  }
  std::ifstream symbolCntFile(symbolCntFileName);
  if (symbolCntFileName && !symbolCntFile.is_open()) {
    error("Error opening symbol count file\n");
  }

  readSymbols(cg, symbFile);
  if (symbolCntFileName != nullptr) {
    readSymbolCntData(cg, symbolCntFile);
    symbolCntFile.close();
  }
  if (perfFileName != nullptr) {
    readPerfData(cg, perfFile, (edgcntFileName == nullptr));
  }
  if (edgcntFileName != nullptr) {
    readEdgcntData(cg, edgcntFile);
    fclose(edgcntFile);
  }
  cg.printDot("cg.dot",
              [&](TargetId id) {
                return cg.funcs[id].mangledNames[0].c_str();
              });

  std::vector<Cluster> clusters;

  const char* filename;
  if (algorithm == Algorithm::Hfsort) {
    HFTRACE(1, "=== algorithm : hfsort\n\n");
    clusters = clusterize(cg);
    filename = "hotfuncs.txt";
  } else if (algorithm == Algorithm::HfsortPlus) {
    HFTRACE(1, "=== algorithm : hfsort-plus\n\n");
    clusters = hfsortPlus(cg);
    filename = "hotfuncs.txt";
  } else if (algorithm == Algorithm::PettisHansen) {
    HFTRACE(1, "=== algorithm : pettis-hansen\n\n");
    clusters = pettisAndHansen(cg);
    filename = "hotfuncs-pettis.txt";
  } else {
    error("Unknown layout algorithm\n");
  }

  sort(clusters.begin(), clusters.end(), compareClustersDensity);
  print(cg, filename, clusters, useWildcards);

  fclose(symbFile);
  if (perfFile != Z_NULL) {
    gzclose(perfFile);
  }

  return 0;
}
