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

#include <lib/expression/new_object_expression.h>
#include <lib/expression/scalar_expression.h>
#include <lib/expression/expression_list.h>
#include <lib/analysis/code_error.h>
#include <lib/analysis/class_scope.h>
#include <lib/analysis/function_scope.h>
#include <lib/analysis/dependency_graph.h>
#include <lib/option.h>
#include <lib/analysis/variable_table.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

NewObjectExpression::NewObjectExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr variable, ExpressionListPtr params)
  : FunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES,
                 variable, "", params, NULL), m_redeclared(false) {
}

ExpressionPtr NewObjectExpression::clone() {
  NewObjectExpressionPtr exp(new NewObjectExpression(*this));
  FunctionCall::deepCopy(exp);
  exp->m_params = Clone(m_params);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void NewObjectExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (!m_name.empty()) {
    addUserClass(ar, m_name);
  }

  m_nameExp->analyzeProgram(ar);
  if (m_params) {
    m_params->controlOrder();
    m_params->analyzeProgram(ar);
  }
}

ExpressionPtr NewObjectExpression::preOptimize(AnalysisResultPtr ar) {
  return FunctionCall::preOptimize(ar);
}

ExpressionPtr NewObjectExpression::postOptimize(AnalysisResultPtr ar) {
  return FunctionCall::postOptimize(ar);
}

TypePtr NewObjectExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                        bool coerce) {
  reset();
  ConstructPtr self = shared_from_this();
  if (!m_name.empty()) {
    ClassScopePtr cls = ar->resolveClass(m_name);
    if (cls) {
      m_name = cls->getName();
    }
    if (!cls || cls->isRedeclaring()) {
      if (cls) {
        m_redeclared = true;
        ar->getScope()->getVariables()->
          setAttribute(VariableTable::NeedGlobalPointer);
      }
      if (!cls && ar->isFirstPass()) {
        ar->getCodeError()->record(self, CodeError::UnknownClass, self);
      }
      if (m_params) m_params->inferAndCheck(ar, NEW_TYPE(Any), false);
      return NEW_TYPE(Object);
    }

    m_validClass = true;
    FunctionScopePtr func = cls->findConstructor(ar, true);
    if (!func) {
      if (m_params && m_params->getCount()) {
        if (ar->isFirstPass()) {
          ar->getCodeError()->record(self, CodeError::BadConstructorCall,
                                     self);
        }
        m_params->inferAndCheck(ar, NEW_TYPE(Any), false);
        m_params->setOutputCount(0);
      }
    } else {
      m_extraArg = func->inferParamTypes(ar, self, m_params,
                                          m_validClass);
      m_variableArgument = func->isVariableArgument();
    }
    return Type::CreateObjectType(m_name);
  } else {
    ar->containsDynamicClass();
    if (ar->isFirstPass()) {
      ar->getCodeError()->record(self, CodeError::UseDynamicClass,
                                 self);
    }
    if (m_params) {
      for (int i = 0; i < m_params->getCount(); i++) {
        (*m_params)[i]->setContext(Expression::RefValue);
      }
    }
  }

  m_nameExp->inferAndCheck(ar, Type::String, false);
  if (m_params) m_params->inferAndCheck(ar, NEW_TYPE(Any), false);
  return Type::Variant;//NEW_TYPE(Object);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void NewObjectExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  outputLineMap(cg, ar);

  cg.printf("new ");
  m_nameExp->outputPHP(cg, ar);
  cg.printf("(");
  if (m_params) m_params->outputPHP(cg, ar);
  cg.printf(")");
}

void NewObjectExpression::outputCPPImpl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  bool linemap = outputLineMap(cg, ar);
  if (m_params) m_params->outputCPPControlledEvalOrderPre(cg, ar);
  if (!m_name.empty() && !m_redeclared && m_validClass) {
    bool tooManyArgs =
      (m_params && m_params->outputCPPTooManyArgsPre(cg, ar, m_name));
    cg.printf("%s%s(%s%s(NEWOBJ(%s%s)())->create(",
              Option::SmartPtrPrefix, m_name.c_str(),
              Option::SmartPtrPrefix, m_name.c_str(),
              Option::ClassPrefix, m_name.c_str());
    FunctionScope::outputCPPArguments(m_params, cg, ar, m_extraArg,
                                      m_variableArgument, m_argArrayId);
    cg.printf("))");
    if (tooManyArgs) {
      m_params->outputCPPTooManyArgsPost(cg, ar, m_voidReturn);
    }
  } else {
    if (m_redeclared) {
      cg.printf("g->%s%s->create(", Option::ClassStaticsObjectPrefix,
                m_name.c_str());
    } else {
      cg.printf("create_object(");
      if (!m_name.empty()) {
        cg.printf("\"%s\"", m_name.c_str());
      } else if (m_nameExp->is(Expression::KindOfSimpleVariable)) {
        m_nameExp->outputCPP(cg, ar);
      } else {
        cg.printf("(");
        m_nameExp->outputCPP(cg, ar);
        cg.printf(")");
      }
      cg.printf(", ");
    }
    if (m_params) {
      FunctionScope::outputCPPArguments(m_params, cg, ar, -1, false);
    } else {
      cg.printf("Array()");
    }
    cg.printf(")");
  }
  if (m_params) m_params->outputCPPControlledEvalOrderPost(cg, ar);
  if (linemap) cg.printf(")");
}
