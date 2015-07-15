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

#include "hphp/compiler/expression/object_method_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/statement/statement.h"
#include "hphp/util/hash.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/parser/parser.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ObjectMethodExpression::ObjectMethodExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr object, ExpressionPtr method, ExpressionListPtr params,
 bool nullsafe)
  : FunctionCall(
      EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ObjectMethodExpression),
      method, "", false, params, ExpressionPtr()),
    m_object(object),
    m_nullsafe(nullsafe),
    m_xhpGetAttr(false),
    m_bindClass(true) {
  m_object->setContext(Expression::ObjectContext);
  m_object->clearContext(Expression::LValue);
  m_object->clearContext(Expression::AccessContext);
}

ExpressionPtr ObjectMethodExpression::clone() {
  ObjectMethodExpressionPtr exp(new ObjectMethodExpression(*this));
  FunctionCall::deepCopy(exp);
  exp->m_object = Clone(m_object);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ObjectMethodExpression::analyzeProgram(AnalysisResultPtr ar) {
  FunctionCall::analyzeProgram(ar);
  m_object->analyzeProgram(ar);

  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    FunctionScopePtr func = m_funcScope;
    if (!func && m_object->isThis() && !m_origName.empty()) {
      ClassScopePtr cls = getClassScope();
      if (cls) {
        m_classScope = cls;
        func = cls->findFunction(ar, m_origName, true, true);
        if (func &&
            !cls->isInterface() &&
            !(func->isVirtual() &&
              (func->isAbstract() ||
               (func->hasOverride() &&
                cls->getAttribute(ClassScope::NotFinal))))) {
          m_funcScope = func;
          func->addCaller(getScope());
        }
      }
    }

    markRefParams(func, m_origName);
  }
}

ConstructPtr ObjectMethodExpression::getNthKid(int n) const {
  if (!n) return m_object;
  return FunctionCall::getNthKid(n);
}

void ObjectMethodExpression::setNthKid(int n, ConstructPtr cp) {
  if (!n) {
    m_object = dynamic_pointer_cast<Expression>(cp);
  } else {
    FunctionCall::setNthKid(n, cp);
  }
}

ExpressionPtr ObjectMethodExpression::preOptimize(AnalysisResultConstPtr ar) {
  return ExpressionPtr();
}

///////////////////////////////////////////////////////////////////////////////

void ObjectMethodExpression::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("ObjectMethodCallExpression",
      m_params == nullptr ? 3 : 4);
  cg.printPropertyHeader("object");
  m_object->outputCodeModel(cg);
  if (m_nameExp->is(Expression::KindOfScalarExpression)) {
    cg.printPropertyHeader("methodName");
  } else {
    cg.printPropertyHeader("methodExpression");
  }
  m_nameExp->outputCodeModel(cg);
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

void ObjectMethodExpression::outputPHP(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  m_object->outputPHP(cg, ar);
  cg_printf("->");
  if (m_nameExp->getKindOf() == Expression::KindOfScalarExpression) {
    m_nameExp->outputPHP(cg, ar);
  } else {
    cg_printf("{");
    m_nameExp->outputPHP(cg, ar);
    cg_printf("}");
  }
  cg_printf("(");
  m_params->outputPHP(cg, ar);
  cg_printf(")");
}
