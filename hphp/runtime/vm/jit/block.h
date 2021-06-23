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

#include <algorithm>

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/edge.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"

namespace HPHP { namespace jit {

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
struct Block {
  typedef InstructionList::iterator iterator;
  typedef InstructionList::const_iterator const_iterator;
  typedef InstructionList::reverse_iterator reverse_iterator;
  typedef InstructionList::const_reverse_iterator const_reverse_iterator;
  typedef InstructionList::reference reference;
  typedef InstructionList::const_reference const_reference;

  /*
   * Execution frequency hint; codegen will put Unlikely blocks in acold,
   * and Unused blocks in afrozen.
   *
   * 'Main' code, or code that executes most frequently, should have either
   * the 'Likely' or 'Neither' Block::Hint. Code for these blocks is
   * emitted into the 'a' section.
   *
   * Code that handles infrequent cases should have the 'Unlikely'
   * Block::Hint. Example of such code are decref helpers that free objects
   * when the ref-count goes to zero. Code for these blocks is emitted into
   * the 'acold' section.
   *
   * Code that is either executed once, or is highly unlikely to be ever
   * executed, or code that will become dead in the future should have
   * the 'Unlikely' Hint. Examples of these include Service Request stubs
   * (executed once), Catch blocks (highly unlikely), and cold code
   * emitted in profiling mode (which become dead after optimized code is
   * emitted). Code for these blocks is emitted into the 'afrozen' section.
   *
   * See also runtime/vm/jit/code-cache.h for comment on the 'hot' and 'prof'
   * sections.
   *
   * IMPORTANT NOTE: These hints are sorted in increasing order or likelihood.
   * This order is used in fixBlockHints().
   */

  enum class Hint { Unused, Unlikely, Neither, Likely };

  explicit Block(unsigned id, uint64_t profCount)
    : m_profCount(checkedProfCount(profCount))
    , m_id(id)
    , m_hint(Hint::Neither)
  {}

  Block(const Block&) = delete;
  Block& operator=(const Block&) = delete;

  unsigned    id() const           { return m_id; }
  Hint        hint() const         { return m_hint; }
  void        setHint(Hint hint)   { m_hint = hint; }
  uint64_t    profCount() const    { return m_profCount; }

  void setProfCount(uint64_t count) { m_profCount = checkedProfCount(count); }

  // Returns true if this block has no successors.
  bool isExit() const { return !empty() && !taken() && !next(); }

  // Returns whether this block is the initial entry block for the tracelet.
  bool isEntry() const { return id() == 0; }

  // Returns whether this block starts with BeginCatch
  bool isCatch() const;

  // Returns true if this block is an exit, assuming that the last
  // instruction won't throw an exception.  In other words, the block
  // doesn't have a next edge, and it either has no taken edge or its
  // taken edge goes to a catch block.
  bool isExitNoThrow() const {
    return !empty() && back().isTerminal() && (!taken() || taken()->isCatch());
  }

  bool isUnreachable() const {
    return !empty() && back().is(Unreachable);
  }

  // If its a catch block, the BeginCatch's marker
  const BCMarker& catchMarker() const;

  // return the fallthrough block.  Should be nullptr if the last instruction
  // is a Terminal.
  Block* next() const { return back().next(); }
  Edge*  nextEdge()   { return back().nextEdge(); }

  // return the target block if the last instruction is a branch.
  Block* taken() const { return back().taken(); }
  Edge*  takenEdge()   { return back().takenEdge(); }

  // returns the number of successors.
  size_t numSuccs() const { return (bool)taken() + (bool)next(); }

  // return the postorder number of this block. (updated each time
  // sortBlocks() is called.

  // Insert inst after this block's optional DefLabel and BeginCatch,
  // then return an iterator to the newly inserted instruction.
  iterator prepend(IRInstruction* inst);

  // return iterator to first instruction after any DefFP, DefFrameRelSP,
  // DefRegSP, DefLabel, and/or BeginCatch instructions.
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
  template <typename L> void forEachPred(L body) const;

  // execute body(P) for each successor block P of this block
  template <typename L> void forEachSucc(L body) const;

  // list-compatible interface; these delegate to m_instrs but also update
  // inst.m_block
  InstructionList& instrs()      { return m_instrs; }
  const InstructionList&
                   instrs() const{ return m_instrs; }
  bool             empty() const { return m_instrs.empty(); }
  iterator         begin()       { return m_instrs.begin(); }
  iterator         end()         { return m_instrs.end(); }
  const_iterator   begin() const { return m_instrs.begin(); }
  const_iterator   end()   const { return m_instrs.end(); }
  reverse_iterator rbegin()       { return m_instrs.rbegin(); }
  reverse_iterator rend()         { return m_instrs.rend(); }
  const_reverse_iterator rbegin() const { return m_instrs.rbegin(); }
  const_reverse_iterator rend()   const { return m_instrs.rend(); }

  // Erase the given instruction from this block and unlinks any outgoing edges.
  // These methods don't delete the instruction, so it may be reused after
  // calling erase().
  iterator         erase(iterator pos);
  iterator         erase(IRInstruction* inst);

  iterator insert(iterator pos, IRInstruction* inst);
  void splice(iterator pos, Block* from, iterator begin, iterator end);
  void push_back(std::initializer_list<IRInstruction*> insts);
  void push_back(IRInstruction* inst);
  template <class Predicate> void remove_if(Predicate p);
  InstructionList&& moveInstrs();

  // return the first instruction in the block.
  reference front();
  const_reference front() const;

