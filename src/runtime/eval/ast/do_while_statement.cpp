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

#include <runtime/eval/ast/do_while_statement.h>
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DoWhileStatement::DoWhileStatement(STATEMENT_ARGS, StatementPtr body,
                                   ExpressionPtr cond)
  : Statement(STATEMENT_PASS), m_cond(cond), m_body(body) {}

void DoWhileStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  do {
    if (!m_body) continue;
    EVAL_STMT_HANDLE_BREAK(m_body, env);
 } while (m_cond->eval(env));
}

void DoWhileStatement::dump() const {
  printf("do {");
  if (m_body) m_body->dump();
  printf("} while(");
  m_cond->dump();
  printf(");");
}

///////////////////////////////////////////////////////////////////////////////
}
}

