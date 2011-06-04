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

#include <runtime/eval/ast/simple_function_call_expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/base/string_util.h>
#include <runtime/eval/ast/scalar_expression.h>
#include <runtime/eval/ast/closure_expression.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/parser/parser.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

SimpleFunctionCallExpression::SimpleFunctionCallExpression
(EXPRESSION_ARGS, NamePtr name, const std::vector<ExpressionPtr> &params) :
  FunctionCallExpression(EXPRESSION_PASS, params), m_name(name) {}

Variant SimpleFunctionCallExpression::eval(VariableEnvironment &env) const {
  SET_LINE;

  Variant var(m_name->getAsVariant(env));
  if (var.is(KindOfObject)) {
    const CallInfo *cit; 
    void *extra;
    get_call_info_or_fail(cit, extra, var);
    ASSERT(cit);
    return evalCallInfo(cit, extra, env);
  }

  String name = var.toString();
  String originalName = name;
  bool renamed = false;

  name = get_renamed_function(name, &renamed);
  if (name[0] == '\\') {
    name = name.substr(1); // try namespaced function first
    renamed = true;
  }

  // fast path for interpreted fn
  const Function *fs = RequestEvalState::findFunction(name.data());
  if (fs) {
    return strongBind(fs->directInvoke(env, this));
  }

  if (originalName[0] == '\\') {
    name = originalName.lastToken('\\');
    name = get_renamed_function(name, &renamed);
    renamed = true;
    fs = RequestEvalState::findFunction(name.data());
    if (fs) {
      return strongBind(fs->directInvoke(env, this));
    }
  }

  // Handle builtins
  const CallInfo* cit1;
  void* vt1;
  get_call_info_or_fail(cit1, vt1, name);
  // If the lookup failed get_call_info_or_fail() must throw an exception,
  // so if we reach here cit1 must not be NULL
  ASSERT(cit1);
  return evalCallInfo(cit1, vt1, env);
}

Variant SimpleFunctionCallExpression::evalCallInfo(
    const CallInfo *cit,
    void *extra,
    VariableEnvironment &env) const {
  ASSERT(cit);
  ArrayInit ai(m_params.size(), true);
  for (unsigned int i = 0; i < m_params.size(); ++i) {
    if (cit->mustBeRef(i)) {
      ai.setRef(m_params[i]->refval(env));
    } else if (cit->isRef(i)) {
      ai.setRef(m_params[i]->refval(env, 0));
    } else {
      ai.set(m_params[i]->eval(env));
    }
  }
  return strongBind((cit->getFunc())(extra, Array(ai.create())));
}

void SimpleFunctionCallExpression::dump(std::ostream &out) const {
  m_name->dump(out);
  dumpParams(out);
}

ExpressionPtr
SimpleFunctionCallExpression::make(EXPRESSION_ARGS, NamePtr name,
                                   const vector<ExpressionPtr> &params,
                                   const Parser &p) {
  String sname = name->get();
  if (!sname.isNull()) {
    if (strcasecmp(sname.data(), "get_class") == 0 && params.size() == 0) {
      if (p.currentClass()) {
        return new ScalarExpression(EXPRESSION_PASS, p.currentClass()->name());
      } else {
        return new ScalarExpression(EXPRESSION_PASS, false);
      }
    } else if (strcasecmp(sname.data(), "get_parent_class") == 0 &&
               params.size() == 0) {
      if (p.currentClass() && !p.currentClass()->parent().empty()) {
        return new ScalarExpression(EXPRESSION_PASS,
                                    p.currentClass()->parent());
      } else {
        return new ScalarExpression(EXPRESSION_PASS, false);
      }
    }
  }
  return new SimpleFunctionCallExpression(EXPRESSION_PASS, name, params);
}

///////////////////////////////////////////////////////////////////////////////
}
}

