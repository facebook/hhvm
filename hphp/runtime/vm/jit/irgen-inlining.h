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

namespace HPHP {

struct Func;

namespace jit {

struct SSATmp;

namespace irgen {

struct IRGS;

///////////////////////////////////////////////////////////////////////////////

/*
 * Emit a return from an inlined function.
 */
void retFromInlined(IRGS&);

/*
 * Exit the (now suspended) inline frame. The frame must no longer be live, and
 * its contents must now reside in waithandle.
 */
void suspendFromInlined(IRGS&, SSATmp* waithandle);

/*
 * Side exit the translation from an inlined frame to the specified target.
 */
void sideExitFromInlined(IRGS&, SrcKey target);

/*
 * Emit an EndCatch equivalent from an inlined function.
 */
bool endCatchFromInlined(IRGS&);

/*
 * Make sure all inlined frames are written on the stack and a part of the FP
 * chain. Returns true iff any frames were spilled.
 */
bool spillInlinedFrames(IRGS& env);

/*
 * Construct a FP that can be used to inline callee. Must be used while the
 * FCall bytecode is being translated.
 */
SSATmp* genCalleeFP(IRGS& env, const Func* callee);

///////////////////////////////////////////////////////////////////////////////

}}}

