/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/compiler/expression/class_constant_expression.h"
#include <set>
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/util/hash.h"
#include "hphp/util/text-util.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/constant_expression.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

// constructors/destructors

ClassConstantExpression::ClassConstantExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr classExp, const std::string &varName)
  : Expression(
      EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ClassConstantExpression)),
    StaticClassName(classExp), m_varName(varName),
    m_originalScopeSet(false) {
}

ExpressionPtr ClassConstantExpression::clone() {
  auto exp = std::make_shared<ClassConstantExpression>(*this);
  Expression::deepCopy(exp);
  exp->m_class = Clone(m_class);
  exp->m_originalScopeSet = true;
  exp->m_originalScope = m_originalScope ? m_originalScope : getScope();
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

ClassScopeRawPtr ClassConstantExpression::getOriginalClassScope() const {
  auto scope = m_originalScopeSet ? m_originalScope : getScope();
  return scope ? scope->getContainingClass() : ClassScopeRawPtr();
}

void ClassConstantExpression::analyzeProgram(AnalysisResultConstRawPtr ar) {
  if (!m_class && ar->getPhase() >= AnalysisResult::AnalyzeAll) {
    resolveClass();
  }
}

ConstructPtr ClassConstantExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_class;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int ClassConstantExpression::getKidCount() const {
  return 1;
}

void ClassConstantExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_class = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ClassConstantExpression::outputPHP(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  cg_printf("\\");
  StaticClassName::outputPHP(cg, ar);
  cg_printf("::%s", m_varName.c_str());
}
