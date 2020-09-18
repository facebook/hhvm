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

#include <vector>

#include "hphp/runtime/base/type-string.h"

#include "hphp/hhbbc/misc.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace php { struct WideFunc; }

struct FuncAnalysis;
struct Index;
struct State;
struct VisitContext;

//////////////////////////////////////////////////////////////////////

/*
 * Perform DCE on a single basic block.
 */
void local_dce(VisitContext& visit, BlockId bid, const State&);

/*
 * Eliminate dead code in a function, across basic blocks, based on
 * results from a previous analyze_func call.
 *
 * Returns true if we should re-run the optimizer.
 */
bool global_dce(const Index&, const FuncAnalysis&, php::WideFunc&);

//////////////////////////////////////////////////////////////////////

const StaticString s_unreachable("static analysis error: supposedly "
                                 "unreachable code was reached");

//////////////////////////////////////////////////////////////////////

}}

