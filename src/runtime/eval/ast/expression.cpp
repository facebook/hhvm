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
#include <runtime/eval/ast/lval_expression.h>
#include <runtime/eval/ast/name.h>
#include <util/parser/hphp.tab.hpp>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

Expression::Expression(EXPRESSION_ARGS) : Construct(CONSTRUCT_PASS) {}

Variant Expression::evalVector(const std::vector<ExpressionPtr> &v,
                               VariableEnvironment &env) {
  Variant res;
  for (std::vector<ExpressionPtr>::const_iterator it = v.begin();
       it != v.end(); ++it) {
    res = (*it)->eval(env);
  }
  return res;
}

Variant Expression::refval(VariableEnvironment &env, int strict /* = 2 */)
  const {
  if (strict == 2) {
    raise_error("Value cannot be used in reference context");
  } else if (strict == 1) {
    raise_notice("Value cannot be used in reference context");
  }
  return ref(eval(env));
}

bool Expression::exist(VariableEnvironment &env, int op) const {
  if (op == T_ISSET) {
    return HPHP::isset(evalExist(env));
  }
  ASSERT(op == T_EMPTY);
  return HPHP::empty(evalExist(env));
}

Variant Expression::evalExist(VariableEnvironment &env) const {
  return eval(env);
}

const LvalExpression *Expression::toLval() const {
  return NULL;
}

bool Expression::isRefParam() const {
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
}
