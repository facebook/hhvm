/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/trans-cfg.h"

namespace HPHP {
namespace JIT {

static const Trace::Module TRACEMOD = Trace::pgo;

static TransIDSet findPredTrans(const SrcRec* sr,
                                const TcaTransIDMap& jmpToTransID) {
  assert(sr);
  TransIDSet predSet;

  for (auto inBr : sr->incomingBranches()) {
    TransID srcId = mapGet(jmpToTransID, inBr.toSmash(), InvalidID);
    FTRACE(5, "findPredTrans: toSmash = {}   srcId = {}\n",
           inBr.toSmash(), srcId);
    if (srcId != InvalidID) {
      predSet.insert(srcId);
    }
  }

  return predSet;
}

/**
 * This function tries to infer the weight of any arc in the arcVec given the
 * weights of other arcs in the list and totalWeight, which is the
 * known sum of all their weights.
 * Returns whether or not the weight of any arc was inferred and, in case of
 * success, the weight of such arc is updated.
 */
static bool inferredArcWeight(const TransCFG::ArcPtrVec& arcVec,
                              int64_t totalWeight) {
  int64_t arcWeight = totalWeight;
  TransCFG::Arc* unknownArc = nullptr;
  for (auto arc : arcVec) {
    if (arc->weight() == TransCFG::Arc::kUnknownWeight) {
      if (unknownArc != nullptr) {
        // More than one arc with unknown weight, so can't infer
        return false;
      }
      unknownArc = arc;
    } else {
      arcWeight -= arc->weight();
    }
  }
  if (unknownArc == nullptr) return false;
  // Avoid creating negative-weight arcs. Node weights are not required to be
  // accurate and, since arc weights are derived from nodes' weights, they
  // aren't accurate either.  This can result in arcWeight to be negative here.
  if (arcWeight < 0) arcWeight = 0;
  unknownArc->setWeight(arcWeight);
  return true;
}

TransCFG::TransCFG(FuncId funcId,
                   const ProfData* profData,
                   const SrcDB& srcDB,
                   const TcaTransIDMap& jmpToTransID) {
  assert(profData);

  // add nodes
  for (TransID tid = 0; tid < profData->numTrans(); tid++) {
    if (profData->transKind(tid) == TransProfile &&
        profData->transBlock(tid) != nullptr &&
        profData->transFuncId(tid) == funcId) {
      int64_t counter = profData->transCounter(tid);
      int64_t weight  = RuntimeOption::EvalJitPGOThreshold - counter;
      addNode(tid, weight);
    }
  }

  // add arcs
  for (TransID dstId : nodes()) {
    SrcKey dstSK = profData->transSrcKey(dstId);
    const SrcRec* dstSR = srcDB.find(dstSK);
    FTRACE(5, "TransCFG: adding incoming arcs in dstId = {}\n", dstId);
    TransIDSet predIDs = findPredTrans(dstSR, jmpToTransID);
    for (auto predId : predIDs) {
      if (hasNode(predId)) {
        FTRACE(5, "TransCFG: adding arc {} -> {}\n", predId, dstId);
        addArc(predId, dstId, TransCFG::Arc::kUnknownWeight);
      }
    }
  }

  // infer arc weights
  bool changed;
  do {
    changed = false;
    for (TransID tid : nodes()) {
      int64_t nodeWeight = weight(tid);
      if (inferredArcWeight(inArcs(tid),  nodeWeight)) changed = true;
      if (inferredArcWeight(outArcs(tid), nodeWeight)) changed = true;
    }
  } while (changed);

  // guess weight or non-inferred arcs
  for (TransID tid : nodes()) {
    for (auto arc : outArcs(tid)) {
      if (arc->weight() == Arc::kUnknownWeight) {
        arc->setGuessed();
        int64_t arcWgt = std::min(weight(arc->src()), weight(arc->dst())) / 2;
        arc->setWeight(arcWgt);
      }
    }
  }
}

int64_t TransCFG::weight(TransID id) const {
  assert(hasNode(id));
  size_t idx = mapGet(m_idToIdx, id);
  return m_nodeInfo[idx].weight();
}

const TransCFG::ArcPtrVec& TransCFG::inArcs(TransID id) const {
  assert(hasNode(id));
  size_t idx = mapGet(m_idToIdx, id);
  return m_nodeInfo[idx].inArcs();
}

const TransCFG::ArcPtrVec& TransCFG::outArcs(TransID id) const {
  assert(hasNode(id));
  size_t idx = mapGet(m_idToIdx, id);
  return m_nodeInfo[idx].outArcs();
}

TransCFG::Node::~Node() {
  for (auto arc : m_outArcs) {
    delete arc;
  }
}

void TransCFG::addNode(TransID id, int64_t weight) {
  size_t idx = m_transIds.size();
  m_transIds.push_back(id);
  m_idToIdx[id] = idx;
  m_nodeInfo.push_back(Node(id, weight));
}

bool TransCFG::hasNode(TransID id) const {
  return m_idToIdx.find(id) != m_idToIdx.end();
}

void TransCFG::addArc(TransID srcId, TransID dstId, int64_t weight) {
  assert(hasNode(srcId));
  assert(hasNode(dstId));
  size_t srcIdx = m_idToIdx[srcId];
  size_t dstIdx = m_idToIdx[dstId];
  Arc* arc = new Arc(srcId, dstId, weight);
  m_nodeInfo[srcIdx].addOutArc(arc);
  m_nodeInfo[dstIdx].addInArc(arc);
}

void TransCFG::print(std::string fileName, const ProfData* profData,
                     const TransIDSet* selected) const {
  FILE* file = fopen(fileName.c_str(), "wt");
  if (!file) return;

  fprintf(file, "digraph CFG {\n");

  // find max node weight
  int64_t maxWeight = 1; // 1 to avoid div by 0
  for (auto tid : nodes()) {
    auto w = weight(tid);
    if (w > maxWeight) maxWeight = w;
  }

  // print nodes
  for (auto tid : nodes()) {
    int64_t w = weight(tid);
    uint32_t coldness  = 255 - (255 * w / maxWeight);
    Offset bcStart = profData->transStartBcOff(tid);
    Offset bcStop  = profData->transStopBcOff(tid);
    const char* shape = selected && setContains(*selected, tid) ? "oval"
                                                                : "box";
    fprintf(file,
            "t%u [shape=%s,label=\"T: %u\\np: %" PRId64 "\\nbc: [0x%x-0x%x)\","
            "style=filled,fillcolor=\"#ff%02x%02x\"];\n", tid, shape, tid, w,
            bcStart, bcStop, coldness, coldness);
  }

  // print arcs
  for (auto srcId : nodes()) {
    for (auto arc : outArcs(srcId)) {
      int64_t w = arc->weight();
      fprintf(file, "t%u -> t%u [color=\"%s\",label=\"%" PRId64 "\"] ;\n",
              srcId,
              arc->dst(),
              arc->guessed() ? "red" : "green4",
              w);
    }
  }

  fprintf(file, "}\n");
  fclose(file);
}

} }
