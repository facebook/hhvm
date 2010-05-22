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

#include <runtime/eval/ast/return_statement.h>
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/lval_expression.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

ReturnStatement::ReturnStatement(STATEMENT_ARGS, ExpressionPtr value)
  : Statement(STATEMENT_PASS), m_value(value) {}

void ReturnStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  if (m_value) {
    if (env.refReturn()) {
      env.setRet(ref(m_value->refval(env)));
      return;
    }
    env.setRet(m_value->eval(env));
    return;
  }
  env.setRet();
}

void ReturnStatement::dump() const {
  printf("return");
  if (m_value) {
    printf(" ");
    m_value->dump();
  }
  printf(";");
}

///////////////////////////////////////////////////////////////////////////////
}
}

