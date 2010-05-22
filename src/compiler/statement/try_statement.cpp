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

#include <compiler/statement/try_statement.h>
#include <compiler/statement/statement_list.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

TryStatement::TryStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 StatementPtr tryStmt, StatementListPtr catches)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES),
    m_tryStmt(tryStmt), m_catches(catches) {
}

StatementPtr TryStatement::clone() {
  TryStatementPtr stmt(new TryStatement(*this));
  stmt->m_tryStmt = Clone(m_tryStmt);
  stmt->m_catches = Clone(m_catches);
  return stmt;
}

int TryStatement::getRecursiveCount() const {
  return (m_tryStmt ? m_tryStmt->getRecursiveCount() : 0) +
    (m_catches ? m_catches->getRecursiveCount() : 0);
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void TryStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  if (m_tryStmt) m_tryStmt->analyzeProgram(ar);
  m_catches->analyzeProgram(ar);
}

bool TryStatement::hasDecl() const {
  if (m_tryStmt && m_tryStmt->hasDecl()) return true;
  return m_catches->hasDecl();
}

bool TryStatement::hasRetExp() const {
  if (m_tryStmt && m_tryStmt->hasRetExp()) return true;
  return m_catches->hasRetExp();
}

ConstructPtr TryStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_tryStmt;
    case 1:
      return m_catches;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int TryStatement::getKidCount() const {
  return 2;
}

void TryStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_tryStmt = boost::dynamic_pointer_cast<Statement>(cp);
      break;
    case 1:
      m_catches = boost::dynamic_pointer_cast<StatementList>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

StatementPtr TryStatement::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_tryStmt);
  ar->preOptimize(m_catches);
  return StatementPtr();
}

StatementPtr TryStatement::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_tryStmt);
  ar->postOptimize(m_catches);
  return StatementPtr();
}

void TryStatement::inferTypes(AnalysisResultPtr ar) {
  if (m_tryStmt) m_tryStmt->inferTypes(ar);
  m_catches->inferTypes(ar);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void TryStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.indentBegin("try {\n");
  if (m_tryStmt) m_tryStmt->outputPHP(cg, ar);
  cg.indentEnd("}");
  m_catches->outputPHP(cg, ar);
  cg.printf("\n");
}

void TryStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  ASSERT(m_catches->getCount());

  cg.indentBegin("try {\n");
  if (m_tryStmt) m_tryStmt->outputCPP(cg, ar);
  cg.indentEnd("}");
  cg.indentBegin(" catch (Object e) {\n");
  for (int i = 0; i < m_catches->getCount(); i++) {
    if (i > 0) cg.printf(" else ");
    (*m_catches)[i]->outputCPP(cg, ar);
  }
  cg.printf(" else {\n");
  cg.printf("  throw;\n");
  cg.printf("}\n");
  cg.indentEnd("}\n");
}
