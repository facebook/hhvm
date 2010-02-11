/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <cpp/eval/ast/lval_expression.h>
#include <cpp/eval/parser/hphp.tab.hpp>
#include <cpp/eval/bytecode/bytecode.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

LvalExpression::LvalExpression(EXPRESSION_ARGS) : Expression(EXPRESSION_PASS)
{}

Variant LvalExpression::set(VariableEnvironment &env, CVarRef val) const {
  return lval(env) = val;
}

Variant LvalExpression::setOpVariant(Variant &lhs, int op, CVarRef rhs) const {
  switch (op) {
  case T_PLUS_EQUAL:
    lhs += rhs;
    break;
  case T_MINUS_EQUAL:
    lhs -= rhs;
    break;
  case T_MUL_EQUAL:
    lhs *= rhs;
    break;
  case T_DIV_EQUAL:
    lhs /= rhs;
    break;
  case T_CONCAT_EQUAL:
    concat_assign(lhs, rhs);
    break;
  case T_MOD_EQUAL:
    lhs %= rhs;
    break;
  case T_AND_EQUAL:
    lhs &= rhs;
    break;
  case T_OR_EQUAL:
    lhs |= rhs;
    break;
  case T_XOR_EQUAL:
    lhs ^= rhs;
    break;
  case T_SL_EQUAL:
    lhs <<= rhs;
    break;
  case T_SR_EQUAL:
    lhs >>= rhs;
    break;
  default:
    ASSERT(false);
  }
  return lhs;
}

Variant LvalExpression::setOp(VariableEnvironment &env, int op, CVarRef rhs)
  const {
  Variant &lhs = lval(env);
  return setOpVariant(lhs, op, rhs);
}

void LvalExpression::unset(VariableEnvironment &env) const {
  lval(env).unset();
}

Variant LvalExpression::refval(VariableEnvironment &env) const {
  return ref(lval(env));
}

const LvalExpression *LvalExpression::toLval() const {
  return this;
}

void LvalExpression::byteCodeLval(ByteCodeProgram &code) const {
  throw FatalErrorException("Cannot compile %s:%d", m_loc.file, m_loc.line1);
}

void LvalExpression::byteCodeSet(ByteCodeProgram &code) const {
  byteCodeLval(code);
  code.add(ByteCode::Bind);
}

///////////////////////////////////////////////////////////////////////////////
}
}

