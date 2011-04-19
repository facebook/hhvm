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

#include <runtime/eval/ast/foreach_statement.h>
#include <runtime/eval/ast/lval_expression.h>
#include <runtime/eval/ast/temp_expression_list.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

ForEachStatement::ForEachStatement(STATEMENT_ARGS, ExpressionPtr source,
                                   LvalExpressionPtr key,
                                   LvalExpressionPtr value, StatementPtr body)
  :  Statement(STATEMENT_PASS), m_source(source), m_key(key), m_value(value),
     m_body(body) {}

void ForEachStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  ENTER_STMT;
  DECLARE_THREAD_INFO;
  LOOP_COUNTER(1);
  Variant map(m_source->eval(env));
  if (m_key) {
    TempExpressionList *texp = m_key->cast<TempExpressionList>();
    if (texp) {
      for (ArrayIter iter = map.begin(env.currentContext(), true);
           !iter.end(); iter.next()) {
        {
          LOOP_COUNTER_CHECK_INFO(1);
          const Variant &value = iter.second();
          const Variant &key = iter.first();
          TempExpressionHelper helper(texp, env);
          m_value->set(env, value);
          texp->setImpl(env, key);
        }
        if (!m_body) continue;
        EVAL_STMT_HANDLE_GOTO_BEGIN(restart1);
        EVAL_STMT_HANDLE_BREAK(m_body, env);
        EVAL_STMT_HANDLE_GOTO_END(restart1);
      }
    } else {
      for (ArrayIter iter = map.begin(env.currentContext(), true);
           !iter.end(); iter.next()) {
        LOOP_COUNTER_CHECK_INFO(1);
        const Variant &value = iter.second();
        const Variant &key = iter.first();
        m_value->set(env, value);
        m_key->set(env, key);
        if (!m_body) continue;
        EVAL_STMT_HANDLE_GOTO_BEGIN(restart2);
        EVAL_STMT_HANDLE_BREAK(m_body, env);
        EVAL_STMT_HANDLE_GOTO_END(restart2);
      }
    }
  } else {
    for (ArrayIter iter = map.begin(env.currentContext(), true);
         !iter.end(); iter.next()) {
      LOOP_COUNTER_CHECK_INFO(1);
      m_value->set(env, iter.second());
      if (!m_body) continue;
      EVAL_STMT_HANDLE_GOTO_BEGIN(restart3);
      EVAL_STMT_HANDLE_BREAK(m_body, env);
      EVAL_STMT_HANDLE_GOTO_END(restart3);
    }
  }
}

void ForEachStatement::dump(std::ostream &out) const {
  out << "foreach (";
  m_source->dump(out);
  out << " as ";
  if (m_key) {
    m_key->dump(out);
    out << " => ";
  }
  m_value->dump(out);
  out << ") {";
  if (m_body) {
    m_body->dump(out);
  }
  out << "}\n";
}

///////////////////////////////////////////////////////////////////////////////
}
}

