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

#include <compiler/statement/block_statement.h>
#include <compiler/statement/statement_list.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

BlockStatement::BlockStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, StatementListPtr stmts)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES), m_stmts(stmts) {
}

StatementPtr BlockStatement::clone() {
  BlockStatementPtr stmt(new BlockStatement(*this));
  stmt->m_stmts = Clone(m_stmts);
  return stmt;
}

int BlockStatement::getRecursiveCount() const {
  return m_stmts->getRecursiveCount();
}
///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void BlockStatement::analyzeProgramImpl(AnalysisResultPtr ar) {
  if (m_stmts) m_stmts->analyzeProgram(ar);
}

bool BlockStatement::hasDecl() const {
  return m_stmts && m_stmts->hasDecl();
}

bool BlockStatement::hasImpl() const {
  return m_stmts && m_stmts->hasImpl();
}

bool BlockStatement::hasRetExp() const {
  return m_stmts && m_stmts->hasRetExp();
}

ConstructPtr BlockStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_stmts;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int BlockStatement::getKidCount() const {
  return 1;
}

void BlockStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_stmts = boost::dynamic_pointer_cast<StatementList>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

StatementPtr BlockStatement::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_stmts);
  return StatementPtr();
}

StatementPtr BlockStatement::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_stmts);
  return StatementPtr();
}

void BlockStatement::inferTypes(AnalysisResultPtr ar) {
  if (m_stmts) m_stmts->inferTypes(ar);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void BlockStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.indentBegin("{\n");
  if (m_stmts) m_stmts->outputPHP(cg, ar);
  cg.indentEnd("}\n");
}

void BlockStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_stmts) {
    cg.indentBegin("{\n");
    m_stmts->outputCPP(cg, ar);
    cg.indentEnd("}\n");
  }
}
