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

/* The Peephole Optimizer
 * ======================
 *
 * The peephole optimizer performs some postprocessing on bytecode. Currently we
 * perform the following optimizations:
 *
 * - Jump rewriting: if we jump to a jump, we rewrite the original jump to just
 *   directly jump to the final destination. We can handle long chains of jumps
 *   and are careful about infinite loops and conditional jumps.
 * - PostInc/PostDec to PreInc/PreDec conversion: PostInc and PostDec are
 *   necessarily slower than PreInc and PreDec, so when possible we convert
 *   PostInc/PostDec to PreInc/PreDec.
 * - Conditional jump conversion: if a JmpZ uses an item from the stack which
 *   was placed there by a Not, we remove the Not and replace the JmpZ with
 *   JmpNZ. We likewise convert Not + JmpNZ to JmpZ.
 */

#ifndef incl_HPHP_COMPILER_ANALYSIS_PEEPHOLE_H_
#define incl_HPHP_COMPILER_ANALYSIS_PEEPHOLE_H_

#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/func.h"

namespace HPHP { namespace Compiler {

class MetaInfoBuilder;

class Peephole {
public:
  Peephole(UnitEmitter& ue, MetaInfoBuilder& metaInfo);

private:
  void buildFuncTargets(FuncEmitter* fe);
  void buildJumpTargets();

  UnitEmitter& m_ue;
  hphp_hash_set<Offset> m_jumpTargets;
};

}}

#endif
