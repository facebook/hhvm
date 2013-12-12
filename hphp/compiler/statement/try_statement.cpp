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

#include "hphp/compiler/statement/try_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/analysis/function_scope.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

TryStatement::TryStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 StatementPtr tryStmt, StatementListPtr catches)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(TryStatement)),
    m_tryStmt(tryStmt), m_catches(catches) {
}

TryStatement::TryStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 StatementPtr tryStmt, StatementListPtr catches, StatementPtr finallyStmt)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(TryStatement)),
    m_tryStmt(tryStmt), m_catches(catches), m_finallyStmt(finallyStmt) {
}

StatementPtr TryStatement::clone() {
  TryStatementPtr stmt(new TryStatement(*this));
  stmt->m_tryStmt = Clone(m_tryStmt);
  if (m_catches)
    stmt->m_catches = Clone(m_catches);
  if (m_finallyStmt)
    stmt->m_finallyStmt = Clone(m_finallyStmt);
  return stmt;
}

int TryStatement::getRecursiveCount() const {
  return (m_tryStmt ? m_tryStmt->getRecursiveCount() : 0) +
    (m_catches->getCount() ? m_catches->getRecursiveCount() : 0) +
    (m_finallyStmt ? m_finallyStmt->getRecursiveCount() : 0);
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void TryStatement::analyzeProgram(AnalysisResultPtr ar) {
  if (m_tryStmt) m_tryStmt->analyzeProgram(ar);
  if (m_catches)
    m_catches->analyzeProgram(ar);
  if (m_finallyStmt)
    m_finallyStmt->analyzeProgram(ar);
  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    FunctionScopeRawPtr fs = getFunctionScope();
    if (fs) fs->setHasTry();
  }
}

bool TryStatement::hasDecl() const {
  if (m_tryStmt && m_tryStmt->hasDecl()) return true;
  if (m_catches && m_catches->hasDecl()) return true;
  if (m_finallyStmt && m_finallyStmt->hasDecl()) return true;
  return false;
}

bool TryStatement::hasRetExp() const {
  if (m_tryStmt && m_tryStmt->hasRetExp()) return true;
  if (m_catches && m_catches->hasRetExp()) return true;
  if (m_finallyStmt && m_finallyStmt->hasRetExp()) return true;
  return false;
}

ConstructPtr TryStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_tryStmt;
    case 1:
      return m_catches;
    case 2:
      return m_finallyStmt;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int TryStatement::getKidCount() const {
  return 3;
}

void TryStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_tryStmt = dynamic_pointer_cast<Statement>(cp);
      break;
    case 1:
      m_catches = dynamic_pointer_cast<StatementList>(cp);
      break;
    case 2:
      m_finallyStmt = dynamic_pointer_cast<Statement>(cp);
    default:
      assert(false);
      break;
  }
}

void TryStatement::inferTypes(AnalysisResultPtr ar) {
  if (m_tryStmt) m_tryStmt->inferTypes(ar);
  if (m_catches) m_catches->inferTypes(ar);
  if (m_finallyStmt) m_finallyStmt->inferTypes(ar);
}

///////////////////////////////////////////////////////////////////////////////

void TryStatement::outputCodeModel(CodeGenerator &cg) {
  auto numProps = 1;
  if (m_tryStmt != nullptr) numProps++;
  if (m_catches->getCount() > 0) numProps++;
  if (m_finallyStmt != nullptr) numProps++;
  cg.printObjectHeader("TryStatement", numProps);
  if (m_tryStmt != nullptr) {
    cg.printPropertyHeader("block");
    cg.printAsBlock(m_tryStmt);
  }
  if (m_catches->getCount() > 0)  {
    cg.printPropertyHeader("catchStatements");
    cg.printStatementVector(m_catches);
  }
  if (m_finallyStmt != nullptr) {
    cg.printPropertyHeader("finallyStatement");
    m_finallyStmt->outputCodeModel(cg);
  }
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void TryStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_indentBegin("try {\n");
  if (m_tryStmt) m_tryStmt->outputPHP(cg, ar);
  cg_indentEnd("}");
  if (m_catches->getCount())
    m_catches->outputPHP(cg, ar);
  cg_printf("\n");
}
