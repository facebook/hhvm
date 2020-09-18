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

#include "hphp/hhbbc/representation.h"

namespace HPHP { namespace HHBBC {

struct Index;
struct FuncAnalysis;

//////////////////////////////////////////////////////////////////////

/*
 * Assist in removing blocks that aren't reachable by removing
 * conditional jumps that are never taken.  Conditional jumps that are
 * always taken are turned into unconditional jumps in first_pass.
 *
 * If options.RemoveDeadBlocks is off, this function just replaces
 * blocks we believe are unreachable with fatal opcodes.
 */
void remove_unreachable_blocks(const FuncAnalysis&, php::WideFunc& func);

/*
 * Simplify control flow, and create Switch and SSwitch bytecodes.
 */
bool control_flow_opts(const FuncAnalysis&, php::WideFunc& func);

/*
 * Split critical edges.
 *
 * Some optimizations require or are better if we split critical edges in the
 * cfg.  One example of this is UnsetL insertion.  With the following cfg:
 *
 *  B1
 *  | \
 *  |  B2
 *  | /
 *  B3
 *
 * Suppose in B1 we have local l1 holding a counted value.  B2 may leave local
 * l1 uninit (after a move optisation is applied during a call).  Inserting an
 * UnsetL(l1) at the start of block B3 will require a decref of
 * Union(TCnt, TUninit), which is not great.  If we split critical edges, the
 * UnsetL(l1) (and its decref) could sit on the edge from B1 to B2 making it
 * a decref of TCnt.
 *
 * Critical edge blocks that remain a single nop will get folded away by
 * control_flow_opts.
 */
void split_critical_edges(const Index&, FuncAnalysis&, php::WideFunc& func);

//////////////////////////////////////////////////////////////////////

}}

