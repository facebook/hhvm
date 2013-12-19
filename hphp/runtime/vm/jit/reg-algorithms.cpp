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

#include "hphp/runtime/vm/jit/reg-algorithms.h"

namespace HPHP { namespace JIT {


bool cycleHasSIMDReg(const CycleInfo& cycle,
                     PhysReg::Map<PhysReg>& moves) {
  auto& first = cycle.node;
  auto node = first;
  do {
    if (node.isSIMD()) return true;
    node = moves[node];
  } while (node != first);
  return false;
}

smart::vector<MoveInfo> doRegMoves(PhysReg::Map<PhysReg>& moves,
                                   PhysReg rTmp) {
  smart::vector<MoveInfo> howTo;
  smart::vector<CycleInfo> cycles;
  PhysReg::Map<int> outDegree;
  RegSet inCycle;

  for (auto reg : moves) {
    if (reg == moves[reg]) {
      // Get rid of 1-cycles.
      moves[reg] = InvalidReg;
    }
    if (reg == rTmp && moves[reg] != InvalidReg) {
      // ERROR: rTmp cannot be referenced in moves[].
      assert(false);
    }
  }

  // Iterate over the nodes filling in outDegree[] and cycles[] as we go
  for (auto reg : moves) {
    if (moves[reg] != InvalidReg) {
      ++outDegree[moves[reg]];
    }

    if (inCycle.contains(reg)) {
      // We've already found this node in a cycle.
      continue;
    }

    auto node = reg;
    smart::vector<PhysReg> path;

    // Follow the path from this node. If it's in a cycle, we will eventually
    // find this node again.
    do {
      path.push_back(node);
      node = moves[node];
    } while (node != InvalidReg && node != reg);

    if (node != InvalidReg) {
      // Found a cycle.
      for (auto const& r : path) {
        inCycle.add(r);
      }
      cycles.push_back({node, (int)path.size()});
    }
  }

  // Handle all moves that aren't part of a cycle. Only nodes with outdegree
  // zero are put into the queue, which is how nodes in a cycle get excluded.
  {
    smart::deque<PhysReg> q;
    for (auto node : outDegree) {
      if (outDegree[node] == 0) {
        q.push_back(node);
      }
    }
    while (!q.empty()) {
      auto node = q.front();
      q.pop_front();

      if (moves[node] != InvalidReg) {
        auto nextNode = moves[node];
        howTo.emplace_back(MoveInfo::Kind::Move, nextNode, node);
        --outDegree[nextNode];
        if (outDegree[nextNode] == 0) {
          q.push_back(nextNode);
        }
      }
    }
  }

  // Deal with any cycles we encountered
  for (auto const& cycle : cycles) {
    // can't use xchg if one of the registers is SIMD
    bool hasSIMDReg = cycleHasSIMDReg(cycle, moves);
    if (cycle.length == 2 && !hasSIMDReg) {
      auto v = cycle.node;
      auto w = moves[v];
      howTo.push_back(MoveInfo(MoveInfo::Kind::Xchg, w, v));
    } else if (cycle.length == 3 && !hasSIMDReg) {
      auto v = cycle.node;
      auto w = moves[v];
      howTo.push_back(MoveInfo(MoveInfo::Kind::Xchg, w, v));
      auto x = moves[w];
      howTo.push_back(MoveInfo(MoveInfo::Kind::Xchg, x, w));
    } else {
      auto v = cycle.node;
      howTo.push_back(MoveInfo(MoveInfo::Kind::Move, v, rTmp));
      auto w = v;
      auto x = moves[w];
      while (x != v) {
        howTo.push_back(MoveInfo(MoveInfo::Kind::Move, x, w));
        w = x;
        x = moves[w];
      }
      howTo.push_back(MoveInfo(MoveInfo::Kind::Move, rTmp, w));
    }
  }
  return howTo;
}

}}
