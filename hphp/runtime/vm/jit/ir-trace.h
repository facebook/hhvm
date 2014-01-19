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
#include "hphp/util/slice.h"
#include <algorithm>

namespace HPHP { namespace JIT {

struct IRUnit;

/*
 * An IRTrace is a single-entry, multi-exit, sequence of blocks. Typically
 * each block falls through to the next block but this is not guaranteed;
 * traces may contain internal forward-only control flow.
 */
struct IRTrace : private boost::noncopyable {
  static auto const kMinCap = 2;
  typedef List<Block*> Blocks;
  typedef Blocks::const_iterator const_iterator;
  typedef Blocks::iterator iterator;

  // Create a new trace, reserving room for cap blocks, then
  // add the given block.
  explicit IRTrace(IRUnit& unit, Block* first, size_t cap = kMinCap);

  Blocks& blocks() { return m_blocks; }
  const Blocks& blocks() const { return m_blocks; }

  Block* front() { return m_blocks[0]; }
  Block* back() { return m_blocks[m_blocks.size() - 1]; }
  const Block* front() const { return m_blocks[0]; }
  const Block* back()  const { return m_blocks[m_blocks.size() - 1 ]; }

  const_iterator cbegin() const { return blocks().cbegin(); }
  const_iterator cend()   const { return blocks().cend(); }
  const_iterator begin()  const { return blocks().begin(); }
  const_iterator end()    const { return blocks().end(); }
        iterator begin()        { return blocks().begin(); }
        iterator end()          { return blocks().end(); }

  // Unlink a block from a trace by forwarding any incoming edges to the
  // block's successor, then erasing it.
  iterator unlink(iterator it);

  // Erase a block from a trace. Updates any successor blocks that
  // have a DefLabel with a dest depending on this block.
  iterator erase(iterator it);

  // Add a block to the back of this trace's block list.
  Block* push_back(Block* b);

  // ensure the internal block list is presized to hold at least nblocks.
  void reserve(size_t nblocks);

  bool isMain() const;

  std::string toString() const;
  const IRUnit& unit() const { return m_unit; }

private:
  IRUnit& m_unit;
  Blocks m_blocks; // Blocks in main trace starting with entry
};

// defined here to avoid circular dependency
inline bool Block::isMain() const {
  return m_trace->isMain();
}

}}

#endif
