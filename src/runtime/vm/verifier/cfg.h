/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_VM_VERIFIER_CFG_H_
#define incl_VM_VERIFIER_CFG_H_

#include "runtime/vm/repo.h"
#include "runtime/vm/verifier/util.h"
#include "util/arena.h"

namespace HPHP {
namespace VM {
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
class Block {
public:
  explicit Block(PC start) :
      start(start), last(0), end(0) , id(-1), rpo_id(-1), next_linear(0),
      next_rpo(0), succs(0), exns(0) {
  }
private:
  // Never copy Blocks.
  explicit Block(const Block&);
  Block& operator=(const Block&);
public:
  PC start;           // first instruction
  PC last;            // last instruction (inclusive)
  PC end;             // points past last instruction (exclusive)
  int id;             // plain block id (creation order)
  int rpo_id;         // reverse postorder number. 0 = entry.
  Block* next_linear; // next block in linear order
  Block* next_rpo;    // next block in reverse postorder
  Block** succs;      // array of succesors (can have nulls)
  Block** exns;       // array of exception edges (can have nulls)
};

/**
 * Graph is a control-flow-graph container of Blocks.  The idea is
 * to Arena-allocate these (and Blocks) and only store pass-agnostic
 * stuff about the function.
 *
 * Iteration over blocks can be done directly if necessary but when
 * you can, use a predefined Range, like LinearBlocks or RpoBlocks.
 */
class Graph {
public:
  Graph() : first_linear(0), first_rpo(0), entries(0), block_count(0),
      exn_cap(0) {
  }
private:
  // Never copy Graphs.
  explicit Graph(const Graph&);
  Graph& operator=(const Graph&);
public:
  Block* first_linear; // blocks in linear order
  Block* first_rpo;    // blocks in reverse postorder
  Block** entries; // entry points indexed by arg count [0:param_count]
  int param_count;
  int block_count;
  int exn_cap; // capacity of exns array in each Block
};

inline bool isTF(PC pc) {
  return (instrFlags(*pc) & TF) != 0;
}

inline bool isCF(PC pc) {
  // exclude call-like opcodes marked with CF flag.
  return instrIsControlFlow(*pc) &&
         (isSwitch(*pc) ||
          instrJumpOffset(const_cast<Opcode*>(pc)) != NULL);
}

inline bool isFF(PC pc) {
  return instrReadsCurrentFpi(*pc);
}

inline bool isRet(PC pc) {
  return isTF(pc) && Op(*pc) >= OpRetC && Op(*pc) <= OpRetV;
}

inline bool isIter(PC pc) {
  return Op(*pc) >= OpIterInit && Op(*pc) <= OpIterFree;
}

inline int getImmIva(PC pc) {
  return getImm((Opcode*)pc, 0).u_IVA;
}

inline int numSuccBlocks(const Block* b) {
  return numSuccs(b->last);
}

/**
 * A GraphBuilder holds the temporary state required for building
 * a graph and is typically not needed once a Graph is built.
 */
class GraphBuilder {
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
  Offset offset(PC addr) { return m_unit->offsetOf(addr); }
  void addEdge(Block* from, EdgeKind k, Block* target);
  Block** succs(Block* b);
  Block** exns(Block* b);
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
class LinearBlocks {
 public:
  LinearBlocks(Block* first, Block* end) : b(first), end(end) {
  }
  bool empty() const { return b == end; }
  Block* front() const { ASSERT(!empty()); return b; }
  Block* popFront() { Block* f = front(); b = b->next_linear; return f; }
 private:
  Block *b;   // The current block.
  Block *end; // End marker; empty when b==end.
};

/**
 * Range over the non-null Block* pointers in array.
 */
class BlockPtrRange {
 public:
  BlockPtrRange(Block** a, int cap) : i(a), end(&a[cap]) {
    trimFront();
    trimBack();
  }
  bool empty() const { return i >= end; }
  Block* front() const { ASSERT(!empty()); return i[0]; }
  Block* back()  const { ASSERT(!empty()); return end[-1]; }
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
class InstrRange {
 public:
  InstrRange(PC pc, PC end) : pc(pc), end(end) {
  }
  bool empty() const {
    return pc >= end;
  }
  PC front() const {
    ASSERT(!empty());
    return pc;
  }
  PC popFront() {
    PC i = front();
    pc += instrLen((Opcode*)i);
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

inline BlockPtrRange exnBlocks(const Graph* g, const Block* b) {
  return b->exns ? BlockPtrRange(b->exns, g->exn_cap) :
         BlockPtrRange(0, 0);
}

inline BlockPtrRange entryBlocks(const Graph* g) {
  return BlockPtrRange(g->entries, g->param_count + 1);
}

inline LinearBlocks linearBlocks(const Graph* g) {
  return LinearBlocks(g->first_linear, 0);
}

typedef std::pair<Id, Offset> CatchEnt;

// A callsite starts with FPush*, has 0 or more FPass*, and ends with FCall*.
// The FPI Region protects the range of instructions that execute with the
// partial activation on the stack, which is the instruction after FPush*
// up to and including FCall*.  FPush* is not in the protected region.

inline Offset fpiBase(const FPIEnt& fpi, PC bc) {
  PC fpush = bc + fpi.m_fpushOff;
  return fpush + instrLen((Opcode*)fpush) - bc;
}

inline Offset fpiPast(const FPIEnt& fpi, PC bc) {
  PC fcall = bc + fpi.m_fcallOff;
  return fcall + instrLen((Opcode*)fcall) - bc;
}

}}} // HPHP::VM::Verifier

#endif // incl_VM_VERIFIER_CFG_H_
