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

#include <runtime/eval/ast/object_method_expression.h>
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

ObjectMethodExpression::ObjectMethodExpression(EXPRESSION_ARGS,
                                               ExpressionPtr obj, NamePtr name,
                                               std::vector<ExpressionPtr> param)
  : SimpleFunctionCallExpression(EXPRESSION_PASS, name, param), m_obj(obj) {}

Expression *ObjectMethodExpression::optimize(VariableEnvironment &env) {
  Eval::optimize(env, m_obj);
  SimpleFunctionCallExpression::optimize(env);
  return NULL;
}

Variant ObjectMethodExpression::eval(VariableEnvironment &env) const {
  String name(m_name->get(env));
  Variant obj(m_obj->eval(env));
  if (!obj.is(KindOfObject)) {
    throw_call_non_object(name.c_str());
  }
  Variant cobj(env.currentObject());
  const MethodStatement *ms = NULL;
  MethodStatementWrapper tmsw;
  const MethodStatementWrapper *msw = NULL;
  if (cobj.is(KindOfObject) && obj.getObjectData() == cobj.getObjectData()) {
    // Have to try current class first for private method
    const ClassStatement *cls = env.currentClassStatement();
    if (cls && !cls->isTrait()) {
      const MethodStatement *ccms = cls->findMethod(name);
      if (ccms && (ccms->getModifiers() & ClassStatement::Private) &&
          !ccms->getClass()->isTrait()) {
        ms = ccms;
        tmsw = MethodStatementWrapper(ms, ClassStatement::Private,
                                      ccms->getClass()->name().get());
        msw = &tmsw;
      }
    }
  }
  if (!ms) {
    msw = obj.getObjectData()->getMethodStatementWrapper(name);
    if (msw) ms = msw->m_methodStatement;
  }
  SET_LINE;
  if (ms) {
    return strongBind(ms->invokeInstanceDirect(toObject(obj), name, env, this,
                                               msw));
  }

  // Handle builtins
  MethodCallPackage mcp1;
  mcp1.methodCall(obj, name, -1);
  const CallInfo* ci = mcp1.ci;
  // If the lookup failed methodCall() must throw an exception,
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
    EvalFrameInjection::EvalStaticClassNameHelper helper(obj.toObject());
    return
      strongBind((ci->getMethFewArgs())(mcp1, count, a0, a1, a2, a3, a4, a5));
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

  EvalFrameInjection::EvalStaticClassNameHelper helper(obj.toObject());
  return strongBind((ci->getMeth())(mcp1, Array(ai.create())));
}

void ObjectMethodExpression::dump(std::ostream &out) const {
  m_obj->dump(out);
  out << "->";
  SimpleFunctionCallExpression::dump(out);
}

///////////////////////////////////////////////////////////////////////////////
}
}

