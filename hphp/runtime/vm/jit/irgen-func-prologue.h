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

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/types.h"

#include <cstdint>

namespace HPHP { namespace jit {

struct SSATmp;

namespace irgen {

struct IRGS;

///////////////////////////////////////////////////////////////////////////////

void emitGenericsMismatchCheck(IRGS& env, const Func* callee,
                               SSATmp* callFlags);

void emitCalleeDynamicCallCheck(IRGS& env, const Func* callee,
                                SSATmp* callFlags);

void emitImplicitContextCheck(IRGS& env, const Func* callee);

void emitPrologueLocals(IRGS& env, const Func* callee, uint32_t argc,
                        SSATmp* callFlags, SSATmp* closureOpt);

void emitFuncPrologue(IRGS& env, const Func* callee, uint32_t argc,
                      TransID transID);

///////////////////////////////////////////////////////////////////////////////

}}}

