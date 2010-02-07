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

#include <cpp/eval/ast/assignment_op_expression.h>
#include <cpp/eval/ast/lval_expression.h>
#include <cpp/eval/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

AssignmentOpExpression::AssignmentOpExpression(EXPRESSION_ARGS, int op,
                                               LvalExpressionPtr lhs,
                                               ExpressionPtr rhs)
  : Expression(EXPRESSION_PASS), m_op(op), m_lhs(lhs), m_rhs(rhs) {}

Variant AssignmentOpExpression::eval(VariableEnvironment &env) const {
  Variant rhs = m_rhs->eval(env);
  if (m_op == '=') return m_lhs->set(env, rhs);
  return m_lhs->setOp(env, m_op, rhs);
}

void AssignmentOpExpression::dump() const {
  m_lhs->dump();
  const char* op = "<bad op>";
  switch (m_op) {
  case '=': op = "=";  break;
  case T_PLUS_EQUAL: op = "+="; break;
  case T_MINUS_EQUAL: op = "-="; break;
  case T_MUL_EQUAL: op = "*="; break;
  case T_DIV_EQUAL: op = "/="; break;
  case T_CONCAT_EQUAL: op = ".="; break;
  case T_MOD_EQUAL: op = "%="; break;
  case T_AND_EQUAL: op = "&="; break;
  case T_OR_EQUAL: op = "|="; break;
  case T_XOR_EQUAL: op = "^="; break;
  case T_SL_EQUAL: op = "<<="; break;
  case T_SR_EQUAL: op = ">>="; break;
  default:
    ASSERT(false);
  }
  printf(" %s ", op);
  m_rhs->dump();
}

///////////////////////////////////////////////////////////////////////////////
}
}

