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
  // Create a new trace, reserving room for cap blocks, then
  // add the given block.
  explicit IRTrace(IRUnit& unit, Block* first);

  Block* entry() { return m_entry; }

  // Unlink a block from a trace by forwarding any incoming edges to the
  // block's successor, then erasing it.
  void unlink(Block* b);

  // Erase a block from a trace. Updates any successor blocks that
  // have a DefLabel with a dest depending on this block.
  void erase(Block* b);

  std::string toString() const;
  const IRUnit& unit() const { return m_unit; }

private:
  IRUnit& m_unit;
  Block* m_entry;
};

}}

#endif
