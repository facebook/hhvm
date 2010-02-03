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

#include <cpp/eval/ast/expr_statement.h>
#include <cpp/eval/ast/expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

ExprStatement::ExprStatement(STATEMENT_ARGS, ExpressionPtr exp)
  : Statement(STATEMENT_PASS), m_exp(exp) {}

void ExprStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  m_exp->eval(env);
}

void ExprStatement::dump() const {
  m_exp->dump();
  printf(";");
}

///////////////////////////////////////////////////////////////////////////////
}
}

