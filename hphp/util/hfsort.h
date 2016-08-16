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
  explicit Target(uint32_t size, uint32_t samples = 0)
    : size(size)
    , samples(samples)
  {}

  uint32_t size;
  uint32_t samples;

  // preds and succs contain no duplicate elements and self arcs are not allowed
  std::vector<TargetId> preds;
  std::vector<TargetId> succs;
};

struct TargetGraph {
  TargetId addTarget(uint32_t size, uint32_t samples = 0);
  const Arc& incArcWeight(TargetId src, TargetId dst, double w = 1.0);

  std::vector<Target> targets;
  std::unordered_set<Arc, ArcHash> arcs;
};

struct Cluster {
  Cluster(TargetId id, const Target& f);

  std::string toString() const;

  std::vector<TargetId> targets;
  uint32_t samples;
  double arcWeight; // intra-cluster callgraph arc weight
  uint32_t size;
  bool frozen; // not a candidate for merging
};

bool compareClustersDensity(const Cluster& c1, const Cluster& c2);
std::vector<Cluster> clusterize(const TargetGraph& cg);

/*
 * Pettis-Hansen code layout algorithm
 * reference: K. Pettis and R. C. Hansen, "Profile Guided Code Positioning",
 *            PLDI '90
 */
std::vector<Cluster> pettisAndHansen(const TargetGraph& cg);

}}

#endif
