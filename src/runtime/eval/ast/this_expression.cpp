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

#include <runtime/eval/ast/this_expression.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

ThisExpression::ThisExpression(EXPRESSION_ARGS)
  : LvalExpression(EXPRESSION_PASS) {}

Variant &ThisExpression::lval(VariableEnvironment &env) const {
  if (!env.currentObject().is(KindOfObject)) {
    SET_LINE;
    raise_error("Using $this when not in an object context");
  }
  return env.currentObject();
}

Variant ThisExpression::set(VariableEnvironment &env, CVarRef val) const {
  throw FatalErrorException("Cannot re-assign $this");
}

void ThisExpression::dump() const {
  printf("$this");
}

///////////////////////////////////////////////////////////////////////////////
}
}

