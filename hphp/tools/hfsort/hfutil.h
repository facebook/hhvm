/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_FACEBOOK_HFSORT_HFUTIL_H
#define incl_HPHP_FACEBOOK_HFSORT_HFUTIL_H

#include <folly/Format.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

namespace HPHP { namespace hfsort {

// The number of pages to reserve for the functions with highest
// density (samples / size).  The functions put in these pages are not
// considered for clustering.
constexpr uint32_t kFrozenPages = 0;

// The minimum approximate probability of a callee being called from a
// particular arc to consider merging with the caller's cluster.
constexpr double kMinArcProbability = 0.1;

// This is a factor to determine by how much a caller cluster is
// willing to degrade it's density by merging a callee.
constexpr int kCallerDegradeFactor = 8;

// Maximum size of a cluster, in bytes.
constexpr uint32_t kMaxClusterSize = 1 << 20;

constexpr uint32_t kPageSize = 2 << 20;

constexpr uint32_t BUFLEN = 1000;

// Change this and recompile to change tracing level.
constexpr uint8_t tracing = 1;

// Supported code layout algorithms

enum class Algorithm { Hfsort, PettisHansen, Invalid };

void trace(const char* fmt, ...);
#define HFTRACE(LEVEL, ...)                  \
  if (tracing >= LEVEL) { trace(__VA_ARGS__); }

typedef   int32_t FuncId;
constexpr int32_t InvalidId = -1;

extern struct CallGraph cg;
struct Cluster;

struct Arc {
  Arc(FuncId s, FuncId d, double w)
      : src(s)
      , dst(d)
      , weight(w) {}
  FuncId src;
  FuncId dst;
  double weight;
  double normalizedWeight{0};
};

struct Func {
  Func(FuncId id, std::string name, uint64_t a, uint32_t s, uint32_t g)
      : id(id)
      , group(g)
      , addr(a)
      , size(s)
      , samples(0) {
    mangledNames.push_back(name);
  }

  bool valid() const {
    return mangledNames.size() > 0;
  }

  std::string toString() const {
    return folly::format("func = {:5} : samples = {:6} : size = {:6} : {}\n",
                         id, samples, size, mangledNames[0]).str();
  }

  FuncId         id;
  uint32_t       group;
  uint64_t       addr;
  uint32_t       size;
  uint32_t       samples;
  std::vector<std::string> mangledNames;
  std::vector<Arc*>   inArcs;
  std::vector<Arc*>   outArcs;
};

struct CallGraph {
  ~CallGraph() { clear(); }
  void clear();

  bool addFunc(Func f) {
    if (f.valid()) {
      auto it = addr2FuncId.find(f.addr);
      if (it != addr2FuncId.end()) {
        Func& base = funcs[it->second];
        base.mangledNames.push_back(f.mangledNames[0]);
        if (f.size > base.size) base.size = f.size;
        HFTRACE(2, "Func: adding '%s' to (%u)\n",
                f.mangledNames[0].c_str(),
                base.id);
      } else {
        funcs.push_back(f);
        addr2FuncId[f.addr] = f.id;
        HFTRACE(2, "Func: adding (%u): %016lx %s %u\n",
                f.id,
                (long)f.addr,
                f.mangledNames[0].c_str(),
                f.size);
        return true;
      }
    }
    return false;
  }

  void addCluster(Cluster* c) {
    clusters.push_back(c);
  }

  Arc* findArc(FuncId f1, FuncId f2) const {
    const auto& outArcs = funcs[f1].outArcs;
    for (size_t i = 0; i < outArcs.size(); i++) {
      Arc* arc = outArcs[i];
      if (arc->dst == f2) return arc;
    }
    return nullptr;
  }

  Arc* incArcWeight(FuncId src, FuncId dst, double w = 1.0) {
    for (size_t i = 0; i < funcs[src].outArcs.size(); i++) {
      Arc* arc = funcs[src].outArcs[i];
      if (arc->dst == dst) {
        arc->weight += w;
        return arc;
      }
    }
    Arc* arc = new Arc(src, dst, w);
    funcs[src].outArcs.push_back(arc);
    funcs[dst].inArcs. push_back(arc);
    arcs.push_back(arc);
    return arc;
  }

  FuncId addrToFuncId(uint64_t addr) const {
    auto it = addr2FuncId.upper_bound(addr);
    if (it == addr2FuncId.begin()) return InvalidId;
    --it;
    const auto &f = funcs[it->second];
    assert(f.addr <= addr);
    if (f.addr + f.size <= addr) {
      return InvalidId;
    }
    return it->second;
  }

  void printDot(char *fileName) const {
    FILE* file = fopen(fileName, "wt");
    if (!file) return;
    fprintf(file, "digraph g {\n");
    for (size_t f = 0; f < funcs.size(); f++) {
      if (funcs[f].samples == 0) continue;
      fprintf(file, "f%lu [label=\"%s\\nsamples=%u\"]\n",
              f, funcs[f].mangledNames[0].c_str(), funcs[f].samples);
    }
    for (size_t f = 0; f < funcs.size(); f++) {
      if (funcs[f].samples == 0) continue;
      for (size_t i = 0; i < funcs[f].outArcs.size(); i++) {
        fprintf(file, "f%lu -> f%u [label=%.3lf]\n", f,
                funcs[f].outArcs[i]->dst,
                funcs[f].outArcs[i]->normalizedWeight);
      }
    }
    fprintf(file, "}\n");
    fclose(file);
  }

  std::vector<Func> funcs;
  std::map<uint64_t,FuncId> addr2FuncId;
  std::vector<Arc*> arcs;
 private:
  std::vector<Cluster*> clusters;
};

struct Cluster {
  explicit Cluster(FuncId fid) {
    funcs.push_back(fid);
    size    = cg.funcs[fid].size;
    arcWeight = 0;
    samples = cg.funcs[fid].samples;
    frozen  = false;
    HFTRACE(1, "new Cluster: %s: %s\n", toString().c_str(),
            cg.funcs[fid].toString().c_str());
  }

  void merge(const Cluster& other, const double aw = 0) {
    for (size_t if2 = 0; if2 < other.funcs.size(); if2++) {
      FuncId fid = other.funcs[if2];
      funcs.push_back(fid);
    }
    size    += other.size;
    samples += other.samples;
    arcWeight += (other.arcWeight + aw);
  }

  std::string toString() const {
    return folly::format("funcs = [{}]", folly::join(", ", funcs)).str();
  }

  std::vector<FuncId> funcs;
  uint32_t       samples;
  double         arcWeight; // intra-cluster callgraph arc weight
  uint32_t       size;
  bool           frozen; // not a candidate for merging
};

struct ClusterArc {
  ClusterArc(Cluster* c1, Cluster* c2, double w)
    : c1(c1)
    , c2(c2)
    , weight(w) {}

  friend bool operator==(const ClusterArc& carc1, const ClusterArc& carc2) {
    return ((carc1.c1 == carc2.c1 && carc1.c2 == carc2.c2) ||
            (carc1.c1 == carc2.c2 && carc1.c2 == carc2.c1));
  }

  friend bool operator!=(const ClusterArc& c1, const ClusterArc& c2) {
    return !(c1 == c2);
  }

  Cluster* c1;
  Cluster* c2;
  double   weight;
};

void error(const char*);
bool compareClustersDensity(const Cluster* c1, const Cluster* c2);
std::vector<Cluster*> clusterize();
std::vector<Cluster*> pettisAndHansen();

} }

#endif
