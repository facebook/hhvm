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
#ifndef incl_HPHP_HHIR_OPT_H_
#define incl_HPHP_HHIR_OPT_H_

#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

struct IRBuilder;
struct IRUnit;
struct IRInstruction;
struct FrameStateMgr;

//////////////////////////////////////////////////////////////////////

/*
 * The main optimization passes.
 */
void optimizeRefcounts2(IRUnit&);
void optimizePredictions(IRUnit&);
void hoistTypeChecks(IRUnit&);
void gvn(IRUnit&);
void optimizeLoads(IRUnit&);
void optimizeStores(IRUnit&);
void optimizeLoopInvariantCode(IRUnit&);
void cleanCfg(IRUnit&);

/*
 * For debugging, we can run this pass, which inserts various sanity checking
 * assertion instructions.
 */
void insertAsserts(IRUnit&);

/*
 * Run all the optimization passes.
 */
void optimize(IRUnit& unit, IRBuilder& builder, TransKind kind);

//////////////////////////////////////////////////////////////////////

}}

#endif
