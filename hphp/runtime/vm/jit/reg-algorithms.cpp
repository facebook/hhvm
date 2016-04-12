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

#include "hphp/runtime/vm/jit/reg-algorithms.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

namespace HPHP { namespace jit {

struct CycleInfo {
  PhysReg node;
  int length;
};

bool cycleHasSIMDReg(const CycleInfo& cycle, MovePlan& moves) {
  auto& first = cycle.node;
  auto node = first;
  do {
    if (node.isSIMD()) return true;
    node = moves[node];
  } while (node != first);
  return false;
}

jit::vector<VMoveInfo>
doVregMoves(Vunit& unit, MovePlan& moves) {
  constexpr auto N = 64;
  assertx(abi().all().size() <= N);
  jit::vector<VMoveInfo> howTo;
  CycleInfo cycles[N];
  size_t num_cycles = 0;
  PhysReg::Map<int> outDegree;
  PhysReg::Map<int> index;

  for (auto reg : moves) {
    // Ignore moves from a register to itself
    if (reg == moves[reg]) moves[reg] = InvalidReg;
    index[reg] = -1;
  }

  // Iterate over the nodes filling in outDegree[] and cycles[] as we go
  int nextIndex = 0;
  for (auto reg : moves) {
    // skip registers we've visited already.
    if (index[reg] >= 0) continue;

    // Begin walking a path from reg.
    for (auto node = reg;;) {
      assertx(nextIndex < N);
      index[node] = nextIndex++;
      auto next = moves[node];
      if (next != InvalidReg) {
        ++outDegree[next];
        if (index[next] < 0) {
          // There is an edge from node to next, and next has not been
          // visited.  Extend current path to include next, then loop.
          node = next;
          continue;
        }
        // next already visited; check if next is on current path.
        if (index[next] >= index[reg]) {
          // found a cycle.
          assert(num_cycles < N);
          cycles[num_cycles++] = { next, nextIndex - index[next] };
        }
      }
      break;
    }
  }

  // Handle all moves that aren't part of a cycle. Only nodes with outdegree
  // zero are put into the queue, which is how nodes in a cycle get excluded.
  {
    PhysReg q[N];
    int qBack = 0;
    auto enque = [&](PhysReg r) { assertx(qBack < N); q[qBack++] = r; };
    for (auto node : outDegree) {
      if (outDegree[node] == 0) enque(node);
    }
    for (int i = 0; i < qBack; ++i) {
      auto node = q[i];
      if (moves[node] == InvalidReg) continue;
      auto nextNode = moves[node];
      howTo.push_back({VMoveInfo::Kind::Move, nextNode, node});
      --outDegree[nextNode];
      if (outDegree[nextNode] == 0) enque(nextNode);
    }
  }

  // Deal with any cycles we encountered
  for (size_t i = 0; i < num_cycles; ++i) {
    auto const& cycle = cycles[i];
    // can't use xchg if one of the registers is SIMD
    bool hasSIMDReg = cycleHasSIMDReg(cycle, moves);
    if (cycle.length == 2 && !hasSIMDReg) {
      auto v = cycle.node;
      auto w = moves[v];
      howTo.push_back({VMoveInfo::Kind::Xchg, w, v});
    } else if (cycle.length == 3 && !hasSIMDReg) {
      auto v = cycle.node;
      auto w = moves[v];
      howTo.push_back({VMoveInfo::Kind::Xchg, w, v});
      auto x = moves[w];
      howTo.push_back({VMoveInfo::Kind::Xchg, x, w});
    } else {
      auto t = unit.makeReg();
      auto v = cycle.node;
      howTo.push_back({VMoveInfo::Kind::Move, v, t});
      auto w = v;
      auto x = moves[w];
      while (x != v) {
        howTo.push_back({VMoveInfo::Kind::Move, x, w});
        w = x;
        x = moves[w];
      }
      howTo.push_back({VMoveInfo::Kind::Move, t, w});
    }
  }
  return howTo;
}

jit::vector<MoveInfo> doRegMoves(MovePlan& moves, PhysReg rTmp) {
  constexpr auto N = 64;
  assertx(abi().all().size() <= N);
  jit::vector<MoveInfo> howTo;
  CycleInfo cycles[N];
  size_t num_cycles = 0;
  PhysReg::Map<int> outDegree;
  PhysReg::Map<int> index;

  assertx(moves[rTmp] == InvalidReg);
  for (auto reg : moves) {
    // Ignore moves from a register to itself
    if (reg == moves[reg]) moves[reg] = InvalidReg;
    index[reg] = -1;
  }

  // Iterate over the nodes filling in outDegree[] and cycles[] as we go
  int nextIndex = 0;
  for (auto reg : moves) {
    // skip registers we've visited already.
    if (index[reg] >= 0) continue;

    // Begin walking a path from reg.
    for (auto node = reg;;) {
      assertx(nextIndex < N);
      index[node] = nextIndex++;
      auto next = moves[node];
      if (next != InvalidReg) {
        ++outDegree[next];
        if (index[next] < 0) {
          // There is an edge from node to next, and next has not been
          // visited.  Extend current path to include next, then loop.
          node = next;
          continue;
        }
        // next already visited; check if next is on current path.
        if (index[next] >= index[reg]) {
          // found a cycle.
          assert(num_cycles < N);
          cycles[num_cycles++] = { next, nextIndex - index[next] };
        }
      }
      break;
    }
  }

  // Handle all moves that aren't part of a cycle. Only nodes with outdegree
  // zero are put into the queue, which is how nodes in a cycle get excluded.
  {
    PhysReg q[N];
    int qBack = 0;
    auto enque = [&](PhysReg r) { assertx(qBack < N); q[qBack++] = r; };
    for (auto node : outDegree) {
      if (outDegree[node] == 0) enque(node);
    }
    for (int i = 0; i < qBack; ++i) {
      auto node = q[i];
      if (moves[node] == InvalidReg) continue;
      auto nextNode = moves[node];
      howTo.push_back({MoveInfo::Kind::Move, nextNode, node});
      --outDegree[nextNode];
      if (outDegree[nextNode] == 0) enque(nextNode);
    }
  }

  // Deal with any cycles we encountered
  for (size_t i = 0; i < num_cycles; ++i) {
    auto const& cycle = cycles[i];
    // can't use xchg if one of the registers is SIMD
    bool hasSIMDReg = cycleHasSIMDReg(cycle, moves);
    if (cycle.length == 2 && !hasSIMDReg) {
      auto v = cycle.node;
      auto w = moves[v];
      howTo.push_back({MoveInfo::Kind::Xchg, w, v});
    } else if (cycle.length == 3 && !hasSIMDReg) {
      auto v = cycle.node;
      auto w = moves[v];
      howTo.push_back({MoveInfo::Kind::Xchg, w, v});
      auto x = moves[w];
      howTo.push_back({MoveInfo::Kind::Xchg, x, w});
    } else {
      auto v = cycle.node;
      howTo.push_back({MoveInfo::Kind::Move, v, rTmp});
      auto w = v;
      auto x = moves[w];
      while (x != v) {
        howTo.push_back({MoveInfo::Kind::Move, x, w});
        w = x;
        x = moves[w];
      }
      howTo.push_back({MoveInfo::Kind::Move, rTmp, w});
    }
  }
  return howTo;
}

}}
