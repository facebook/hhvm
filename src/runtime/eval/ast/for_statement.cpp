/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/for_statement.h>
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

static Variant evalVector(const std::vector<ExpressionPtr> &v,
                          VariableEnvironment &env) {
  ASSERT(v.size() != 1);

  for (unsigned int i  = 0; i < v.size(); i++) {
    CVarRef res = v[i]->eval(env);
    if (i == v.size() - 1) return res;
  }
  return null;
}

Statement *ForStatement::optimize(VariableEnvironment &env) {
  for (unsigned int i = 0; i < m_init.size(); i++) {
    Eval::optimize(env, m_init[i]);
  }
  for (unsigned int i = 0; i < m_cond.size(); i++) {
    Eval::optimize(env, m_cond[i]);
  }
  for (unsigned int i = 0; i < m_next.size(); i++) {
    Eval::optimize(env, m_next[i]);
  }
  Eval::optimize(env, m_body);
  return NULL;
}

void ForStatement::eval(VariableEnvironment &env) const {
  DECLARE_THREAD_INFO;

  if (env.isGotoing()) {
    if (env.isLimitedGoto()) return;
    goto body;
  }
  ENTER_STMT;
  LOOP_COUNTER(1);

  if (LIKELY(m_init.size() == 1)) {
    m_init[0]->eval(env);
  } else {
    evalVector(m_init, env);
  }

  begin:
  if (LIKELY(m_cond.size() == 1)) {
    if (!m_cond[0]->eval(env)) return;
  } else if (m_cond.size() == 0 || evalVector(m_cond, env)) {
    goto body;
  } else {
    return;
  }
  body:
  LOOP_COUNTER_CHECK_INFO(1);
  EVAL_STMT_HANDLE_GOTO_BEGIN(restart);
  if (m_body) EVAL_STMT_HANDLE_GOTO(m_body, env);
  EVAL_STMT_HANDLE_GOTO_END(restart);

  if (LIKELY(m_next.size() == 1)) {
    m_next[0]->eval(env);
  } else {
    evalVector(m_next, env);
  }
  goto begin;
  end:;
}

void ForStatement::dump(std::ostream &out) const {
  out << "for (";
  dumpVector(out, m_init);
  out << "; ";
  dumpVector(out, m_cond);
  out << "; ";
  dumpVector(out, m_next);
  out << ") {";
  if (m_body) m_body->dump(out);
  out << "}\n";
}

///////////////////////////////////////////////////////////////////////////////
}
}

