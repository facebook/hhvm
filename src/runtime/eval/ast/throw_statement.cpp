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

#include <runtime/eval/ast/throw_statement.h>
#include <runtime/eval/ast/expression.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

ThrowStatement::ThrowStatement(STATEMENT_ARGS, ExpressionPtr value)
  : Statement(STATEMENT_PASS), m_value(value) {}

void ThrowStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  throw m_value->eval(env).toObject();
}

void ThrowStatement::dump() const {
  printf("throw ");
  m_value->dump();
  printf(";");
}

///////////////////////////////////////////////////////////////////////////////
}
}

