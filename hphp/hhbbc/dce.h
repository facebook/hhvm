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
#ifndef incl_HHBBC_DCE_H_
#define incl_HHBBC_DCE_H_

#include <vector>

#include "hphp/runtime/base/type-string.h"

#include "hphp/hhbbc/misc.h"

namespace HPHP { namespace HHBBC {

struct Index;
struct State;
struct Context;
struct Bytecode;
struct FuncAnalysis;
struct CollectedInfo;
namespace php { struct Block; }

//////////////////////////////////////////////////////////////////////

/*
 * Perform DCE on a single basic block.
 */
void local_dce(const Index&, const FuncAnalysis&, CollectedInfo& collect,
               borrowed_ptr<php::Block>, const State&);

/*
 * Eliminate dead code in a function, across basic blocks, based on
 * results from a previous analyze_func call.
 */
void global_dce(const Index&, const FuncAnalysis&);

//////////////////////////////////////////////////////////////////////

const StaticString s_unreachable("static analysis error: supposedly "
                                 "unreachable code was reached");

//////////////////////////////////////////////////////////////////////

}}

#endif
