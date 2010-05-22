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

#include <runtime/eval/ast/foreach_statement.h>
#include <runtime/eval/ast/lval_expression.h>
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
  ENTER_STMT;
  Variant map(m_source->eval(env));
  Variant &v = m_value->lval(env);
  Variant *k = NULL;
  if (m_key) {
    k = &m_key->lval(env);
  }
  for (ArrayIterPtr iter = map.begin(env.currentContext()); !iter->end();
       iter->next()) {
    v = iter->second();
    if (k) *k = iter->first();
    if (!m_body) continue;
    EVAL_STMT_HANDLE_BREAK(m_body, env);
  }
}

void ForEachStatement::dump() const {
  printf("foreach (");
  m_source->dump();
  printf(" as ");
  if (m_key) {
    m_key->dump();
    printf(" => ");
  }
  m_value->dump();
  printf(") {");
  m_body->dump();
  printf("}");
}

///////////////////////////////////////////////////////////////////////////////
}
}

