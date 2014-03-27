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

#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"

namespace HPHP {  namespace JIT {

TRACE_SET_MOD(hhir);

ArgDesc::ArgDesc(SSATmp* tmp, const PhysLoc& loc, bool val)
  : m_zeroExtend(false), m_done(false) {
  if (tmp->isConst()) {
    // tmp is a constant
    m_srcReg = InvalidReg;
    if (val) {
      m_imm64 = tmp->type() <= Type::Null ? 0 : tmp->rawVal();
    } else {
      m_imm64 = toDataTypeForCall(tmp->type());
    }
    m_kind = Kind::Imm;
    return;
  }
  if (val) {
    assert(loc.reg(0) != InvalidReg);
    m_srcReg = loc.reg(0);
    m_kind = Kind::Reg;
    // zero extend any boolean value that we pass to the helper in case
    // the helper expects it (e.g., as TypedValue)
    if (tmp->isA(Type::Bool)) m_zeroExtend = true;
    return;
  }
  if (tmp->numWords() > 1) {
    assert(loc.reg(1) != InvalidReg);
    m_srcReg = loc.reg(1);
    // Since val is false then we're passing tmp's type. TypeReg lets
    // CodeGenerator know that the value might require some massaging
    // to be in the right format for the call.
    m_kind = Kind::TypeReg;
    return;
  }
  // arg is the (constant) type of a known-typed value.
  m_srcReg = InvalidReg;
  m_imm64 = toDataTypeForCall(tmp->type());
  m_kind = Kind::Imm;
}

}}
