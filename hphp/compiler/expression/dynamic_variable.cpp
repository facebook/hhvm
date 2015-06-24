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

#include "hphp/compiler/expression/dynamic_variable.h"
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/code_model_enums.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

DynamicVariable::DynamicVariable
(EXPRESSION_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(DynamicVariable)),
    m_exp(exp) {
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
      assert(false);
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
      m_exp = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

void DynamicVariable::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("UnaryOpExpression", 3);
  cg.printPropertyHeader("expression");
  m_exp->outputCodeModel(cg);
  cg.printPropertyHeader("operation");
  cg.printValue(PHP_DYNAMIC_VARIABLE_OP) ;
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void DynamicVariable::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("${");
  m_exp->outputPHP(cg, ar);
  cg_printf("}");
}
