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

#include "hphp/compiler/statement/while_statement.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/block_scope.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

WhileStatement::WhileStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr condition, StatementPtr stmt)
  : LoopStatement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(WhileStatement)),
    m_condition(condition), m_stmt(stmt) {
}

StatementPtr WhileStatement::clone() {
  WhileStatementPtr stmt(new WhileStatement(*this));
  stmt->m_condition = Clone(m_condition);
  stmt->m_stmt = Clone(m_stmt);
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void WhileStatement::analyzeProgram(AnalysisResultPtr ar) {
  m_condition->analyzeProgram(ar);
  if (m_stmt) m_stmt->analyzeProgram(ar);
}

ConstructPtr WhileStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_condition;
    case 1:
      return m_stmt;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int WhileStatement::getKidCount() const {
  return 2;
}

void WhileStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_condition = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_stmt = dynamic_pointer_cast<Statement>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

void WhileStatement::inferTypes(AnalysisResultPtr ar) {
  m_condition->inferAndCheck(ar, Type::Boolean, false);
  if (m_stmt) {
    getScope()->incLoopNestedLevel();
    m_stmt->inferTypes(ar);
    getScope()->decLoopNestedLevel();
  }
}

///////////////////////////////////////////////////////////////////////////////

void WhileStatement::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("WhileStatement", m_stmt != nullptr ? 3 : 2);
  cg.printPropertyHeader("condition");
  m_condition->outputCodeModel(cg);
  if (m_stmt != nullptr) {
    cg.printPropertyHeader("block");
    cg.printAsBlock(m_stmt);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void WhileStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("while (");
  m_condition->outputPHP(cg, ar);
  cg_printf(") ");
  if (m_stmt) {
    m_stmt->outputPHP(cg, ar);
  } else {
    cg_printf("{}\n");
  }
}
