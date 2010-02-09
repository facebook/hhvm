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

#include <lib/statement/unset_statement.h>
#include <lib/expression/expression_list.h>
#include <lib/analysis/code_error.h>
#include <lib/analysis/analysis_result.h>
#include <lib/analysis/variable_table.h>
#include <lib/expression/simple_variable.h>
#include <lib/analysis/block_scope.h>
#include <lib/expression/array_element_expression.h>
#include <lib/expression/scalar_expression.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

UnsetStatement::UnsetStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionListPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES), m_exp(exp) {
  for (int i = 0; i < m_exp->getCount(); i++) {
    ExpressionPtr exp = (*m_exp)[i];
    exp->setContext(Expression::UnsetContext);
    exp->setContext(Expression::LValue);
    exp->setContext(Expression::NoLValueWrapper);
  }
}

StatementPtr UnsetStatement::clone() {
  UnsetStatementPtr stmt(new UnsetStatement(*this));
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void UnsetStatement::analyzeProgram(AnalysisResultPtr ar) {
  m_exp->analyzeProgram(ar);
}

ConstructPtr UnsetStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      return ConstructPtr();
  }
  ASSERT(0);
}

int UnsetStatement::getKidCount() const {
  return 1;
}

int UnsetStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = boost::dynamic_pointer_cast<ExpressionList>(cp);
      return 1;
    default:
      return 0;
  }
  ASSERT(0);
}

StatementPtr UnsetStatement::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_exp);
  return StatementPtr();
}

StatementPtr UnsetStatement::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_exp);
  return StatementPtr();
}

void UnsetStatement::inferTypes(AnalysisResultPtr ar) {
  m_exp->inferAndCheck(ar, Type::Variant, true);
  ConstructPtr self = shared_from_this();
  VariableTablePtr variables = ar->getScope()->getVariables();
  for (int i = 0; i < m_exp->getCount(); i++) {
    ExpressionPtr exp = (*m_exp)[i];
    if (ar->isFirstPass() &&
        !exp->is(Expression::KindOfSimpleVariable) &&
        !exp->is(Expression::KindOfArrayElementExpression)) {
      ar->getCodeError()->record(self, CodeError::UseNotSupportedUnset, exp);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void UnsetStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf("unset(");
  m_exp->outputPHP(cg, ar);
  cg.printf(");\n");
}

void UnsetStatement::outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_exp->getCount() > 1) cg.indentBegin("{\n");
  for (int i = 0; i < m_exp->getCount(); i++) {
    (*m_exp)[i]->outputCPPUnset(cg, ar);
  }
  if (m_exp->getCount() > 1) cg.indentEnd("}\n");
}
