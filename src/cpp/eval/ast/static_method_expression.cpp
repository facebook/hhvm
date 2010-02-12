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

#include <cpp/eval/ast/static_method_expression.h>
#include <cpp/eval/ast/name.h>
#include <cpp/eval/runtime/variable_environment.h>
#include <cpp/eval/runtime/eval_state.h>
#include <cpp/eval/ast/method_statement.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

StaticMethodExpression::
StaticMethodExpression(EXPRESSION_ARGS, const string &cname,
                       NamePtr &name,
                       const vector<ExpressionPtr> &params) :
  SimpleFunctionCallExpression(EXPRESSION_PASS, name, params), m_cname(cname),
  m_construct(name->getStatic() == "__construct") {}

Variant StaticMethodExpression::eval(VariableEnvironment &env) const {
  SET_LINE;
  // Static method expressions can be object method expressions inside
  // of a method when an object is available and the object's class inherits.
  // Super slow.
  String name = m_name->get(env);
  Object co = env.currentObject();
  bool withinClass = !co.isNull() && co->o_instanceof(m_cname.data());
  bool foundClass;
  const MethodStatement *ms = RequestEvalState::findMethod(m_cname.data(),
                                                           name.data(),
                                                           foundClass);
  if (withinClass) {
    if (m_construct && !ms) {
      // In a class method doing __construct will go to the name constructor
      ms = RequestEvalState::findMethod(m_cname.data(),
                                        m_cname.data(),
                                        foundClass);
    }
    if (ms) {
      return ref(ms->invokeInstanceDirect(co, env, this));
    }
    return ref(co->o_invoke_ex(m_cname.data(), name.data(), getParams(env),
                               m_name->hashLwr()));

  }
  if (ms) {
    return ref(ms->invokeStaticDirect(m_cname.data(), env, this));
  }
  return ref(invoke_static_method(m_cname.data(), name.data(), getParams(env)));
}

void StaticMethodExpression::dump() const {
  printf("%s::", m_cname.c_str());
  SimpleFunctionCallExpression::dump();
}

///////////////////////////////////////////////////////////////////////////////
}
}

