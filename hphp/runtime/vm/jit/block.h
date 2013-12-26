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
  typedef InstructionList::reference reference;
  typedef InstructionList::const_reference const_reference;

  // Execution frequency hint; codegen will put Unlikely blocks in astubs.
  enum class Hint { Neither, Likely, Unlikely };

  explicit Block(unsigned id)
    : m_trace(nullptr)
    , m_id(id)
    , m_hint(Hint::Neither)
  {}

  uint32_t    id() const           { return m_id; }
  IRTrace*    trace() const        { return m_trace; }
  void        setTrace(IRTrace* t) { m_trace = t; }
  Hint        hint() const         { return m_hint; }
  void        setHint(Hint hint)   { m_hint = hint; }

  void        addEdge(IRInstruction* jmp);
  void        removeEdge(IRInstruction* jmp);

  // Returns true if this block is part of the main trace.
  bool isMain() const;

  // Returns true if this block has no successors.
  bool isExit() const { return !taken() && !next(); }

  // Returns whether this block is the initial entry block for the tracelet.
  bool isEntry() const { return id() == 0; }

  // Returns whether this block starts with BeginCatch
  bool isCatch() const;

  // return the fallthrough block.  Should be nullptr if the last
  // instruction is a Terminal.
  Block* next() const { return back().next(); }
  Edge*  nextEdge()   { return back().nextEdge(); }

  // return the target block if the last instruction is a branch.
  Block* taken() const { return back().taken(); }
  Edge*  takenEdge()   { return back().takenEdge(); }

  // returns the number of successors.
  size_t numSuccs() const { return (bool)taken() + (bool)next(); }

  // return the postorder number of this block. (updated each time
  // sortBlocks() is called.
  unsigned postId() const { return m_postid; }
  void setPostId(unsigned id) { m_postid = id; }

  // Insert inst after this block's optional DefLabel and BeginCatch,
  // then return an iterator to the newly inserted instruction.
  iterator prepend(IRInstruction* inst);

  // return iterator to first instruction after the DefLabel (if
  // present) and BeginCatch (if present).
  iterator skipHeader();
  const_iterator skipHeader() const;

  // return iterator to last instruction
  iterator backIter();

  // return an iterator to a specific instruction
  iterator iteratorTo(IRInstruction* inst);

  // Accessors of list of predecessor edges.  Each edge has a inst() property
  // which is the instruction in the predecessor block.
  EdgeList& preds()             { return m_preds; }
  const EdgeList& preds() const { return m_preds; }
  size_t numPreds() const { return m_preds.size(); }

  // Remove edge from its destination's predecessor list and insert it in
  // new_to's predecessor list.
  static Block* updatePreds(Edge* edge, Block* new_to);

  // visit each src that provides a value to label->dsts[i]. body
  // should take an IRInstruction* and an SSATmp*.
  template<typename L> void forEachSrc(unsigned i, L body) const;

  // return the first src providing a value to label->dsts[i] for
  // which body(src) returns true, or nullptr if none are found.
  template<typename L> SSATmp* findSrc(unsigned i, L body);

  // execute body(P) for each predecessor block P of this block
  template <typename L> void forEachPred(L body);

  // list-compatible interface; these delegate to m_instrs but also update
  // inst.m_block
  InstructionList& instrs()      { return m_instrs; }
  bool             empty() const { return m_instrs.empty(); }
  iterator         begin()       { return m_instrs.begin(); }
  iterator         end()         { return m_instrs.end(); }
  const_iterator   begin() const { return m_instrs.begin(); }
  const_iterator   end()   const { return m_instrs.end(); }
  iterator         erase(iterator pos);
  iterator         erase(IRInstruction* inst);
  iterator insert(iterator pos, IRInstruction* inst);
  void splice(iterator pos, Block* from, iterator begin, iterator end);
  void push_back(IRInstruction* inst);
  template <class Predicate> void remove_if(Predicate p);

  // return the first instruction in the block.
  reference front();
  const_reference front() const;

  // return the last instruction in the block
  reference back();
  const_reference back() const;

  friend const Edge* nextEdge(Block*); // only for validation

  std::string toString() const;

 private:
  InstructionList m_instrs; // instructions in this block
  IRTrace* m_trace;         // owner of this block.
  const unsigned m_id;      // unit-assigned unique id of this block
  unsigned m_postid;        // postorder number of this block
  EdgeList m_preds;         // Edges that point to this block
  Hint m_hint;              // execution frequency hint
};

