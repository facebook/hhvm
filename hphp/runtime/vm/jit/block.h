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

#ifndef incl_HPHP_VM_BLOCK_H_
#define incl_HPHP_VM_BLOCK_H_

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/edge.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"

namespace HPHP { namespace JIT {

/*
 * A Block refers to a basic block: single-entry, single-exit, list of
 * instructions.  The instruction list is an intrusive list, so each
 * instruction can only be in one block at a time.  Likewise, a block
 * can only be owned by one trace at a time.
 *
 * Block owns the InstructionList, but exposes several list methods itself
 * so usually you can use Block directly.  These methods also update
 * IRInstruction::m_block transparently.
 */
struct Block : boost::noncopyable {
  typedef InstructionList::iterator iterator;
  typedef InstructionList::const_iterator const_iterator;

  // Execution frequency hint; codegen will put Unlikely blocks in astubs.
  enum class Hint { Neither, Likely, Unlikely };

  Block(unsigned id, const Func* func)
    : m_trace(nullptr)
    , m_func(func)
    , m_next(this, nullptr)
    , m_id(id)
    , m_hint(Hint::Neither)
  {}

  const IRInstruction* beginCatch() const {
    auto it = begin();
    ++it;
    assert(it->op() == BeginCatch);
    return &*it;
  }

  uint32_t    id() const           { return m_id; }
  IRTrace*    trace() const        { return m_trace; }
  void        setTrace(IRTrace* t) { m_trace = t; }
  Hint        hint() const         { return m_hint; }
  void        setHint(Hint hint)   { m_hint = hint; }

  void        addEdge(IRInstruction* jmp);
  void        removeEdge(IRInstruction* jmp);

  /*
   * Returns true if this block is the main trace exit.  This block
   * post-dominates all main trace blocks.
   *
   * Currently there is only ever a single main trace exit.
   */
  bool isMainExit() const;

  /*
   * Returns true if this block is part of the main trace.  I.e. it is
   * post-dominated by a block with isMainExit() == true.
   */
  bool isMain() const;

  /*
   * Returns: !taken() && !next().
   */
  bool isExit() const;

  /*
   * Returns whether this block is the initial entry block for the
   * tracelet.
   */
  bool isEntry() const { return id() == 0; }

  // return the last instruction in the block
  IRInstruction* back() const {
    assert(!m_instrs.empty());
    auto it = m_instrs.end();
    return const_cast<IRInstruction*>(&*(--it));
  }

  // return the first instruction in the block.
  IRInstruction* front() const {
    assert(!m_instrs.empty());
    return const_cast<IRInstruction*>(&*m_instrs.begin());
  }

  // return the fallthrough block.  Should be nullptr if the last
  // instruction is a Terminal.
  Block* next() const { return m_next.to(); }
  void setNext(Block* b) { m_next.setTo(b); }

  // return the target block if the last instruction is a branch.
  Block* taken() const {
    return empty() ? nullptr : back()->taken();
  }

  // return the postorder number of this block. (updated each time
  // sortBlocks() is called.
  unsigned postId() const { return m_postid; }
  void setPostId(unsigned id) { m_postid = id; }

  /*
   * Insert inst after this block's optional DefLabel and BeginCatch,
   * then return an iterator to the newly inserted instruction.
   */
  iterator prepend(IRInstruction* inst) {
    assert(inst->marker().valid());
    auto it = skipHeader();
    return insert(it, inst);
  }

  // return iterator to first instruction after the DefLabel (if
  // present) and BeginCatch (if present).
  iterator skipHeader() {
    auto it = begin();
    auto e = end();
    if (it != e && it->op() == DefLabel) ++it;
    if (it != e && it->op() == BeginCatch) ++it;
    return it;
  }

  const_iterator skipHeader() const {
    return const_cast<Block*>(this)->skipHeader();
  }

  // return iterator to last instruction
  iterator backIter() {
    assert(!empty());
    auto it = end();
    return --it;
  }

  // return an iterator to a specific instruction
  iterator iteratorTo(IRInstruction* inst) {
    assert(inst->block() == this);
    return m_instrs.iterator_to(*inst);
  }

  // Accessors of list of predecessor edges.  Each edge has a from() property
  // which is the predecessor block.
  EdgeList& preds()             { return m_preds; }
  const EdgeList& preds() const { return m_preds; }
  size_t numPreds() const { return m_preds.size(); }

  // Remove edge from its destination's predecessor list and insert it in
  // new_to's predecessor list.
  static Block* updatePreds(Edge* edge, Block* new_to) {
    if (Block* old_to = edge->to()) {
      auto &preds = old_to->m_preds;
      preds.erase(preds.iterator_to(*edge));
    }
    if (new_to) {
      new_to->m_preds.push_front(*edge);
    }
    return new_to;
  }

  // visit each src that provides a value to label->dsts[i]. body
  // should take an IRInstruction* and an SSATmp*.
  template<typename L>
  void forEachSrc(unsigned i, L body) {
    for (Edge& e : m_preds) {
      IRInstruction* jmp = e.from()->back();
      assert(jmp->op() == Jmp_ && jmp->taken() == this);
      body(jmp, jmp->src(i));
    }
  }

  // return the first src providing a value to label->dsts[i] for
  // which body(src) returns true, or nullptr if none are found.
  template<typename L>
  SSATmp* findSrc(unsigned i, L body) {
    for (Edge& e : m_preds) {
      SSATmp* src = e.from()->back()->src(i);
      if (body(src)) return src;
    }
    return nullptr;
  }

  template <typename L>
  void forEachPred(L body) {
    for (auto i = m_preds.begin(), e = m_preds.end(); i != e;) {
      Block* from = i->from();
      ++i;
      body(from);
    }
  }

  // list-compatible interface; these delegate to m_instrs but also update
  // inst.m_block
  InstructionList& instrs()      { return m_instrs; }
  bool             empty() const { return m_instrs.empty(); }
  iterator         begin()       { return m_instrs.begin(); }
  iterator         end()         { return m_instrs.end(); }
  const_iterator   begin() const { return m_instrs.begin(); }
  const_iterator   end()   const { return m_instrs.end(); }

  iterator insert(iterator pos, IRInstruction* inst) {
    assert(inst->marker().valid());
    inst->setBlock(this);
    return m_instrs.insert(pos, *inst);
  }
  void splice(iterator pos, Block* from, iterator begin, iterator end,
              BCMarker newMarker) {
    assert(from != this);
    for (auto i = begin; i != end; ++i) {
      i->setBlock(this);
      i->setMarker(newMarker);
    }
    m_instrs.splice(pos, from->instrs(), begin, end);
  }
  void push_back(IRInstruction* inst) {
    assert(inst->marker().valid());
    inst->setBlock(this);
    return m_instrs.push_back(*inst);
  }
  template <class Predicate> void remove_if(Predicate p) {
    m_instrs.remove_if(p);
  }
  iterator erase(iterator pos) {
    return m_instrs.erase(pos);
  }

  friend const Edge* nextEdge(Block*); // only for validation

 private:
  InstructionList m_instrs; // instructions in this block
  IRTrace* m_trace;         // owner of this block.
  const Func* m_func;       // which func are we in
  Edge m_next;              // fall-through path; null if back()->isTerminal().
  const unsigned m_id;      // factory-assigned unique id of this block
  unsigned m_postid;        // postorder number of this block
  EdgeList m_preds;         // Edges that point to this block
  Hint m_hint;              // execution frequency hint
};

typedef smart::vector<Block*> BlockList;

inline void Edge::setTo(Block* to) {
  m_to = Block::updatePreds(this, to);
}

}}

#endif
