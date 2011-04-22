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

#include <runtime/eval/ast/strong_foreach_statement.h>
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/lval_expression.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

StrongForEachStatement::
StrongForEachStatement(STATEMENT_ARGS, ExpressionPtr source,
                       LvalExpressionPtr key, LvalExpressionPtr value,
                       StatementPtr body) :
  Statement(STATEMENT_PASS), m_source(source), m_key(key), m_value(value),
  m_body(body)
{}

void StrongForEachStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;

  ENTER_STMT;
  LvalExpression* lvalSource = m_source->cast<LvalExpression>();
  Variant source(strongBind(lvalSource ?
                            lvalSource->lval(env) :
                            (CVarRef)m_source->eval(env)));
  source.escalate(true);
  Variant vTemp;

  if (m_key) {
    Variant kTemp;
    for (MutableArrayIter iter =
           source.begin(&kTemp, vTemp, env.currentContext());
         iter.advance();) {
      m_key->set(env, kTemp);
      m_value->set(env, ref(vTemp));
      if (!m_body) continue;
      EVAL_STMT_HANDLE_GOTO_BEGIN(restart1);
      EVAL_STMT_HANDLE_BREAK(m_body, env);
      EVAL_STMT_HANDLE_GOTO_END(restart1);
    }
  } else {
    for (MutableArrayIter iter =
           source.begin(NULL, vTemp, env.currentContext());
         iter.advance();) {
      m_value->set(env, ref(vTemp));
      if (!m_body) continue;
      EVAL_STMT_HANDLE_GOTO_BEGIN(restart2);
      EVAL_STMT_HANDLE_BREAK(m_body, env);
      EVAL_STMT_HANDLE_GOTO_END(restart2);
    }
  }
}

void StrongForEachStatement::dump(std::ostream &out) const {
  out << "foreach (";
  m_source->dump(out);
  out << " as ";
  if (m_key) {
    m_key->dump(out);
    out << " => ";
  }
  out << "&";
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

