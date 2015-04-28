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

#include "hphp/tools/hfsort/hfutil.h"

#include <stdio.h>
#include <assert.h>
#include <zlib.h>
#include <ctype.h>
#include <stdarg.h>

#include <folly/Format.h>
#include "hphp/util/text-util.h"

namespace HPHP { namespace hfsort {

void error(const char* msg) {
  printf("ERROR: %s\n", msg);
  exit(1);
}

void trace(const char* fmt, ...) {
  va_list args;

  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}

void readSymbols(FILE *file) {
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
      cg.addFunc(Func(cg.funcs.size(), name, addr, size, 0));
    }
  }
}

FuncId getFuncId(char* perfLine) {
  uint64_t addr;
  int ret = sscanf(perfLine, "%" SCNx64, &addr);
  if (ret != 1) return InvalidId;
  return cg.addrToFuncId(addr);
}

void readPerfData(gzFile file, bool computeArcWeight) {
  char line[BUFLEN];

  while (gzgets(file, line, BUFLEN) != Z_NULL) {
    HFTRACE(2, "readPerfData: line: %s\n", line);
    if (line[0] == '#') continue;
    if (isspace(line[0])) continue;

    // process one sample
    if (gzgets(file, line, BUFLEN) == Z_NULL) error("reading perf data");
    FuncId idTop = getFuncId(line);
    if (idTop == InvalidId) continue;
    cg.funcs[idTop].samples++;
    HFTRACE(2, "readPerfData: idTop: %u %s\n", idTop,
            cg.funcs[idTop].mangledNames[0].c_str());
    if (gzgets(file, line, BUFLEN) == Z_NULL) error("reading perf data");
    FuncId idCaller = getFuncId(line);
    if (idCaller != InvalidId) {
      if (computeArcWeight) cg.incArcWeight(idCaller, idTop);
      HFTRACE(2, "readPerfData: idCaller: %u %s\n", idCaller,
              cg.funcs[idCaller].mangledNames[0].c_str());
    }
  }

  if (!computeArcWeight) return;

  // Normalize incoming arc weights for each node.
  for (auto& func : cg.funcs) {
    for (auto arc : func.inArcs) {
      arc->normalizedWeight = arc->weight / func.samples;
    }
  }
}

void readEdgcntData(FILE* file) {
  char     line[BUFLEN];
  uint64_t src;
  uint64_t dst;
  char     kind;
  uint32_t count;

  HFTRACE(1, "=== use edgcnt profile to build callgraph\n\n");

  while (fgets(line, BUFLEN, file)) {
    if (sscanf(line, "%lx %lx %c %u %*x", &src, &dst, &kind, &count) == 4) {
      if (kind != 'C') continue;

      // process one sample
      FuncId caller = cg.addrToFuncId(src);
      FuncId callee = cg.addrToFuncId(dst);
      if (caller != InvalidId && callee != InvalidId) {
        cg.incArcWeight(caller, callee, count);
      }
    }
  }

  // Normalize incoming arc weights for each node.
  for (size_t f = 0; f < cg.funcs.size(); f++) {
    Func& func = cg.funcs[f];
    auto& inArcs = func.inArcs;
    for (size_t a = 0; a < inArcs.size(); a++) {
      Arc* arc = inArcs[a];
      arc->normalizedWeight = arc->weight / func.samples;
    }
  }
}

void print(const std::vector<Cluster*>& clusters) {
  FILE* outfile = fopen("hotfuncs.txt", "wt");
  if (!outfile) error("opening output file hotfuncs.txt");
  uint32_t totalSize = 0;
  uint32_t curPage   = 0;
  uint32_t hotfuncs  = 0;
  HFTRACE(1, "============== page 0 ==============\n");
  for (auto cluster : clusters) {
    HFTRACE(1,
            "-------- density = %.3lf (%u / %u) arcWeight = %.1lf --------\n",
            (double) cluster->samples / cluster->size,
            cluster->samples, cluster->size, cluster->arcWeight);
    for (FuncId fid : cluster->funcs) {
      if (cg.funcs[fid].samples > 0) {
        hotfuncs++;
        int space = 0;
        for (const auto& mangledName : cg.funcs[fid].mangledNames) {
          fprintf(outfile, "%.*s*.text.%s\n",
                  space, " ", mangledName.c_str());
          space = 1;
        }
        HFTRACE(1, "start = %6u : %s\n", totalSize,
                cg.funcs[fid].toString().c_str());
        totalSize += cg.funcs[fid].size;
        uint32_t newPage = totalSize / kPageSize;
        if (newPage != curPage) {
          curPage = newPage;
          HFTRACE(1, "============== page %u ==============\n", curPage);
        }
      }
    }
  }
  fclose(outfile);
  printf("Output saved in hotfuncs.txt\n");
  printf("  Number of hot functions: %u\n  Number of clusters: %lu\n",
         hotfuncs, clusters.size());
}

Algorithm checkAlgorithm(const char* algorithm) {
  auto a = HPHP::toLower(algorithm);

  if (a == "hfsort") return Algorithm::Hfsort;
  if (a == "pettishansen") return Algorithm::PettisHansen;

  return Algorithm::Invalid;
}

} }

int main(int argc, char* argv[]) {
  using namespace HPHP::hfsort;

  char* symbFileName = nullptr;
  char* perfFileName = nullptr;
  char* edgcntFileName = nullptr;

  FILE*  symbFile;
  gzFile perfFile;
  FILE*  edgcntFile;

  Algorithm algorithm = Algorithm::Hfsort;

  // parse commandline arguments

  extern char* optarg;
  extern int optind;
  int c;

  while ((c = getopt(argc, argv, "pe:")) != -1) {
    switch (c) {
      case 'p':
        algorithm = Algorithm::PettisHansen;
        break;
      case 'e':
        edgcntFileName = optarg;
        break;
      case '?':
        error("Unsupported command line argument");
    }
  }

  if ((optind + 2) != argc) {
    error(
      "Usage: hfsort [-p] [-e <EDGCNT_FILE>] <SYMBOL_FILE> <PERF_DATA_FILE>\n"
      "   -p,               use pettis-hansen algorithm for code layout\n"
      "   -e <EDGCNT_FILE>, use edge profile result to build the call graph"
    );
  }

  symbFileName = argv[optind];
  perfFileName = argv[optind + 1];

  if (!(symbFile = fopen(symbFileName, "rt"))) {
    error("Error opening symbol file\n");
  }
  if (!(perfFile = gzopen(perfFileName, "r"))) {
    error("Error opening perf data file\n");
  }
  if (edgcntFileName && !(edgcntFile = fopen(edgcntFileName, "rt"))) {
    error("Error opening edge count file\n");
  }

  readSymbols(symbFile);
  readPerfData(perfFile, (edgcntFileName == nullptr));
  if (edgcntFileName != nullptr) {
    readEdgcntData(edgcntFile);
    fclose(edgcntFile);
  }
  cg.printDot("cg.dot");

  std::vector<Cluster*> clusters;

  if (algorithm == Algorithm::Hfsort) {
    HFTRACE(1, "=== algorithm : hfsort\n\n");
    clusters = clusterize();
  } else {
    HFTRACE(1, "=== algorithm : pettis-hansen\n\n");
    assert(algorithm == Algorithm::PettisHansen);
    clusters = pettisAndHansen();
  }

  sort(clusters.begin(), clusters.end(), compareClustersDensity);
  print(clusters);

  fclose(symbFile);
  gzclose(perfFile);

  return 0;
}
