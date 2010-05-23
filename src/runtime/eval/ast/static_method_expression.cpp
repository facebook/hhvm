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

#include <runtime/eval/ast/static_method_expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/ast/method_statement.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

StaticMethodExpression::
StaticMethodExpression(EXPRESSION_ARGS, const NamePtr &cname, bool sp,
    const NamePtr &name, const vector<ExpressionPtr> &params) :
  SimpleFunctionCallExpression(EXPRESSION_PASS, name, params), m_cname(cname),
  m_sp(sp), m_construct(name->getStatic() == "__construct") {}

Variant StaticMethodExpression::eval(VariableEnvironment &env) const {
  SET_LINE;
  // Static method expressions can be object method expressions inside
  // of a method when an object is available and the object's class inherits.
  // Super slow.
  String cname = getClassName(m_cname, env);
  FrameInjection::EvalStaticClassNameHelper helper
    (m_cname->getStatic(), cname, m_sp);

  String name(m_name->get(env));
  Variant &vco = env.currentObject();
  Object co;
  if (!vco.isNull()) co = vco.toObject();
  bool withinClass = !co.isNull() && co->o_instanceof(cname.data());
  bool foundClass;
  const MethodStatement *ms = RequestEvalState::findMethod(cname.data(),
                                                           name.data(),
                                                           foundClass);
  if (withinClass) {
    if (m_construct && !ms) {
      // In a class method doing __construct will go to the name constructor
      ms = RequestEvalState::findMethod(cname.data(),
                                        cname.data(),
                                        foundClass);
    }
    if (ms) {
      return ref(ms->invokeInstanceDirect(co, env, this));
    }
    return ref(co->o_invoke_ex(cname.data(), name.data(), getParams(env),
                               m_name->hashLwr()));

  }
  if (ms) {
    return ref(ms->invokeStaticDirect(cname.data(), env, this));
  }
  return ref(invoke_static_method(cname.data(), name.data(), getParams(env)));
}

void StaticMethodExpression::dump() const {
  m_cname->dump();
  printf("::");
  SimpleFunctionCallExpression::dump();
}

///////////////////////////////////////////////////////////////////////////////
}
}

