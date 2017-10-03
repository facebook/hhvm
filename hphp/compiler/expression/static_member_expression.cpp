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

#include "hphp/compiler/expression/static_member_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/dynamic_variable.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/util/text-util.h"
#include "hphp/util/hash.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/compiler/option.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

StaticMemberExpression::StaticMemberExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr classExp, ExpressionPtr exp)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(StaticMemberExpression)),
    StaticClassName(classExp), m_exp(exp) {
  if (exp->is(KindOfSimpleVariable)) {
    auto s = dynamic_pointer_cast<SimpleVariable>(exp);
    m_exp = ExpressionPtr
      (new ScalarExpression(getScope(), getRange(),
                            T_STRING, s->getName(), true));

  } else {
    always_assert(exp->is(KindOfDynamicVariable));
    m_exp = dynamic_pointer_cast<DynamicVariable>(exp)->getSubExpression();
  }
}

ExpressionPtr StaticMemberExpression::clone() {
  StaticMemberExpressionPtr exp(new StaticMemberExpression(*this));
  Expression::deepCopy(exp);
  if (m_class) exp->m_class = m_class->clone();
  exp->m_exp = m_exp->clone();
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions
void StaticMemberExpression::analyzeProgram(AnalysisResultConstRawPtr ar) {
  resolveClass();
}

ConstructPtr StaticMemberExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_class;
    case 1:
      return m_exp;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int StaticMemberExpression::getKidCount() const {
  return 2;
}

void StaticMemberExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_class = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_exp = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void StaticMemberExpression::outputPHP(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  StaticClassName::outputPHP(cg, ar);

  cg_printf("::$");
  bool needsClose = false;
  switch (m_exp->getKindOf()) {
  case KindOfScalarExpression:
    {
      auto var = dynamic_pointer_cast<ScalarExpression>(m_exp);
      cg_printf("%s", var->getString().c_str());
      return;
    }
  case KindOfSimpleVariable:
  case KindOfDynamicVariable:
    break;
  default:
    cg_printf("{");
    needsClose = true;
  }
  m_exp->outputPHP(cg, ar);
  if (needsClose) {
    cg_printf("}");
  }
}
