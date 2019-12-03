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

#ifndef incl_HPHP_UTIL_HFSORT_H
#define incl_HPHP_UTIL_HFSORT_H

#include <string>
#include <unordered_set>
#include <vector>

#include "hphp/util/hash.h"

namespace HPHP { namespace hfsort {

using TargetId = int32_t;
constexpr TargetId InvalidId = -1;

struct Arc {
  Arc(TargetId s, TargetId d, double w = 0)
      : src(s)
      , dst(d)
      , weight(w)
  {}
  Arc(const Arc&) = delete;

  friend bool operator==(const Arc& lhs, const Arc& rhs) {
    return lhs.src == rhs.src && lhs.dst == rhs.dst;
  }

  const TargetId src;
  const TargetId dst;
  mutable double weight;
  mutable double normalizedWeight{0};
  mutable double avgCallOffset{0};
};

struct ArcHash {
  int64_t operator()(const Arc& arc) const {
    return hash_int64_pair(int64_t(arc.src), int64_t(arc.dst));
  }
};

struct Target {
  explicit Target(uint32_t size, uint64_t samples = 0)
    : size(size)
    , samples(samples)
  {}

  uint32_t size;
  uint64_t samples;

  // preds and succs contain no duplicate elements and self arcs are not allowed
  std::vector<TargetId> preds;
  std::vector<TargetId> succs;
};

struct TargetGraph {
  TargetId addTarget(uint32_t size, uint64_t samples = 0);
  void setSamples(TargetId id, uint64_t samples);
  uint64_t getSamples(TargetId id);
  const Arc& incArcWeight(TargetId src, TargetId dst, double w = 1.0);
  void normalizeArcWeights();

  template<class L>
  void printDot(char* fileName, L getLabel) const;

  std::vector<Target> targets;
  std::unordered_set<Arc, ArcHash> arcs;
};

struct Cluster {
  Cluster(TargetId id, const Target& f);

  std::string toString() const;
  double density() const;

  std::vector<TargetId> targets;
  uint32_t samples;
  uint32_t size;
  bool frozen; // not a candidate for merging
};

/////////////////////////////////////////////////////////////////////////

bool compareClustersDensity(const Cluster& c1, const Cluster& c2);
std::vector<Cluster> clusterize(const TargetGraph& cg);

/*
 * HFSortPlus - layout of hot functions with iTLB cache optimization
 */
std::vector<Cluster> hfsortPlus(const TargetGraph& cg);

/*
 * Pettis-Hansen code layout algorithm
 * reference: K. Pettis and R. C. Hansen, "Profile Guided Code Positioning",
 *            PLDI '90
 */
std::vector<Cluster> pettisAndHansen(const TargetGraph& cg);

/////////////////////////////////////////////////////////////////////////

template<class L>
void TargetGraph::printDot(char* fileName, L getLabel) const {
  FILE* file = fopen(fileName, "wt");
  if (!file) return;

  fprintf(file, "digraph g {\n");
  for (size_t f = 0; f < targets.size(); f++) {
    if (targets[f].samples == 0) continue;
    fprintf(
      file,
      "f%lu [label=\"%s\\nsamples=%lu\\nsize=%u\"];\n",
      f,
      getLabel(f),
      targets[f].samples,
      targets[f].size);
  }
  for (size_t f = 0; f < targets.size(); f++) {
    if (targets[f].samples == 0) continue;
    for (auto dst : targets[f].succs) {
      auto& arc = *arcs.find(Arc(f, dst));
      fprintf(
        file,
        "f%lu -> f%u [label=\"normWgt=%.3lf,weight=%.0lf,callOffset=%.1lf\"];"
        "\n",
        f,
        dst,
        arc.normalizedWeight,
        arc.weight,
        arc.avgCallOffset);
    }
  }
  fprintf(file, "}\n");
  fclose(file);
}

}}

#endif
