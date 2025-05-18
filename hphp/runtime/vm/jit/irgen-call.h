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

#include <cstdint>

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/module.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/minstr-helpers.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"

namespace HPHP {

struct StringData;
struct Func;

namespace jit {

struct Type;
struct SSATmp;

namespace irgen {

struct IRGS;

//////////////////////////////////////////////////////////////////////

/*
 * Lock the object on top of the stack if we just unwound a constructor
 * frame called using FCallCtor with the LockWhileUnwinding flag.
 */
void emitLockObjOnFrameUnwind(IRGS& env, PC pc);

Type callReturnType(const Func* callee);
Type awaitedCallReturnType(const Func* callee);
Type callOutType(const Func* callee, uint32_t index);

/*
 * Emits instructions to check and enforce module boundary violations
 */
void emitModuleBoundaryCheck(IRGS&, SSATmp* symbol, bool func = true);

void emitModuleBoundaryCheckKnown(IRGS&, const Class* symbol);
void emitModuleBoundaryCheckKnown(IRGS&, const Func* symbol);
void emitModuleBoundaryCheckKnown(IRGS&, const Class::Prop* symbol);
void emitModuleBoundaryCheckKnown(IRGS&, const Class::SProp* symbol);
//////////////////////////////////////////////////////////////////////


}}}
