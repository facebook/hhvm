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

#include <runtime/eval/ast/new_object_expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

static StaticString s_trait_marker("[trait]");

NewObjectExpression::NewObjectExpression(EXPRESSION_ARGS, NamePtr name,
                      const std::vector<ExpressionPtr> &params)
  : FunctionCallExpression(EXPRESSION_PASS, params), m_name(name) {}

Expression *NewObjectExpression::optimize(VariableEnvironment &env) {
  Eval::optimize(env, m_name);
  FunctionCallExpression::optimize(env);
  return NULL;
}

Variant NewObjectExpression::eval(VariableEnvironment &env) const {
  String name(m_name->get(env));
  if (name.same(s_trait_marker)) {
    name = ClassStatement::resolveSpInTrait(env, Object(), m_name.get());
  }
  Object o(create_object_only(name));
  SET_LINE;

  const MethodStatement* ms = o.get()->getConstructorStatement();
  if (ms) {
    ms->invokeInstanceDirect(o, env, this);
    return o;
  }

  // Handle builtins
  MethodCallPackage mcp1;
  mcp1.construct(o);
  const CallInfo* ci = mcp1.ci;
  ASSERT(ci);
  unsigned int count = m_params.size();
  // few args
  if (count <= 6) {
    CVarRef a0 = (count > 0) ? evalParam(env, ci, 0) : null;
    CVarRef a1 = (count > 1) ? evalParam(env, ci, 1) : null;
    CVarRef a2 = (count > 2) ? evalParam(env, ci, 2) : null;
    CVarRef a3 = (count > 3) ? evalParam(env, ci, 3) : null;
    CVarRef a4 = (count > 4) ? evalParam(env, ci, 4) : null;
    CVarRef a5 = (count > 5) ? evalParam(env, ci, 5) : null;
    (ci->getMethFewArgs())(mcp1, count, a0, a1, a2, a3, a4, a5);
    return o;
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
  (ci->getMeth())(mcp1, Array(ai.create()));
  return o;
}

void NewObjectExpression::dump(std::ostream &out) const {
  out << "new ";
  m_name->dump(out);
  dumpParams(out);
}

///////////////////////////////////////////////////////////////////////////////
}
}

