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

#include <runtime/eval/ast/inc_op_expression.h>
#include <runtime/eval/ast/lval_expression.h>
#include <util/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

IncOpExpression::IncOpExpression(EXPRESSION_ARGS, LvalExpressionPtr exp,
                                 bool inc, bool front)
  : Expression(EXPRESSION_PASS), m_exp(exp), m_inc(inc), m_front(front) {}

Variant IncOpExpression::eval(VariableEnvironment &env) const {
  SET_LINE;
  return m_exp->setOp(env, m_inc ? T_INC : T_DEC, m_front ? null : Variant(0));
}

Variant IncOpExpression::refval(VariableEnvironment &env,
    int strict /* = 2 */) const {
  if (m_front) {
    return ref(eval(env));
  } else {
    return ref(Expression::refval(env, strict));
  }
}

void IncOpExpression::dump(std::ostream &out) const {
  if (m_front) {
    out << (m_inc ? "++" : "--");
  }
  m_exp->dump(out);
  if (!m_front) {
    out << (m_inc ? "++" : "--");
  }
}

///////////////////////////////////////////////////////////////////////////////
}

}

