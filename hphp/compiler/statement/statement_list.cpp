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

#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/statement/exp_statement.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/class_statement.h"
#include "hphp/compiler/statement/function_statement.h"
#include "hphp/compiler/statement/return_statement.h"
#include "hphp/compiler/statement/block_statement.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/expression/include_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

StatementList::StatementList
(STATEMENT_CONSTRUCTOR_PARAMETERS)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(StatementList)) {
}

StatementPtr StatementList::clone() {
  StatementListPtr stmt(new StatementList(*this));
  stmt->m_stmts.clear();
  for (unsigned int i = 0; i < m_stmts.size(); i++) {
    stmt->m_stmts.push_back(Clone(m_stmts[i]));
  }
  return stmt;
}

StatementListPtr StatementList::shallowClone() {
  StatementListPtr stmt(new StatementList(*this));
  stmt->m_stmts.clear();
  for (unsigned int i = 0; i < m_stmts.size(); i++) {
    stmt->m_stmts.push_back(m_stmts[i]);
  }
  return stmt;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

void StatementList::addElement(StatementPtr stmt) {
  m_stmts.push_back(stmt);
}

void StatementList::insertElement(StatementPtr stmt, int index /* = 0 */) {
  assert(index >= 0 && index <= (int)m_stmts.size());
  m_stmts.insert(m_stmts.begin() + index, stmt);
}

void StatementList::removeElement(int index) {
  m_stmts.erase(m_stmts.begin() + index, m_stmts.begin() + index + 1);
}

void StatementList::shift(int from, int to) {
  assert(from >= 0 && from <= (int)m_stmts.size());
  assert(to >= 0 && to <= (int)m_stmts.size());
  StatementPtr stmt = m_stmts[from];
  for (int i = from; i < to; i++) {
    m_stmts[i] = m_stmts[i+1];
  }
  m_stmts[to] = stmt;
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

StatementPtr StatementList::operator[](int index) {
  assert(index >= 0 && index < getCount());
  return m_stmts[index];
}

ConstructPtr StatementList::getNthKid(int n) const {
  if (n < (int)m_stmts.size()) {
    return m_stmts[n];
  }
  return ConstructPtr();
}

int StatementList::getKidCount() const {
  return m_stmts.size();
}

void StatementList::setNthKid(int n, ConstructPtr cp) {
  int s = m_stmts.size();
  if (n >= s) {
    assert(false);
  } else {
    m_stmts[n] = dynamic_pointer_cast<Statement>(cp);
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void StatementList::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  for (unsigned int i = 0; i < m_stmts.size(); i++) {
    StatementPtr stmt = m_stmts[i];

    switch (cg.getContext()) {
    case CodeGenerator::NoContext:
      stmt->outputPHP(cg, ar);
      break;
    case CodeGenerator::PhpDeclaration:
      if (stmt->is(Statement::KindOfFunctionStatement) ||
          stmt->is(Statement::KindOfClassStatement) ||
          stmt->is(Statement::KindOfInterfaceStatement)) {
        cg.setContext(CodeGenerator::PhpImplementation);
        stmt->outputPHP(cg, ar);
        cg.setContext(CodeGenerator::PhpDeclaration);
      }
      break;
    case CodeGenerator::PhpImplementation:
      if (!stmt->is(Statement::KindOfFunctionStatement) &&
          !stmt->is(Statement::KindOfClassStatement) &&
          !stmt->is(Statement::KindOfInterfaceStatement)) {
        stmt->outputPHP(cg, ar);
      }
      break;
    default:
      assert(false);
    }
  }
}
