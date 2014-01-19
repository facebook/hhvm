/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
    Also set m_noStatic, to prevent errors in code gen due to
    m_className being set.
  */
  m_class.reset();
  m_noStatic = true;
}

ExpressionPtr NewObjectExpression::clone() {
  NewObjectExpressionPtr exp(new NewObjectExpression(*this));
  FunctionCall::deepCopy(exp);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void NewObjectExpression::analyzeProgram(AnalysisResultPtr ar) {
  FunctionCall::analyzeProgram(ar);

  if (ar->getPhase() == AnalysisResult::AnalyzeAll ||
      ar->getPhase() == AnalysisResult::AnalyzeFinal) {
    FunctionScopePtr func;
    if (!m_name.empty()) {
      addUserClass(ar, m_name);
      if (ClassScopePtr cls = resolveClass()) {
        m_name = m_className;
        func = cls->findConstructor(ar, true);
        if (func) func->addNewObjCaller(getScope());
      }
    }

    if (m_params) {
      markRefParams(func, "", canInvokeFewArgs());
    }

    if (ar->getPhase() == AnalysisResult::AnalyzeFinal) {
      TypePtr at(getActualType());
      if (at && at->isSpecificObject() && !getExpectedType()) {
        setExpectedType(at);
      }
    }
  }
}

TypePtr NewObjectExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                        bool coerce) {
  reset();
  m_classScope.reset();
  FunctionScopePtr prev = m_funcScope;
  m_funcScope.reset();
  ConstructPtr self = shared_from_this();
  if (!m_name.empty() && !isStatic()) {
    ClassScopePtr cls = resolveClassWithChecks();
    m_name = m_className;
    if (!cls) {
      if (m_params) m_params->inferAndCheck(ar, Type::Any, false);
      return Type::Object;
    }

    if (getScope()->isFirstPass() &&
        (cls->isTrait() ?
         !isSelf() && !isParent() :
         cls->isInterface() || cls->isAbstract())) {
      Compiler::Error(Compiler::InvalidInstantiation, self);
    }

    if (cls->isVolatile() && !isPresent()) {
      getScope()->getVariables()->
        setAttribute(VariableTable::NeedGlobalPointer);
    }
    m_dynamic = cls->derivesFromRedeclaring();
    bool valid = true;
    FunctionScopePtr func = cls->findConstructor(ar, true);
    if (!func) {
      if (m_params) {
        if (!m_dynamic && m_params->getCount()) {
          if (getScope()->isFirstPass()) {
            Compiler::Error(Compiler::BadConstructorCall, self);
          }
        }
        m_params->inferAndCheck(ar, Type::Some, false);
      }
    } else {
      if (func != prev) func->addNewObjCaller(getScope());
      m_extraArg = func->inferParamTypes(ar, self, m_params, valid);
      m_variableArgument = func->isVariableArgument();
    }
    if (valid) {
      m_classScope = cls;
      m_funcScope = func;
    }
    if (!valid || m_dynamic) {
      m_implementedType = Type::Object;
    } else {
      m_implementedType.reset();
    }
    return Type::CreateObjectType(m_name);
  } else {
    if (m_params) {
      m_params->markParams(canInvokeFewArgs());
    }
  }

  m_implementedType.reset();
  m_nameExp->inferAndCheck(ar, Type::String, false);
  if (m_params) m_params->inferAndCheck(ar, Type::Any, false);
  return Type::Object;
}

///////////////////////////////////////////////////////////////////////////////

void NewObjectExpression::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("NewObjectExpression", m_params == nullptr ? 2 : 3);
  if (m_nameExp->is(Expression::KindOfScalarExpression)) {
    cg.printPropertyHeader("className");
  } else {
    cg.printPropertyHeader("classExpression");
  }
  m_nameExp->outputCodeModel(cg);
  if (m_params != nullptr) {
    cg.printPropertyHeader("arguments");
    cg.printExpressionVector(m_params);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
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
