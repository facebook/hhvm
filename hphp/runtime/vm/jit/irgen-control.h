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

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

namespace HPHP::jit {

struct Block;
struct SSATmp;

namespace irgen {

struct IRGS;

//////////////////////////////////////////////////////////////////////

/*
 * Returns an IR block corresponding to the given bytecode offset. If the block
 * starts with a DefLabel expecting a StkPtr, this function will return an
 * intermediate block that passes the current sp.
 */
Block* getBlock(IRGS& env, Offset offset);
Block* getBlock(IRGS& env, SrcKey sk);

/*
 * Helpers for unconditional and conditional jumps.
 */
void surpriseCheck(IRGS&);
void surpriseCheck(IRGS&, Offset);
void surpriseCheckWithTarget(IRGS&, Offset);
void jmpImpl(IRGS&, Offset);
void jmpImpl(IRGS&, SrcKey);
void implCondJmp(IRGS&, Offset taken, bool negate, SSATmp*);

/*
 * Route exception `exc' to the appropriate handler. C++ exceptions are
 * represented by TNullptr and must be ultimately handled by EndCatch, which
 * gives control back to the unwinder. Otherwise `exc' will contain an object
 * implementing the Throwable interface. Hack exceptions might be routed
 * directly to the appropriate handlers.
 */
void emitHandleException(IRGS& env, EndCatchData::CatchMode mode, SSATmp* exc,
                         Optional<IRSPRelOffset> vmspOffset);

//////////////////////////////////////////////////////////////////////

}}