typedef smart::vector<Block*> BlockList;

inline Block::reference Block::front() {
  assert(!m_instrs.empty());
  return m_instrs.front();
}
inline Block::const_reference Block::front() const {
  return const_cast<Block*>(this)->front();
}

inline Block::reference Block::back() {
  assert(!m_instrs.empty());
  return m_instrs.back();
}
inline Block::const_reference Block::back() const {
  return const_cast<Block*>(this)->back();
}

inline Block::iterator Block::erase(iterator pos) {
  pos->setBlock(nullptr);
  return m_instrs.erase(pos);
}

inline Block::iterator Block::erase(IRInstruction* inst) {
  assert(inst->block() == this);
  return erase(iteratorTo(inst));
}

inline Block::iterator Block::prepend(IRInstruction* inst) {
  assert(inst->marker().valid());
  auto it = skipHeader();
  return insert(it, inst);
}

inline Block::iterator Block::skipHeader() {
  auto it = begin();
  auto e = end();
  if (it != e && it->op() == DefLabel) ++it;
  if (it != e && it->op() == BeginCatch) ++it;
  return it;
}

inline Block::const_iterator Block::skipHeader() const {
  return const_cast<Block*>(this)->skipHeader();
}

inline Block::iterator Block::backIter() {
  assert(!empty());
  auto it = end();
  return --it;
}

inline Block::iterator Block::iteratorTo(IRInstruction* inst) {
  assert(inst->block() == this);
  return m_instrs.iterator_to(*inst);
}

inline Block* Block::updatePreds(Edge* edge, Block* new_to) {
  if (Block* old_to = edge->to()) {
    auto &preds = old_to->m_preds;
    preds.erase(preds.iterator_to(*edge));
  }
  if (new_to) {
    new_to->m_preds.push_front(*edge);
  }
  return new_to;
}

template<typename L> inline
void Block::forEachSrc(unsigned i, L body) const {
  for (auto const& e : m_preds) {
    auto jmp = e.inst();
    assert(jmp->op() == Jmp && jmp->taken() == this);
    body(jmp, jmp->src(i));
  }
}

template<typename L> inline
SSATmp* Block::findSrc(unsigned i, L body) {
  for (Edge& e : m_preds) {
    SSATmp* src = e.inst()->src(i);
    if (body(src)) return src;
  }
  return nullptr;
}

template <typename L> inline
void Block::forEachPred(L body) {
  for (auto i = m_preds.begin(), e = m_preds.end(); i != e;) {
    auto inst = i->inst();
    ++i;
    body(inst->block());
  }
}

inline Block::iterator Block::insert(iterator pos, IRInstruction* inst) {
  assert(inst->marker().valid());
  inst->setBlock(this);
  return m_instrs.insert(pos, *inst);
}

inline
void Block::splice(iterator pos, Block* from, iterator begin, iterator end) {
  assert(from != this);
  for (auto i = begin; i != end; ++i) i->setBlock(this);
  m_instrs.splice(pos, from->instrs(), begin, end);
}

inline void Block::push_back(IRInstruction* inst) {
  assert(inst->marker().valid());
  inst->setBlock(this);
  return m_instrs.push_back(*inst);
}

template <class Predicate> inline
void Block::remove_if(Predicate p) {
  m_instrs.remove_if(p);
}

inline bool Block::isCatch() const {
  // Catch blocks always start with DefLabel; BeginCatch.
  if (empty()) return false;
  auto it = skipHeader();
  if (it == begin()) return false;
  return (--it)->op() == BeginCatch;
}

// defined here to avoid circular dependencies
inline void Edge::setTo(Block* to) {
  m_to = Block::updatePreds(this, to);
}

}}

#endif
