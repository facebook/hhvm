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

Variant ObjectMethodExpression::eval(VariableEnvironment &env) const {
  String name(m_name->get(env));
  Variant obj(m_obj->eval(env));
  if (!obj.is(KindOfObject)) {
    raise_error("Call to a member function %s() on a non-object",
                name.c_str());
  }
  Variant cobj(env.currentObject());
  const MethodStatement *ms = NULL;
  if (cobj.is(KindOfObject) && obj.getObjectData() == cobj.getObjectData()) {
    // Have to try current class first for private method
    const ClassStatement *cls = env.currentClassStatement();
    if (cls) {
      const MethodStatement *ccms = cls->findMethod(name.c_str());
      if (ccms && ccms->getModifiers() & ClassStatement::Private) {
        ms = ccms;
      }
    }
  }
  if (!ms) {
    ms = obj.getObjectData()->getMethodStatement(name.data());
  }
  SET_LINE;
  if (ms) {
    return ref(ms->invokeInstanceDirect(toObject(obj), env, this));
  }
  return ref(obj.getObjectData()->o_invoke_from_eval(name.data(), env, this,
                                                     m_name->hashLwr(), true));
}

void ObjectMethodExpression::dump() const {
  m_obj->dump();
  printf("->");
  SimpleFunctionCallExpression::dump();
}

///////////////////////////////////////////////////////////////////////////////
}
}

