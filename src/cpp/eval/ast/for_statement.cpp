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

#include <cpp/eval/ast/for_statement.h>
#include <cpp/eval/ast/expression.h>
#include <cpp/eval/runtime/variable_environment.h>
#include <cpp/eval/bytecode/bytecode.h>

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
    EVAL_STMT_HANDLE_BREAK(m_body, env);
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

void ForStatement::byteCode(ByteCodeProgram &code) const {
  Expression::byteCodeEvalVector(m_init, code);
  code.add(ByteCode::Discard);
  ByteCodeProgram::Label preCond = code.here();
  ByteCodeProgram::JumpTag fail = 0;
  if (!m_cond.empty()) {
    Expression::byteCodeEvalVector(m_cond, code);
    fail = code.jumpIfNot();
  }
  m_body->byteCode(code);
  Expression::byteCodeEvalVector(m_next, code);
  code.add(ByteCode::Discard);
  code.bindJumpTag(code.jump(), preCond);
  if (!m_cond.empty()) {
    code.bindJumpTag(fail);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}

