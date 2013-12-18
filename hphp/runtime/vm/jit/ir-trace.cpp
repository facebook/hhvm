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

#include "hphp/runtime/vm/jit/ir-trace.h"

#include "hphp/runtime/vm/jit/ir-unit.h"

namespace HPHP { namespace JIT {

IRTrace::IRTrace(IRUnit& unit, Block* first, size_t cap)
  : m_unit(unit)
  , m_blocks(new (unit.arena()) Block*[cap], 0, cap) {
  assert(cap >= kMinCap);
  push_back(first);
}

IRTrace::iterator IRTrace::unlink(iterator blockIt) {
  // Update any predecessors to point to the empty block's next block.
  auto block = *blockIt;
  assert(block->empty());
  auto next = block->next();
  for (auto it = block->preds().begin(); it != block->preds().end(); ) {
    auto cur = it;
    ++it;
    cur->setTo(next);
  }
  return erase(blockIt);
}

IRTrace::iterator IRTrace::erase(iterator it) {
  assert((*it)->preds().empty());
  Block* b = *it;
  b->setTrace(nullptr);
  if (!b->empty()) {
    b->back().setTaken(nullptr);
    b->back().setNext(nullptr);
  }
  return m_blocks.erase(it);
}

void IRTrace::reserve(size_t cap) {
  if (m_blocks.capacity() < cap) {
    auto blocks = new (m_unit.arena()) Block*[cap];
    std::copy(m_blocks.begin(), m_blocks.end(), blocks);
    m_blocks = List<Block*>(blocks, m_blocks.len, cap);
  }
}

Block* IRTrace::push_back(Block* b) {
  b->setTrace(this);
  auto len = m_blocks.size();
  auto cap = m_blocks.capacity();
  if (len == cap) {
    static_assert(kMinCap / 2 > 0, "");
    reserve(cap + cap / 2); // grow by 1.5x
  }
  m_blocks.push_back(b);
  return b;
}

}}
