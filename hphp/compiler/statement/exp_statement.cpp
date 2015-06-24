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

#include "hphp/compiler/statement/exp_statement.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/include_expression.h"
#include "hphp/compiler/option.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/compiler/expression/function_call.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/file_scope.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ExpStatement::ExpStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, ExpressionPtr exp)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(ExpStatement)),
    m_exp(exp) {
}

StatementPtr ExpStatement::clone() {
  ExpStatementPtr stmt(new ExpStatement(*this));
  stmt->m_exp = Clone(m_exp);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void ExpStatement::onParse(AnalysisResultConstPtr ar, FileScopePtr scope) {
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void ExpStatement::analyzeProgram(AnalysisResultPtr ar) {
  m_exp->analyzeProgram(ar);
}

ConstructPtr ExpStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int ExpStatement::getKidCount() const {
  return 1;
}

void ExpStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

StatementPtr ExpStatement::preOptimize(AnalysisResultConstPtr ar) {
  assert (ar->getPhase() > AnalysisResult::AnalyzeAll);
  m_exp = m_exp->unneeded();
  return StatementPtr();
}

///////////////////////////////////////////////////////////////////////////////

void ExpStatement::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("ExpressionStatement", 2);
  cg.printPropertyHeader("expression");
  m_exp->outputCodeModel(cg);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ExpStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_exp->outputPHP(cg, ar);
  cg_printf(";\n");
}

bool ExpStatement::shouldEmitStatement() const {
  return hasEffect() || Option::KeepStatementsWithNoEffect;
}
