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

#ifndef incl_HPHP_VM_TRACE_H_
#define incl_HPHP_VM_TRACE_H_

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/ir.h"

namespace HPHP { namespace JIT {

/*
 * An IRTrace is a single-entry, multi-exit, sequence of blocks. Typically
 * each block falls through to the next block but this is not guaranteed;
 * traces may contain internal forward-only control flow.
 */
struct IRTrace : private boost::noncopyable {
  typedef std::list<Block*>::const_iterator const_iterator;
  typedef std::list<Block*>::iterator iterator;

  explicit IRTrace(Block* first, uint32_t bcOff)
    : m_bcOff(bcOff)
    , m_main(nullptr)
  {
    push_back(first);
  }

  ~IRTrace() {
    std::for_each(m_exitTraces.begin(), m_exitTraces.end(),
                  boost::checked_deleter<IRTrace>());
  }

  std::list<Block*>& blocks() { return m_blocks; }
  const std::list<Block*>& blocks() const { return m_blocks; }

  Block* front() { return *m_blocks.begin(); }
  Block* back() { auto it = m_blocks.end(); return *(--it); }
  const Block* front() const { return *m_blocks.begin(); }
  const Block* back()  const { auto it = m_blocks.end(); return *(--it); }

  const_iterator cbegin() const { return blocks().cbegin(); }
  const_iterator cend()   const { return blocks().cend(); }
  const_iterator begin()  const { return blocks().begin(); }
  const_iterator end()    const { return blocks().end(); }
        iterator begin()        { return blocks().begin(); }
        iterator end()          { return blocks().end(); }

  /*
   * Unlink a block from a trace. Updates any successor blocks that
   * have a DefLabel with a dest depending on this block.
   */
  iterator erase(iterator it) {
    Block* b = *it;
    assert(b->preds().empty());
    it = m_blocks.erase(it);
    b->setTrace(nullptr);
    if (!b->empty()) b->back()->setTaken(nullptr);
    b->setNext(nullptr);
    return it;
  }

  /*
   * Add a block to the back of this trace's block list.
   */
  Block* push_back(Block* b) {
    b->setTrace(this);
    m_blocks.push_back(b);
    return b;
  }

  // temporary data field for use by individual passes
  //
  // Used by LinearScan as a "fake" instruction id, that comes
  // between the id of the last instruction that branches to
  // this exit trace, and the next instruction on the main trace.
  uint32_t data() const { return m_data; }
  void setData(uint32_t d) { m_data = d; }

  uint32_t bcOff() const { return m_bcOff; }
  IRTrace* addExitTrace(IRTrace* exit) {
    m_exitTraces.push_back(exit);
    exit->setMain(this);
    return exit;
  }
  bool isMain() const { return m_main == nullptr; }
  /*
   * Catch traces always start with DefLabel; BeginCatch.
   */
  bool isCatch() const {
    if (front()->empty()) return false;

    auto it = front()->skipHeader();
    if (it == front()->begin()) return false;

    return (--it)->op() == BeginCatch;
  }
  void setMain(IRTrace* t) {
    assert(m_main == nullptr);
    m_main = t;
  }
  IRTrace* main() {
    return m_main;
  }

  typedef std::list<IRTrace*> ExitList;
  typedef std::list<IRTrace*>::iterator ExitIterator;

  ExitList& exitTraces() { return m_exitTraces; }
  const ExitList& exitTraces() const { return m_exitTraces; }
  std::string toString() const;

private:
  // offset of the first bytecode in this trace; 0 if this trace doesn't
  // represent a bytecode boundary.
  uint32_t m_bcOff;
  uint32_t m_data;
  std::list<Block*> m_blocks; // Blocks in main trace starting with entry block
  ExitList m_exitTraces;      // traces to which this trace exits
  IRTrace* m_main;            // ptr to parent trace if this is an exit trace
};

}}

#endif
