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

#include <compiler/expression/dynamic_variable.h>
#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/code_error.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/analysis/file_scope.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

DynamicVariable::DynamicVariable
(EXPRESSION_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES), m_exp(exp) {
}

ExpressionPtr DynamicVariable::clone() {
  DynamicVariablePtr exp(new DynamicVariable(*this));
  Expression::deepCopy(exp);
  exp->m_exp = Clone(m_exp);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void DynamicVariable::analyzeProgram(AnalysisResultPtr ar) {
  m_exp->analyzeProgram(ar);
}

ConstructPtr DynamicVariable::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int DynamicVariable::getKidCount() const {
  return 1;
}

void DynamicVariable::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

ExpressionPtr DynamicVariable::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_exp);
  return ExpressionPtr();
}

ExpressionPtr DynamicVariable::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_exp);
  return ExpressionPtr();
}

TypePtr DynamicVariable::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                    bool coerce) {
  ConstructPtr self = shared_from_this();
  if (m_context & (LValue | RefValue)) {
    if (ar->isFirstPass()) {
      ar->getCodeError()->record(self, CodeError::UseLDynamicVariable, self);
    }
    ar->getScope()->getVariables()->forceVariants(ar);
    ar->getScope()->
      getVariables()->setAttribute(VariableTable::ContainsLDynamicVariable);
  } else {
    if (ar->isFirstPass()) {
      ar->getCodeError()->record(self, CodeError::UseRDynamicVariable, self);
    }
  }

  m_exp->inferAndCheck(ar, Type::String, false);
  return m_implementedType = Type::Variant;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void DynamicVariable::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf("${");
  m_exp->outputPHP(cg, ar);
  cg.printf("}");
}

void DynamicVariable::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf("variables->get(");
  m_exp->outputCPP(cg, ar);
  cg.printf(")");
}
