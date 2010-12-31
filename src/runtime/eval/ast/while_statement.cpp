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

#include <runtime/eval/ast/while_statement.h>
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

WhileStatement::WhileStatement(STATEMENT_ARGS, ExpressionPtr cond,
                               StatementPtr body)
  : Statement(STATEMENT_PASS), m_cond(cond), m_body(body) {}

void WhileStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  ENTER_STMT;
  DECLARE_THREAD_INFO;
  LOOP_COUNTER(1);
  while (m_cond->eval(env)) {
    LOOP_COUNTER_CHECK(1);
    if (!m_body) continue;
    EVAL_STMT_HANDLE_GOTO_BEGIN(restart);
    EVAL_STMT_HANDLE_BREAK(m_body, env);
    EVAL_STMT_HANDLE_GOTO_END(restart);
  }
}

void WhileStatement::dump(std::ostream &out) const {
  out << "while (";
  m_cond->dump(out);
  out << ") {";
  if (m_body) m_body->dump(out);
  out << "}\n";
}

///////////////////////////////////////////////////////////////////////////////
}
}

