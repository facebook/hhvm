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

#pragma once

#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

namespace HPHP { namespace jit { namespace layout {
///////////////////////////////////////////////////////////////////////////////

/**
 * This keeps track of the weights of blocks and arcs in a Vunit.
 */
struct Scale {
  explicit Scale(const Vunit& unit);
  explicit Scale(const Vunit& unit, const jit::vector<Vlabel>& blockOrder);

  int64_t weight(Vlabel blk) const;
  int64_t weight(Vlabel src, Vlabel dst) const;
  uint64_t predSize(Vlabel blk) const;
  const jit::vector<Vlabel>& blocks() const;
  std::string toString() const;

 private:
  static const int64_t kUnknownWeight = std::numeric_limits<int64_t>::max();

  void       computeArcWeights();
  TransIDSet findProfTransIDs(Vlabel blk) const;
  int64_t    findProfCount(Vlabel blk)   const;

  static uint64_t arcId(Vlabel src, Vlabel dst) { return (src << 32) + dst; }

  const Vunit&                     m_unit;
  const jit::vector<Vlabel>        m_blocks;
  const PredVector                 m_preds;
  jit::hash_map<uint64_t, int64_t> m_arcWgts; // keyed using arcId()
};

/**
 * This implements a block layout algorithm by Pettis & Hansen
 */
struct Clusterizer {
  Clusterizer(Vunit& unit, const Scale& scale);

  jit::vector<Vlabel> getBlockList() const;

  std::string toString() const;

 private:
  using Cluster = jit::vector<Vlabel>;

  void initClusters();
  void clusterizeGreedy();
  void clusterizeExtTSP();
  void sortClusters();
  void splitHotColdClusters();

  Vunit&                    m_unit;
  const Scale&              m_scale;
  const jit::vector<Vlabel> m_blocks;
  jit::vector<Cluster>      m_clusters;
  jit::vector<Vlabel>       m_blockCluster; // maps block to current cluster
  jit::vector<Vlabel>       m_clusterOrder; // final sorted list of cluster ids
};

///////////////////////////////////////////////////////////////////////////////
}}}
