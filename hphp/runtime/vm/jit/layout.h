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
#ifndef incl_HPHP_JIT_LAYOUT_H_
#define incl_HPHP_JIT_LAYOUT_H_

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/jit/block.h"

namespace HPHP { namespace JIT {

//////////////////////////////////////////////////////////////////////

struct Block;
class IRUnit;

/*
 * Information about where to position the blocks in a trace.
 *
 * The blocks are listed in the order they should be positioned, with
 * astubsIt pointing at the place we've split the blocks between a and
 * astubs.
 */
struct LayoutInfo {
  BlockList blocks;
  BlockList::iterator astubsIt;
};

/*
 * Determine the order that blocks should be emitted for codegen.  The
 * goal is to minimize branching and put related blocks close to each
 * other.
 */
LayoutInfo layoutBlocks(const IRUnit&);

//////////////////////////////////////////////////////////////////////

}}

#endif
