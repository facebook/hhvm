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

#include "hphp/compiler/statement/unset_statement.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/statement/block_statement.h"
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

UnsetStatement::UnsetStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionListPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(UnsetStatement)),
    m_exp(exp) {
  exp->setContext(Expression::UnsetContext);
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
      assert(false);
      break;
  }
  return ConstructPtr();
}

int UnsetStatement::getKidCount() const {
  return 1;
}

void UnsetStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = dynamic_pointer_cast<ExpressionList>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

StatementPtr UnsetStatement::preOptimize(AnalysisResultConstPtr ar) {
  if (m_exp->getCount() == 0) return NULL_STATEMENT();
  return StatementPtr();
}

///////////////////////////////////////////////////////////////////////////////

void UnsetStatement::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("UnsetStatement", 2);
  cg.printPropertyHeader("expressions");
  cg.printExpressionVector(m_exp);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void UnsetStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("unset(");
  m_exp->outputPHP(cg, ar);
  cg_printf(");\n");
}
