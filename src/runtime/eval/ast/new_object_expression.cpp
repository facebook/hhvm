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

#include <runtime/eval/ast/new_object_expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/ast/method_statement.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

NewObjectExpression::NewObjectExpression(EXPRESSION_ARGS, NamePtr name,
                      const std::vector<ExpressionPtr> &params)
  : FunctionCallExpression(EXPRESSION_PASS, params), m_name(name) {}

Variant NewObjectExpression::eval(VariableEnvironment &env) const {
  SET_LINE;
  String name(m_name->get(env));
  Object o(create_object(name.data(), Array(), false));
  o->init();
  o->dynConstructFromEval(env, this);
  return o;
}

void NewObjectExpression::dump() const {
  printf("new ");
  m_name->dump();
  dumpParams();
}

///////////////////////////////////////////////////////////////////////////////
}
}

