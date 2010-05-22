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

#include <runtime/eval/ast/statement_list_statement.h>
#include <runtime/eval/runtime/variable_environment.h>

using namespace std;

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

StatementListStatement::
StatementListStatement(STATEMENT_ARGS, const std::vector<StatementPtr> &stmts)
  : Statement(STATEMENT_PASS), m_stmts(stmts) {}
StatementListStatement::StatementListStatement(STATEMENT_ARGS)
  : Statement(STATEMENT_PASS) {}

void StatementListStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  for (vector<StatementPtr>::const_iterator it = m_stmts.begin();
       it != m_stmts.end(); ++it) {
    EVAL_STMT(*it, env);
  }
}

void StatementListStatement::add(StatementPtr stmt) {
  m_stmts.push_back(stmt);
}

void StatementListStatement::dump() const {
  dumpVector(m_stmts, " ");
}

void StatementListStatement::byteCode(ByteCodeProgram &code) const {
  for (vector<StatementPtr>::const_iterator it = m_stmts.begin();
       it != m_stmts.end(); ++it) {
    (*it)->byteCode(code);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}

