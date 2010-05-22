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

#include <runtime/eval/ast/for_statement.h>
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

ForStatement::ForStatement(STATEMENT_ARGS,
                           const std::vector<ExpressionPtr> &init,
                           const std::vector<ExpressionPtr> &cond,
                           const std::vector<ExpressionPtr> &next,
                           StatementPtr body)
  : Statement(STATEMENT_PASS), m_init(init), m_cond(cond), m_next(next),
    m_body(body) {}

void ForStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  for (Expression::evalVector(m_init, env);
       m_cond.empty() ? true : (bool)Expression::evalVector(m_cond, env);
       Expression::evalVector(m_next, env)) {
    if (m_body) {
      EVAL_STMT_HANDLE_BREAK(m_body, env);
    }
  }
}

void ForStatement::dump() const {
  printf("for (");
  dumpVector(m_init, ", ");
  printf("; ");
  dumpVector(m_cond, ", ");
  printf("; ");
  dumpVector(m_next, ", ");
  printf(") {");
  if (m_body) m_body->dump();
  printf("}");

}

///////////////////////////////////////////////////////////////////////////////
}
}

