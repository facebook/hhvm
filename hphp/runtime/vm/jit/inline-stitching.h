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

#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/ir-builder.h"

namespace HPHP {

namespace jit {

namespace irgen {

struct InlineStitchingContext {
   IRUnit& callerUnit;
   IRUnit& calleeUnit;
   IRBuilder* irb{};
   SrcKey callerSk;
   SrcKey calleeSk;
   SSATmp* fp{};
   SSATmp* sp{};
   SSATmp* objOrClass{};
   // TODO: remove this
   irgen::IRGS& env;
};

/**
 * Stitch callee's IRUnit into caller's IRUnit.
 */
void stitchCalleeUnit(InlineStitchingContext& ctx);
///////////////////////////////////////////////////////////////////////////////

}}}
