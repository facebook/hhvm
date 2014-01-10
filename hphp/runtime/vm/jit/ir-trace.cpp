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

IRTrace::IRTrace(IRUnit& unit, Block* first)
  : m_unit(unit)
  , m_entry(first) {
}

void IRTrace::unlink(Block* block) {
  // Update any predecessors to point to the empty block's next block.
  assert(block->empty());
  auto next = block->next();
  for (auto it = block->preds().begin(); it != block->preds().end(); ) {
    auto cur = it;
    ++it;
    cur->setTo(next);
  }
}

void IRTrace::erase(Block* block) {
  assert(block->preds().empty());
  block->setTrace(nullptr);
  if (!block->empty()) {
    block->back().setTaken(nullptr);
    block->back().setNext(nullptr);
  }
}

}}
