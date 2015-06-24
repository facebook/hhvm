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

#include "hphp/compiler/statement/do_statement.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/block_scope.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

DoStatement::DoStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 StatementPtr stmt, ExpressionPtr condition)
  : LoopStatement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(DoStatement)),
    m_stmt(stmt), m_condition(condition) {
}

StatementPtr DoStatement::clone() {
  DoStatementPtr stmt(new DoStatement(*this));
  stmt->m_stmt = Clone(m_stmt);
  stmt->m_condition = Clone(m_condition);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void DoStatement::analyzeProgram(AnalysisResultPtr ar) {
  if (m_stmt) m_stmt->analyzeProgram(ar);
  m_condition->analyzeProgram(ar);
}

ConstructPtr DoStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_stmt;
    case 1:
      return m_condition;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int DoStatement::getKidCount() const {
  return 2;
}

void DoStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_stmt = dynamic_pointer_cast<Statement>(cp);
      break;
    case 1:
      m_condition = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

void DoStatement::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("DoStatement", 3);
  cg.printPropertyHeader("block");
  cg.printAsBlock(m_stmt);
  cg.printPropertyHeader("condition");
  m_condition->outputCodeModel(cg);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this);
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void DoStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("do ");
  if (m_stmt) {
    m_stmt->outputPHP(cg, ar);
  } else {
    cg_printf("{}\n");
  }
  cg_printf("while (");
  m_condition->outputPHP(cg, ar);
  cg_printf(");\n");
}