  // return the last instruction in the block
  reference back();
  const_reference back() const;

  friend const Edge* nextEdge(Block*); // only for validation

  std::string toString() const;

 private:
  static uint64_t checkedProfCount(uint64_t profCount);

  InstructionList m_instrs; // instructions in this block
  EdgeList m_preds;         // Edges that point to this block
  uint64_t m_profCount;     // execution profile count of the region block
                            // containing this IR block.
  const unsigned m_id;      // unit-assigned unique id of this block
  Hint m_hint;              // execution frequency hint
};

// Try to keep this structure small; watch for alignment issues.
static_assert(sizeof(Block) == 64, "");

using BlockList = jit::vector<Block*>;
using BlockSet = jit::flat_set<Block*>;

inline Block::reference Block::front() {
  assertx(!m_instrs.empty());
  return m_instrs.front();
}
inline Block::const_reference Block::front() const {
  return const_cast<Block*>(this)->front();
}

inline Block::reference Block::back() {
  assertx(!m_instrs.empty());
  return m_instrs.back();
}
inline Block::const_reference Block::back() const {
  return const_cast<Block*>(this)->back();
}

inline Block::iterator Block::erase(iterator pos) {
  if (pos->hasEdges()) pos->clearEdges();
  pos->setBlock(nullptr);
  return m_instrs.erase(pos);
}

inline Block::iterator Block::erase(IRInstruction* inst) {
  assertx(inst->block() == this);
  return erase(iteratorTo(inst));
}

inline Block::iterator Block::prepend(IRInstruction* inst) {
  assertx(inst->marker().valid());
  auto it = skipHeader();
  return insert(it, inst);
}

inline Block::iterator Block::skipHeader() {
  auto it = begin();
  auto e = end();
  while (it != e &&
         it->is(DefFP, DefFrameRelSP, DefRegSP, DefLabel, BeginCatch)) {
    ++it;
  }
  return it;
}

inline Block::const_iterator Block::skipHeader() const {
  return const_cast<Block*>(this)->skipHeader();
}

inline Block::iterator Block::backIter() {
  assertx(!empty());
  auto it = end();
  return --it;
}

inline Block::iterator Block::iteratorTo(IRInstruction* inst) {
  assertx(inst->block() == this);
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
    assertx(jmp->op() == Jmp && jmp->taken() == this);
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
void Block::forEachPred(L body) const {
  for (auto i = m_preds.begin(), e = m_preds.end(); i != e;) {
    auto inst = i->inst();
    ++i;
    body(inst->block());
  }
}

template <typename L> inline
void Block::forEachSucc(L body) const {
  if (auto n = next()) body(n);
  if (auto t = taken()) body(t);
}

inline Block::iterator Block::insert(iterator pos, IRInstruction* inst) {
  assertx(inst->marker().valid());
  inst->setBlock(this);
  return m_instrs.insert(pos, *inst);
}

inline
void Block::splice(iterator pos, Block* from, iterator begin, iterator end) {
  assertx(from != this);
  for (auto i = begin; i != end; ++i) i->setBlock(this);
  m_instrs.splice(pos, from->instrs(), begin, end);
}

inline void Block::push_back(std::initializer_list<IRInstruction*> insts) {
  for (auto inst : insts) { push_back(inst); }
}

inline void Block::push_back(IRInstruction* inst) {
  assertx(inst->marker().valid());
  inst->setBlock(this);
  return m_instrs.push_back(*inst);
}

template <class Predicate>
inline void Block::remove_if(Predicate p) {
  m_instrs.remove_if(p);
}

inline InstructionList&& Block::moveInstrs() {
  for (auto i = begin(); i != end(); ++i) i->setBlock(nullptr);
  return std::move(m_instrs);
}

inline bool Block::isCatch() const {
  // Catch blocks always start with DefLabel; BeginCatch.
  if (empty()) return false;
  auto it = skipHeader();
  if (it == begin()) return false;
  return (--it)->op() == BeginCatch;
}

inline uint64_t Block::checkedProfCount(uint64_t profCount) {
  // If the profCount is too big, it typically means some issue.  So
  // trigger an assert in debug builds, and cap it in opt builds.
  constexpr auto profCountCap = std::numeric_limits<int64_t>::max();
  assert_flog(profCount <= profCountCap,
              "Trying to create IR block with suspicious profCount {}\n",
              profCount);
  if (profCount > profCountCap) profCount = profCountCap;
  return profCount;
}

inline const BCMarker& Block::catchMarker() const {
  assertx(isCatch());
  auto it = skipHeader();
  assertx(it != begin());
  return (--it)->marker();
}

// defined here to avoid circular dependencies
inline void Edge::setTo(Block* to) {
  m_to = Block::updatePreds(this, to);
}

inline Block* Edge::from() const {
  return inst() != nullptr ? inst()->block() : nullptr;
}

inline const char* blockHintName(Block::Hint hint) {
  switch (hint) {
    case Block::Hint::Unused:   return "Unused";
    case Block::Hint::Unlikely: return "Unlikely";
    case Block::Hint::Neither:  return "Neither";
    case Block::Hint::Likely:   return "Likely";
  }
  not_reached();
}

inline Optional<Block::Hint> nameToHint(const std::string& hintStr) {
  if (hintStr == "Unused")   return Block::Hint::Unused;
  if (hintStr == "Unlikely") return Block::Hint::Unlikely;
  if (hintStr == "Neither")  return Block::Hint::Neither;
  if (hintStr == "Likely")   return Block::Hint::Likely;
  return std::nullopt;
}

}}
