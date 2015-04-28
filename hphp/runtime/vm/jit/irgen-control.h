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
#ifndef incl_HPHP_JIT_IRGEN_CONTROL_H_
#define incl_HPHP_JIT_IRGEN_CONTROL_H_

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/jmpflags.h"

namespace HPHP { namespace jit {
struct IRGS;
struct Block;
struct SSATmp;
}}

namespace HPHP { namespace jit { namespace irgen {

//////////////////////////////////////////////////////////////////////

/*
 * Returns an IR block corresponding to the given bytecode offset. If the block
 * starts with a DefLabel expecting a StkPtr, this function will return an
 * intermediate block that passes the current sp.
 */
Block* getBlock(IRGS& env, Offset offset);

/*
 * Helpers for unconditional and conditional jumps.
 */
void surpriseCheck(IRGS&, Offset);
void jmpImpl(IRGS&, Offset, JmpFlags);
void implCondJmp(IRGS&, Offset taken, bool negate, SSATmp*);

//////////////////////////////////////////////////////////////////////

}}}

#endif
