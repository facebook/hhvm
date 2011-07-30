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

#include <runtime/eval/ast/static_method_expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/ast/class_statement.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

StaticMethodExpression::
StaticMethodExpression(EXPRESSION_ARGS, const NamePtr &cname,
    const NamePtr &name, const vector<ExpressionPtr> &params) :
  SimpleFunctionCallExpression(EXPRESSION_PASS, name, params), m_cname(cname),
  m_construct(name->get() == "__construct") {}

Variant StaticMethodExpression::eval(VariableEnvironment &env) const {
  SET_LINE;
  // Static method expressions can be object method expressions inside
  // of a method when an object is available and the object's class inherits.
  // Super slow.
  String cname = m_cname->get(env);
  bool sp = m_cname->isSp();
  String name(m_name->get(env));
  Variant &vco = env.currentObject();
  Object co;
  if (!vco.isNull()) co = vco.toObject();
  bool withinClass = !co.isNull() && co->o_instanceof(cname.data());
  bool foundClass;
  const MethodStatement *ms = RequestEvalState::findMethod(cname,
                                                           name.data(),
                                                           foundClass);
  if (withinClass) {
    if (m_construct) {
      String name = cname;
      while (true) {
        ClassEvalState *ces = RequestEvalState::findClassState(name.data());
        if (!ces) {
          // possibly built in
          cname = name;
          break;
        }
        // Ugly but needed to populate the method table for the parent
        ces->initializeInstance();
        ms = ces->getConstructor();
        if (ms) break;
        name = ces->getClass()->parent().c_str();
        if (name.empty()) break;
      }
    }
    if (!ms) {
      Array params = getParams(env);
      EvalFrameInjection::EvalStaticClassNameHelper helper(cname, sp);
      return strongBind(co->o_invoke_ex(cname, name, params));
    } else if (!(ms->getModifiers() & ClassStatement::Static)) {
      EvalFrameInjection::EvalStaticClassNameHelper helper(cname, sp);
      return strongBind(ms->invokeInstanceDirect(co, env, this));
    }
  }
  if (ms) {
    return strongBind(ms->invokeStaticDirect(cname, env, this, sp));
  }

  // Handle builtins
  MethodCallPackage mcp1;
  mcp1.dynamicNamedCall(cname, name, -1);
  const CallInfo* ci = mcp1.ci;
  // If the lookup failed dynamicNamedCall() must throw an exception,
  // so if we reach here ci must not be NULL
  ASSERT(ci);
  unsigned int count = m_params.size();
  if (count <= 6) {
    CVarRef a0 = (count > 0) ? evalParam(env, ci, 0) : null;
    CVarRef a1 = (count > 1) ? evalParam(env, ci, 1) : null;
    CVarRef a2 = (count > 2) ? evalParam(env, ci, 2) : null;
    CVarRef a3 = (count > 3) ? evalParam(env, ci, 3) : null;
    CVarRef a4 = (count > 4) ? evalParam(env, ci, 4) : null;
    CVarRef a5 = (count > 5) ? evalParam(env, ci, 5) : null;
    return
      strongBind((ci->getMethFewArgs())(mcp1, count, a0, a1, a2, a3, a4, a5));
  }
  if (RuntimeOption::UseArgArray) {
    ArgArray *args = prepareArgArray(env, ci, count);
    return strongBind((ci->getMeth())(mcp1, args));
  }
  ArrayInit ai(count);
  for (unsigned int i = 0; i < count; ++i) {
    if (ci->mustBeRef(i)) {
      ai.setRef(m_params[i]->refval(env));
    } else if (ci->isRef(i)) {
      ai.setRef(m_params[i]->refval(env, 0));
    } else {
      ai.set(m_params[i]->eval(env));
    }
  }
  return strongBind((ci->getMeth())(mcp1, Array(ai.create())));
}

void StaticMethodExpression::dump(std::ostream &out) const {
  m_cname->dump(out);
  out << "::";
  SimpleFunctionCallExpression::dump(out);
}

///////////////////////////////////////////////////////////////////////////////
}
}

