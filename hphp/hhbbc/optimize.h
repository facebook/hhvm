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

#include "hphp/hhbbc/context.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

struct Index;
struct FuncAnalysis;
struct Bytecode;
struct BlockUpdateInfo;

using BlockUpdates = CompactVector<std::pair<BlockId, CompressedBlockUpdate>>;

/*
 * Use information from an analyze call to perform various
 * optimizations on a function.
 *
 * The Index should be unchanged since the one that was provided to
 * the corresponding analyze_func call.
 *
 * This routine may modify the php::Blocks attached to the passed-in
 * php::Func, and may renumber the php::Func's locals, but won't update
 * any of the func's other metadata.
 */
void optimize_func(const Index&, FuncAnalysis&&, php::WideFunc&);

enum class UpdateBCResult {
  None, // No changes
  Changed, // Changes
  ChangedAnalyze // Changes, and should analyze this function again
};
UpdateBCResult update_bytecode(php::WideFunc&,
                               BlockUpdates,
                               FuncAnalysis* = nullptr);

/*
 * Return a bytecode to generate the value in cell
 */
Bytecode gen_constant(const TypedValue& cell);

//////////////////////////////////////////////////////////////////////

}
