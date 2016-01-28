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
#include "hphp/compiler/expression/class_expression.h"

#include <folly/ScopeGuard.h>

#include "hphp/compiler/statement/class_statement.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

ClassExpression::ClassExpression(
    EXPRESSION_CONSTRUCTOR_PARAMETERS,
    ClassStatementPtr cls,
    ExpressionListPtr params)
  : FunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ClassExpression),
      nullptr, "", false, params, nullptr)
  , m_cls(cls)
{
}

void ClassExpression::analyzeProgram(AnalysisResultPtr ar) {
  m_cls->analyzeProgram(ar);
  FunctionCall::analyzeProgram(ar);
}

ExpressionPtr ClassExpression::clone() {
  ClassExpressionPtr exp(new ClassExpression(*this));
  Expression::deepCopy(exp);
  exp->m_cls = Clone(m_cls);
  return exp;
}

void ClassExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  // XXX(jezng): I'm not sure how to test this, so leaving unimplemented
  // for now
}

}
