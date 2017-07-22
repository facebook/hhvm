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

#ifndef incl_HPHP_VM_VERIFIER_CFG_H_
#define incl_HPHP_VM_VERIFIER_CFG_H_

#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/verifier/util.h"
#include "hphp/util/arena.h"

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

inline bool isFF(PC pc) {
  return instrReadsCurrentFpi(peek_op(pc));
}

inline bool isRet(PC pc) {
  auto const op = peek_op(pc);
  return op == Op::RetC || op == Op::RetV;
}

// Return true if pc points to an Iter instruction whose first immedate
// argument is an iterator id.
inline bool isIter(PC pc) {
  // IterBreak is not included, because it has a variable-length list of
  // iterartor ids, rather than a single iterator id.
  switch (peek_op(pc)) {
  case Op::IterInit:
  case Op::MIterInit:
  case Op::WIterInit:
  case Op::IterInitK:
  case Op::MIterInitK:
  case Op::WIterInitK:
  case Op::IterNext:
  case Op::MIterNext:
  case Op::WIterNext:
  case Op::IterNextK:
  case Op::MIterNextK:
  case Op::WIterNextK:
  case Op::DecodeCufIter:
  case Op::IterFree:
  case Op::MIterFree:
  case Op::CIterFree:
    return true;
  default:
    break;
  }
  return false;
}

inline int getImmIva(PC pc) {
  return getImm(pc, 0).u_IVA;
}

inline int numSuccBlocks(const Block* b) {
  // Fault handlers are a special case with 1 edge (to the parent)
  return peek_op(b->last) == Op::Unwind ? 1 : numSuccs(b->last);
}

/**
 * A GraphBuilder holds the temporary state required for building
 * a graph and is typically not needed once a Graph is built.
 */
struct GraphBuilder {
private:
  typedef hphp_hash_map<PC, Block*> BlockMap;
  enum EdgeKind { FallThrough, Taken };
 public:
  GraphBuilder(Arena& arena, const Func* func)
    : m_arena(arena), m_func(func),
      m_unit(func->unit()), m_graph(0) {
  }
  Graph* build();
  Block* at(Offset off) { return at(m_unit->at(off)); }
 private:
  void createBlocks();
  void createExBlocks();
  void linkBlocks();
  void linkExBlocks();
  Block* createBlock(PC pc);
  Block* createBlock(Offset off) { return createBlock(m_unit->at(off)); }
  Block* at(PC addr);
  Offset offset(PC addr) const {
    return m_unit->offsetOf(addr);
  }
  Block** succs(Block* b);
 private:
  BlockMap m_blocks;
  Arena& m_arena;
  const Func* const m_func;
  const Unit* const m_unit;
  Graph* m_graph;
};

/**
 * Range over blocks in linear order
 */
struct LinearBlocks {
  LinearBlocks(Block* first, Block* end) : b(first), end(end) {
  }
  bool empty() const { return b == end; }
  Block* front() const { assert(!empty()); return b; }
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
  Block* front() const { assert(!empty()); return i[0]; }
  Block* back()  const { assert(!empty()); return end[-1]; }
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
    assert(!empty());
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

inline InstrRange funcInstrs(const Func* func) {
  return InstrRange(func->unit()->at(func->base()),
                    func->unit()->at(func->past()));
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

// A callsite starts with FPush*, has 0 or more FPass*, and usually
// ends with FCall* (If there is a terminal making the FCall*
// unreachable, the fpi region will end there). The FPI Region
// protects the range of instructions that execute with the partial
// activation on the stack, which is the instruction after FPush* up
// to and including FCall*.  FPush* is not in the protected region.

inline Offset fpiBase(const FPIEnt& fpi, PC bc) {
  PC fpush = bc + fpi.m_fpushOff;
  return fpush + instrLen(fpush) - bc;
}

inline Offset fpiPast(const FPIEnt& fpi, PC bc) {
  PC endFpiOp = bc + fpi.m_fpiEndOff;
  return endFpiOp + instrLen(endFpiOp) - bc;
}

}} // HPHP::Verifier

#endif // incl_HPHP_VM_VERIFIER_CFG_H_
