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

namespace HPHP { namespace JIT {

struct IRUnit;

/*
 * An IRTrace is a single-entry, multi-exit, sequence of blocks. Typically
 * each block falls through to the next block but this is not guaranteed;
 * traces may contain internal forward-only control flow.
 */
struct IRTrace : private boost::noncopyable {
  typedef std::list<Block*>::const_iterator const_iterator;
  typedef std::list<Block*>::iterator iterator;

  explicit IRTrace(IRUnit& unit, Block* first);

  std::list<Block*>& blocks() { return m_blocks; }
  const std::list<Block*>& blocks() const { return m_blocks; }

  Block* front() { return m_blocks.front(); }
  Block* back() { auto it = m_blocks.end(); return *(--it); }
  const Block* front() const { return *m_blocks.begin(); }
  const Block* back()  const { auto it = m_blocks.end(); return *(--it); }

  const_iterator cbegin() const { return blocks().cbegin(); }
  const_iterator cend()   const { return blocks().cend(); }
  const_iterator begin()  const { return blocks().begin(); }
  const_iterator end()    const { return blocks().end(); }
        iterator begin()        { return blocks().begin(); }
        iterator end()          { return blocks().end(); }

  // Unlink a block from a trace. Updates any successor blocks that
  // have a DefLabel with a dest depending on this block.
  iterator erase(iterator it);

  // Add a block to the back of this trace's block list.
  Block* push_back(Block* b);

  bool isMain() const;

  std::string toString() const;
  const IRUnit& unit() const { return m_unit; }

private:
  IRUnit& m_unit;
  std::list<Block*> m_blocks; // Blocks in main trace starting with entry block
};

inline IRTrace::IRTrace(IRUnit& unit, Block* first)
  : m_unit(unit) {
  push_back(first);
}

inline IRTrace::iterator IRTrace::erase(iterator it) {
  Block* b = *it;
  assert(b->preds().empty());
  it = m_blocks.erase(it);
  b->setTrace(nullptr);
  if (!b->empty()) b->back()->setTaken(nullptr);
  b->setNext(nullptr);
  return it;
}

inline Block* IRTrace::push_back(Block* b) {
  b->setTrace(this);
  m_blocks.push_back(b);
  return b;
}

// defined here to avoid circular dependency
inline bool Block::isMain() const {
  return m_trace->isMain();
}

}}

#endif
