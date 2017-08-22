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

#include "hphp/runtime/vm/verifier/cfg.h"
#include "hphp/runtime/vm/verifier/util.h"
#include <boost/dynamic_bitset.hpp>

namespace HPHP {
namespace Verifier {

/**
 * Create all blocks and edges for one Func.
 */
Graph* GraphBuilder::build() {
  assert(!funcInstrs(m_func).empty());
  m_graph = new (m_arena) Graph();
  createBlocks();
  createExBlocks();
  linkBlocks();
  linkExBlocks();
  return m_graph;
}

/**
 * Create blocks for each entry point as well as ordinary control
 * flow boundaries.  Calls are not treated as basic-block ends.
 */
void GraphBuilder::createBlocks() {
  PC bc = m_unit->entry();
  m_graph->param_count = m_func->params().size();
  m_graph->first_linear = createBlock(m_func->base());
  // DV entry points
  m_graph->entries = new (m_arena) Block*[m_graph->param_count + 1];
  int dv_index = 0;
  for (auto& param : m_func->params()) {
    m_graph->entries[dv_index++] = !param.hasDefaultValue() ? 0 :
                                   createBlock(param.funcletOff);
  }
  // main entry point
  assert(dv_index == m_graph->param_count);
  m_graph->entries[dv_index] = createBlock(m_func->base());
  // ordinary basic block boundaries
  for (InstrRange i = funcInstrs(m_func); !i.empty(); ) {
    PC pc = i.popFront();
    if ((isCF(pc) || isTF(pc)) && !i.empty()) createBlock(i.front());
    if (isSwitch(peek_op(pc))) {
      foreachSwitchTarget(pc, [&](Offset o) { createBlock(pc + o); });
    } else {
      Offset target = instrJumpTarget(bc, pc - bc);
      if (target != InvalidAbsoluteOffset) createBlock(target);
    }
  }
}

/**
 * Link ordinary blocks with ordinary edges and set their last instruction
 * and end offsets
 */
void GraphBuilder::linkBlocks() {
  PC bc = m_unit->entry();
  Block* block = m_graph->first_linear;
  block->id = m_graph->block_count++;
  for (InstrRange i = funcInstrs(m_func); !i.empty(); ) {
    PC pc = i.popFront();
    block->last = pc;
    if (isCF(pc)) {
      if (isSwitch(peek_op(pc))) {
        int i = 0;
        foreachSwitchTarget(pc, [&](Offset o) {
          succs(block)[i++] = at(pc + o);
        });
      } else {
        Offset target = instrJumpTarget(bc, pc - bc);
        if (target != InvalidAbsoluteOffset) {
          assert(numSuccBlocks(block) > 0);
          succs(block)[numSuccBlocks(block) - 1] = at(target);
        }
      }
    }
    PC next_pc = !i.empty() ? i.front() : m_unit->at(m_func->past());
    Block* next = at(next_pc);
    if (peek_op(pc) == Op::Unwind) { // exn block
      const EHEnt* eh = m_func->findEHbyHandler(offset(pc));
      // link the end of each fault handler to the beginning of its parent
      succs(block)[0] = eh->m_parentIndex >= 0 ?
                        at(m_func->ehtab()[eh->m_parentIndex].m_handler) :
                        nullptr;
    }
    if (next) {
      block->next_linear = next;
      block->end = next_pc;
      if (!isTF(pc)) {
        assert(numSuccBlocks(block) > 0);
        succs(block)[0] = next;
      }
      block = next;
      block->id = m_graph->block_count++;
    }
  }
  block->end = m_unit->at(m_func->past());
}

/**
 * Create blocks from exception handler boundaries; the start and end of
 * each protected region is a boundary, and the catch or fault entrypoint
 * of each handler is also a boundary.
 */
void GraphBuilder::createExBlocks() {
  for (auto& handler : m_func->ehtab()) {
    createBlock(handler.m_base);
    if (handler.m_past != m_func->past()) {
      createBlock(handler.m_past);
    }
    createBlock(handler.m_handler);
  }
}

/**
 * return the next outermost handler or 0 if there aren't any
 */
const EHEnt* nextOuter(const Func::EHEntVec& ehtab, const EHEnt* eh) {
  return eh->m_parentIndex != -1 ? &ehtab[eh->m_parentIndex] : 0;
}

/**
 * Link primary body blocks to exception handler blocks as follows:
 * 1. For each block in the body, traverse from the innermost to
 *    outermost exception handler that includes the block.  For
 *    catch handlers, add an edge to each handler entry point. For
 *    fault handlers, add an edge to the fault handler and stop.
 * 2. For each fault handler block ending in Unwind, find the EH
 *    entry for the funclet, then traverse outwards starting from the
 *    next outermost.  Add any catch edges found, and stop at the first
 *    enclosing fault funclet, as in step 1.
 * The resulting linkage reflects exception control-flow; fault funclets
 * unconditionally handle any exception in their protected region, so they
 * "dominate" outer-more handlers.
 */
void GraphBuilder::linkExBlocks() {
  // For every block, add edges to reachable fault and catch handlers.
  for (LinearBlocks i = linearBlocks(m_graph); !i.empty(); ) {
    Block* b = i.popFront();
    assert(m_func->findEH(offset(b->start)) == m_func->findEH(offset(b->last)));
    Offset off = offset(b->start);
    const EHEnt* eh = m_func->findEH(off);
    if (eh != nullptr) {
      assert(eh->m_base <= off && off < eh->m_past);
      // the innermost exception handler is reachable from b
      b->exn = at(eh->m_handler);
    }
  }
}

Block** GraphBuilder::succs(Block* b) {
  if (!b->succs) {
    int num_succs = numSuccBlocks(b);
    Block** s = b->succs = new (m_arena) Block*[num_succs];
    memset(s, 0, sizeof(*s) * num_succs);
  }
  return b->succs;
}

Block* GraphBuilder::createBlock(PC pc) {
  BlockMap::iterator i = m_blocks.find(pc);
  if (i != m_blocks.end()) return i->second;

  Block* b = new (m_arena) Block(pc);
  m_blocks.insert(std::pair<PC,Block*>(pc, b));
  return b;
}

Block* GraphBuilder::at(PC target) const {
  auto const i = m_blocks.find(target);
  return i == m_blocks.end() ? 0 : i->second;
}

bool Block::reachable(Block* from, Block* to,
                      boost::dynamic_bitset<>& visited) {
   if (!from || !to) return false;
   if (visited[from->id]) return false;
   visited[from->id] = true;
   if (from->id == to->id) return true;
   for (int i = 0; i < numSuccBlocks(from); i++) {
     if (reachable(from->succs[i], to, visited)) return true;
   }
   if (from->exn) return reachable(from->exn, to, visited);
   return false;
}

/**
 * RpoSort does a depth-first search over successor and exception edges
 * of a Graph, visits blocks in postorder, and builds the reverse-postorder
 * list of blocks in-place.  Each block's rpo_id is the reverse-postorder
 * number (entry starts at 0, and so-on).
 *
 * Note that in functions with optional parameters, the lowest-numbered
 * DV entry point is typically the "entry" since it falls through to
 * higher-numbered DV entry points, the last of which ultimately jumps to
 * the primary function body.
 */
struct RpoSort {
  explicit RpoSort(Graph* g);
 private:
  void visit(Block* b);
 private:
  Graph* g;
  bool* visited;
  int rpo_id;
};

RpoSort::RpoSort(Graph* g) : g(g), rpo_id(0) {
  Arena scratch;
  visited = new (scratch) bool[g->block_count];
  memset(visited, 0, sizeof(bool) * g->block_count);
  g->first_rpo = 0;
  for (BlockPtrRange i = entryBlocks(g); !i.empty(); ) {
    visit(i.popFront());
  }
}

/**
 * Recursively visit b and its successors.  We search exception edges
 * first in a weak attempt to put exception blocks later in the final
 * reverse-postorder numbering, but it shouldn't matter and anyone
 * counting on that has a bug.
 */
void RpoSort::visit(Block* b) {
  if (visited[b->id]) return;
  visited[b->id] = true;
  if (b->exn) visit(b->exn);
  for (BlockPtrRange i = succBlocks(b); !i.empty(); ) visit(i.popBack());
  b->next_rpo = g->first_rpo;
  g->first_rpo = b;
  rpo_id++;
  b->rpo_id = g->block_count - rpo_id;
}

void sortRpo(Graph* g) {
  RpoSort _(g);
}

}}
