/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HHBBC_DCE_H_
#define incl_HHBBC_DCE_H_

#include <vector>

#include "hphp/hhbbc/misc.h"

namespace HPHP { namespace HHBBC {

struct Index;
struct State;
struct Context;
struct Bytecode;
struct FuncAnalysis;
namespace php { struct Block; }

//////////////////////////////////////////////////////////////////////

/*
 * Perform DCE on a single basic block.
 */
void local_dce(const Index&, Context, borrowed_ptr<php::Block>, const State&);

/*
 * Eliminate dead code in a function, across basic blocks, based on
 * results from a previous analyze_func call.
 */
void global_dce(const Index&, const FuncAnalysis&);

/*
 * Assist in removing blocks that aren't reachable by removing
 * conditional jumps that are never taken.  Conditional jumps that are
 * always taken are turned into unconditional jumps in first_pass.
 *
 * If options.RemoveDeadBlocks is off, this function just replaces
 * blocks we believe are unreachable with fatal opcodes.
 */
void remove_unreachable_blocks(const Index&, const FuncAnalysis&);

//////////////////////////////////////////////////////////////////////

}}

#endif
