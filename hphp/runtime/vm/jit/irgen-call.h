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
#ifndef incl_HPHP_JIT_IRGEN_CALL_H_
#define incl_HPHP_JIT_IRGEN_CALL_H_

#include <cstdint>

#include "hphp/runtime/vm/hhbc.h"
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

IRSPRelOffset fsetActRec(
  IRGS& env,
  SSATmp* func,
  SSATmp* objOrClass,
  uint32_t numArgs,
  const StringData* invName,
  bool dynamicCall,
  SSATmp* tsList
);

void emitDirectCall(IRGS& env, Func* callee, uint32_t numParams,
                    SSATmp* const* const args);

void emitCallerDynamicCallChecks(IRGS& env,
                                 const Func* callee,
                                 IRSPRelOffset actRecOff);
void emitCallerDynamicConstructChecks(IRGS& env, SSATmp* cls);
void emitCallerRxChecks(IRGS& env, const Func* callee, IRSPRelOffset actRecOff);

Type callReturnType(const Func* callee);
Type awaitedCallReturnType(const Func* callee);

//////////////////////////////////////////////////////////////////////

}}}

#endif
