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

#include <runtime/eval/ast/qop_expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

QOpExpression::QOpExpression(EXPRESSION_ARGS, ExpressionPtr cond,
                             ExpressionPtr t, ExpressionPtr f)
  : Expression(EXPRESSION_PASS), m_cond(cond), m_true(t), m_false(f) {}

Variant QOpExpression::eval(VariableEnvironment &env) const {
  Variant cond(m_cond->eval(env));
  if (cond) {
    return m_true->eval(env);
  } else {
    return m_false->eval(env);
  }
}

void QOpExpression::dump() const {
  m_cond->dump();
  printf(" ? ");
  m_true->dump();
  printf(" : ");
  m_false->dump();
}

///////////////////////////////////////////////////////////////////////////////
}
}

