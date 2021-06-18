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

#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/verifier/util.h"
#include "hphp/util/arena.h"
#include <boost/dynamic_bitset.hpp>

namespace HPHP {
namespace Verifier {

/**
 * Block is a standard basic block.  The # of ordinary successors
 * is determined by the last instruction and the # of exception
 * edges is determined by the Graph that owns this Block.
 *
 * id and rpo_id are the block id and reverse-postorder-number,
 * respectively.  Both start from 0.  Pass-specific data about
 * blocks should be stored elsewhere and indexed by one of these ids,
 * or in a Map<Block*> if you're a sadist.
 */
struct Block {
  explicit Block(PC start) :
      start(start), last(0), end(0) , id(-1), rpo_id(-1), next_linear(0),
      next_rpo(0), succs(0), exn(0) {
  }

  // Never copy Blocks.
  explicit Block(const Block&) = delete;
  Block& operator=(const Block&) = delete;

  static bool reachable(Block* from, Block* to,
                        boost::dynamic_bitset<>& visited);

  PC start;           // first instruction
  PC last;            // last instruction (inclusive)
  PC end;             // points past last instruction (exclusive)
  int id;             // plain block id (creation order)
  int rpo_id;         // reverse postorder number. 0 = entry.
  Block* next_linear; // next block in linear order
  Block* next_rpo;    // next block in reverse postorder
  Block** succs;      // array of succesors (can have nulls)
  Block* exn;         // exception edge (can be null)
};

/**
 * Graph is a control-flow-graph container of Blocks.  The idea is
 * to Arena-allocate these (and Blocks) and only store pass-agnostic
 * stuff about the function.
 *
 * Iteration over blocks can be done directly if necessary but when
 * you can, use a predefined Range, like LinearBlocks or RpoBlocks.
 */
struct Graph {
  Graph() : first_linear(0), first_rpo(0), entries(0), block_count(0) {
  }

  explicit Graph(const Graph&) = delete;
  Graph& operator=(const Graph&) = delete;

  Block* first_linear; // blocks in linear order
  Block* first_rpo;    // blocks in reverse postorder
  Block** entries; // entry points indexed by arg count [0:param_count]
  int param_count;
  int block_count;
};

inline bool isTF(PC pc) {
  return (instrFlags(peek_op(pc)) & TF) != 0;
}

inline bool isCF(PC pc) {
  return instrIsNonCallControlFlow(peek_op(pc));
}

inline bool isRet(PC pc) {
  auto const op = peek_op(pc);
  return op == Op::RetC || op == Op::RetCSuspended || op == Op::RetM;
}

// Return true if pc points to an Iter instruction which takes either an
// iterator ID or an IterArgs struct as its first immediate.
inline bool isIter(PC pc) {
  switch (peek_op(pc)) {
  case Op::IterInit:
  case Op::LIterInit:
  case Op::IterNext:
  case Op::LIterNext:
  case Op::IterFree:
  case Op::LIterFree:
    return true;
  default:
    break;
  }
  return false;
}

inline int getIterId(PC pc) {
  assertx(isIter(pc));
  auto const op = peek_op(pc);
  auto const im = getImm(pc, 0);
  return op == Op::IterFree || op == Op::LIterFree ? im.u_IA : im.u_ITA.iterId;
}

inline int getImmIva(PC pc) {
  return getImm(pc, 0).u_IVA;
}

inline int numSuccBlocks(const Block* b) {
  return numSuccs(b->last);
}

#define APPLY(d, l, r)                         \
  if (auto left = d.left()) return left->l;    \
  if (auto right = d.right()) return right->r; \
  not_reached()

struct SomeFunc {
  /* implicit */ SomeFunc(const Func* f) : m_func(f) {}
  /* implicit */ SomeFunc(const FuncEmitter* f) : m_func(f) {}

  SomeFunc& operator=(const Func* f) { m_func = f; return *this; }
  SomeFunc& operator=(const FuncEmitter* f) { m_func = f; return *this; }

  Offset past() const            { APPLY(m_func, bclen(), bcPos()); }
  PC entry() const               { APPLY(m_func, entry(), bc()); }
  PC at(Offset o) const          { return entry() + o; }
  Offset offsetOf(PC addr) const { return static_cast<Offset>(addr - entry()); }

