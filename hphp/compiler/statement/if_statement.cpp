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

#include "hphp/compiler/statement/if_statement.h"
#include "hphp/compiler/statement/if_branch_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/statement/block_statement.h"
#include "hphp/compiler/analysis/function_scope.h"

#include "hphp/runtime/base/type-variant.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

IfStatement::IfStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS, StatementListPtr stmts)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(IfStatement)),
  m_stmts(stmts), m_hasCondCSE(false) {
}

StatementPtr IfStatement::clone() {
  IfStatementPtr stmt(new IfStatement(*this));
  stmt->m_stmts = Clone(m_stmts);
  return stmt;
}

int IfStatement::getRecursiveCount() const {
  return m_stmts->getRecursiveCount();
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

ConstructPtr IfStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_stmts;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int IfStatement::getKidCount() const {
  return 1;
}

void IfStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_stmts = dynamic_pointer_cast<StatementList>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void IfStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  for (int i = 0; i < m_stmts->getCount(); i++) {
    if (i > 0) cg_printf("else");
    (*m_stmts)[i]->outputPHP(cg, ar);
  }
}
