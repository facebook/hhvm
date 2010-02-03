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

#include <cpp/eval/ast/function_call_expression.h>
#include <cpp/eval/ast/lval_expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

FunctionCallExpression::FunctionCallExpression(EXPRESSION_ARGS,
                                               const std::vector<ExpressionPtr>
                                               &params)
  : Expression(EXPRESSION_PASS), m_params(params) {}

Array FunctionCallExpression::getParams(VariableEnvironment &env) const {
  Array params;
  for (std::vector<ExpressionPtr>::const_iterator it = m_params.begin();
       it != m_params.end(); ++it) {
    params.append(ref((*it)->refval(env)));
  }
  return params;
}

void FunctionCallExpression::dumpParams() const {
  printf("(");
  dumpVector(m_params, ", ");
  printf(")");
}

///////////////////////////////////////////////////////////////////////////////
}
}

