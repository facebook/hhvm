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

#include <runtime/eval/ast/temp_expression.h>
#include <runtime/eval/ast/temp_expression_list.h>
#include <runtime/eval/ast/variable_expression.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TempExpression::TempExpression(ExpressionPtr exp, int index /* = 0 */)
    : Expression(exp->loc()), m_exp(exp), m_index(index) {
}

Variant TempExpression::eval(VariableEnvironment &env) const {
  if (m_exp->is<VariableExpression>()) {
    return m_exp->eval(env);
  }
  return env.getTempVariable(m_index);
}

void TempExpression::dump(std::ostream &out) const {
  m_exp->dump(out);
}

///////////////////////////////////////////////////////////////////////////////
}
}

