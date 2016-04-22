/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_JIT_IRGEN_ARITH_H_
#define incl_HPHP_JIT_IRGEN_ARITH_H_

#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/vm/hhbc.h"

namespace HPHP { namespace jit {

struct SSATmp;

namespace irgen {

struct IRGS;

/*
 * Return true iff we support compiling the given artihmetic operation with the
 * given types.
 */
bool areBinaryArithTypesSupported(Op op, Type lhs, Type rhs);

/*
 * If val->isA(TBool), return it converted to TInt. Otherwise, return val.
 */
SSATmp* promoteBool(IRGS& env, SSATmp* val);

/*
 * If either lhs or rhs is TDbl, make sure the other one is as well. Return the
 * hhir Opcode corresponding to the given hhbc Op and the final types of lhs
 * and rhs.
 */
Opcode promoteBinaryDoubles(IRGS& env, Op op, SSATmp*& lhs, SSATmp*& rhs);

/*
 * Query whether the given hhbc Op is a bitwise operation, or what the
 * corresponding hhir Opcode is.
 */
bool isBitOp(Op);
Opcode bitOp(Op);

}}}

#endif