  size_t numParams() const { APPLY(m_func, params().size(), params.size()); }
  const Func::ParamInfo& param(size_t idx) const {
    APPLY(m_func, params()[idx], params[idx]);
  }

  size_t numEHEnts() const { APPLY(m_func, ehtab().size(), ehtab.size()); }
  const EHEnt& ehent(size_t i) const { APPLY(m_func, ehtab()[i], ehtab[i]); }

  const EHEnt* findEH(Offset off) const {
    return m_func.match(
      [&] (const Func* f) { return Func::findEH(f->ehtab(), off); },
      [&] (const FuncEmitter* f) { return Func::findEH(f->ehtab, off); }
    );
  }

private:
  Either<const Func*, const FuncEmitter*> m_func;
};

#undef APPLY

/**
 * A GraphBuilder holds the temporary state required for building
 * a graph and is typically not needed once a Graph is built.
 */
struct GraphBuilder {
private:
  typedef hphp_hash_map<PC, Block*> BlockMap;
  enum EdgeKind { FallThrough, Taken };
 public:
  template<class F>
  GraphBuilder(Arena& arena, const F* func)
    : m_arena(arena), m_func(func), m_graph(0) {
  }
  Graph* build();
  Block* at(Offset off) const { return at(m_func.at(off)); }
 private:
  void createBlocks();
  void createExBlocks();
  void linkBlocks();
  void linkExBlocks();
  Block* createBlock(PC pc);
  Block* createBlock(Offset off) { return createBlock(m_func.at(off)); }
  Block* at(PC addr) const;
  Offset offset(PC addr) const {
    return m_func.offsetOf(addr);
  }
  Block** succs(Block* b);
 private:
  BlockMap m_blocks;
  Arena& m_arena;
  const SomeFunc m_func;
  Graph* m_graph;
};

/**
 * Range over blocks in linear order
 */
struct LinearBlocks {
  LinearBlocks(Block* first, Block* end) : b(first), end(end) {
  }
  bool empty() const { return b == end; }
  Block* front() const { assertx(!empty()); return b; }
  Block* popFront() { Block* f = front(); b = b->next_linear; return f; }
 private:
  Block *b;   // The current block.
  Block *end; // End marker; empty when b==end.
};

/**
 * Range over the non-null Block* pointers in array.
 */
struct BlockPtrRange {
  BlockPtrRange(Block** a, int cap) : i(a), end(&a[cap]) {
    trimFront();
    trimBack();
  }
  bool empty() const { return i >= end; }
  Block* front() const { assertx(!empty()); return i[0]; }
  Block* back()  const { assertx(!empty()); return end[-1]; }
  Block* popFront() {
    Block* b = front();
    ++i; trimFront();
    return b;
  }
  Block* popBack() {
    Block* b = back();
    --end; trimBack();
    return b;
  }
 private:
  void trimFront() { while (!empty() && !i[0]) ++i; }
  void trimBack()  { while (!empty() && !end[-1]) --end; }
 private:
  Block **i;    // current
  Block **end;  // points past end (exclusive)
};

/**
 * Iterate through a contiguous range of instructions
 */
struct InstrRange {
  InstrRange(PC pc, PC end) : pc(pc), end(end) {
  }
  bool empty() const {
    return pc >= end;
  }
  PC front() const {
    assertx(!empty());
    return pc;
  }
  PC popFront() {
    PC i = front();
    pc += instrLen(i);
    return i;
  }
 private:
  PC pc, end;
};

/**
 * Sort blocks in reverse postorder and reset each blocks's next_rpo and
 * and rpo_id fields
 */
void sortRpo(Graph* g);

inline InstrRange funcInstrs(SomeFunc func) {
  return InstrRange(func.at(0), func.at(func.past()));
}

inline InstrRange blockInstrs(const Block* b) {
  return InstrRange(b->start, b->end);
}

inline BlockPtrRange succBlocks(const Block* b) {
  return BlockPtrRange(b->succs, numSuccBlocks(b));
}

inline BlockPtrRange entryBlocks(const Graph* g) {
  return BlockPtrRange(g->entries, g->param_count + 1);
}

inline LinearBlocks linearBlocks(const Graph* g) {
  return LinearBlocks(g->first_linear, 0);
}

}} // HPHP::Verifier
