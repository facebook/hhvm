/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/expression/new_object_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/analysis/variable_table.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

NewObjectExpression::NewObjectExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr variable, ExpressionListPtr params)
  : FunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(NewObjectExpression),
                 variable, "", false, params, variable),
    m_dynamic(false) {
  /*
    StaticClassName takes care of parent & self properly, so
    we pass in variable as the class name.
    But NewObjectExpression is written to use the class as the
    function name, so clear it here, to take care of the dynamic
    case.
  */
  m_class.reset();
}

ExpressionPtr NewObjectExpression::clone() {
  NewObjectExpressionPtr exp(new NewObjectExpression(*this));
  FunctionCall::deepCopy(exp);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void NewObjectExpression::analyzeProgram(AnalysisResultPtr ar) {
  FunctionCall::analyzeProgram(ar);

  if (ar->getPhase() == AnalysisResult::AnalyzeAll ||
      ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    FunctionScopePtr func;
    if (!m_origName.empty()) {
      if (ClassScopePtr cls = resolveClass()) {
        m_origName = m_origClassName;
        func = cls->findConstructor(ar, true);
        if (func) func->addNewObjCaller(getScope());
      }
    }

    if (m_params) {
      markRefParams(func, "");
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void NewObjectExpression::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("NewObjectExpression", m_params == nullptr ? 2 : 3);
  if (m_nameExp->is(Expression::KindOfScalarExpression)) {
    cg.printPropertyHeader("class");
    cg.printTypeExpression(this->getOriginalName());
  } else {
    cg.printPropertyHeader("classExpression");
    m_nameExp->outputCodeModel(cg);
  }
  if (m_params != nullptr) {
    cg.printPropertyHeader("arguments");
    cg.printExpressionVector(m_params);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void NewObjectExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("new ");
  m_nameExp->outputPHP(cg, ar);
  cg_printf("(");
  if (m_params) m_params->outputPHP(cg, ar);
  cg_printf(")");
}
