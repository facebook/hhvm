/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/eval/ast/lval_expression.h>
#include <util/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

LvalExpression::LvalExpression(EXPRESSION_ARGS) : Expression(EXPRESSION_PASS)
{}

bool LvalExpression::weakLval(VariableEnvironment &env, Variant* &v) const {
  v = &lval(env);
  return true;
}

Variant LvalExpression::set(VariableEnvironment &env, CVarRef val) const {
  return lval(env).assignVal(val);
}

Variant LvalExpression::setRef(VariableEnvironment &env, CVarRef val) const {
  return lval(env).assignRef(val);
}

Variant LvalExpression::setOpVariant(Variant &lhs, int op, CVarRef rhs) const {
  switch (op) {
  case T_PLUS_EQUAL:
    return AssignOp<T_PLUS_EQUAL>::assign(lhs, rhs);
  case T_MINUS_EQUAL:
    return AssignOp<T_MINUS_EQUAL>::assign(lhs, rhs);
  case T_MUL_EQUAL:
    return AssignOp<T_MUL_EQUAL>::assign(lhs, rhs);
  case T_DIV_EQUAL:
    return AssignOp<T_DIV_EQUAL>::assign(lhs, rhs);
  case T_CONCAT_EQUAL:
    return AssignOp<T_CONCAT_EQUAL>::assign(lhs, rhs);
  case T_MOD_EQUAL:
    return AssignOp<T_MOD_EQUAL>::assign(lhs, rhs);
  case T_AND_EQUAL:
    return AssignOp<T_AND_EQUAL>::assign(lhs, rhs);
  case T_OR_EQUAL:
    return AssignOp<T_OR_EQUAL>::assign(lhs, rhs);
  case T_XOR_EQUAL:
    return AssignOp<T_XOR_EQUAL>::assign(lhs, rhs);
  case T_SL_EQUAL:
    return AssignOp<T_SL_EQUAL>::assign(lhs, rhs);
  case T_SR_EQUAL:
    return AssignOp<T_SR_EQUAL>::assign(lhs, rhs);
  case T_INC:
    return AssignOp<T_INC>::assign(lhs, rhs);
  case T_DEC:
    return AssignOp<T_DEC>::assign(lhs, rhs);
  default:
    ASSERT(false);
  }
  return null;
}

Variant LvalExpression::setOp(VariableEnvironment &env, int op, CVarRef rhs)
  const {
  Variant &lhs = lval(env);
  return setOpVariant(lhs, op, rhs);
}

void LvalExpression::unset(VariableEnvironment &env) const {
  lval(env).unset();
}

Variant LvalExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  return strongBind(lval(env));
}

const LvalExpression *LvalExpression::toLval() const {
  return this;
}

///////////////////////////////////////////////////////////////////////////////
}
}

