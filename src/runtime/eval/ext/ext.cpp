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
#include <runtime/eval/ext/ext.h>
#include <runtime/eval/base/function.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/parser/parser.h>
#include <runtime/eval/ast/static_statement.h>
#include <runtime/eval/ast/statement_list_statement.h>
#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/ast/function_call_expression.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/ext/ext_class.h>
#include <runtime/ext/ext_array.h>
#include <runtime/ext/ext_misc.h>
#include <runtime/ext/ext_error.h>
#include <runtime/ext/ext_options.h>
#include <runtime/ext/ext_function.h>
#include <runtime/ext/ext_reflection.h>
#include <runtime/eval/eval.h>


namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

class ExtFunction : public Function {
public:
  Variant invoke(CArrRef params) const;
  Variant directInvoke(VariableEnvironment &env,
                       const FunctionCallExpression *caller) const;
  virtual Variant invokeImpl(VariableEnvironment &env,
                             CArrRef params) const = 0;
};
Variant ExtFunction::invoke(CArrRef params) const {
  throw NotSupportedException("Dynamic invoke of special functions",
                              "It's hard and I haven't gotten around to it");
}
Variant ExtFunction::directInvoke(VariableEnvironment &env,
                                  const FunctionCallExpression *caller) const {
  return invokeImpl(env, caller->getParams(env));
}

#define EVAL_EXT(name)                                                 \
  class Eval##name : public ExtFunction {                              \
  public:                                                              \
    static Variant InvokeImpl(VariableEnvironment &env,                \
                       CArrRef params);                                \
    Variant invokeImpl(VariableEnvironment &env,                       \
                       CArrRef params) const {                         \
      return InvokeImpl(env, params);                                  \
    }                                                                  \
    static Variant Invoker(void *extra, CArrRef params) {              \
      return InvokeImpl(*((VariableEnvironment*)extra), params);       \
    }                                                                  \
    const CallInfo *getCallInfo() const {                              \
      return &s_ci;                                                    \
    }                                                                  \
    static CallInfo s_ci;                                              \
  };                                                                   \
  CallInfo Eval##name::s_ci((void*)Eval##name::Invoker, NULL, 0, 0, 0);



#define EVAL_EXT_DYN(name)                                             \
  class Eval##name : public ExtFunction {                              \
  public:                                                              \
    Variant invokeImpl(VariableEnvironment &env,                       \
                       CArrRef params) const {                         \
      return Invoke(params);                                           \
    }                                                                  \
    static Variant Invoke(CArrRef params);                             \
    static Variant Invoker(void *extra, CArrRef params) {              \
      return Invoke(params);                                           \
    }                                                                  \
    const CallInfo *getCallInfo() const {                              \
      return &s_ci;                                                    \
    }                                                                  \
    static CallInfo s_ci;                                              \
  };                                                                   \
  CallInfo Eval##name::s_ci((void*)Eval##name::Invoker, NULL, 0, 0, 0);

EVAL_EXT(Extract);
EVAL_EXT(Define);
EVAL_EXT(FuncGetArg);
EVAL_EXT(FuncGetArgs);
EVAL_EXT(FuncNumArgs);
EVAL_EXT(Compact);
EVAL_EXT(CreateFunction);
EVAL_EXT(Assert);
EVAL_EXT(GetDefinedVars);
EVAL_EXT_DYN(HphpGetClassInfo);
#undef EVAL_EXT
#undef EVAL_EXT_DYN

class EvalFunctionExists : public ExtFunction {
public:
  EvalFunctionExists();
  Variant invokeImpl(VariableEnvironment &env, CArrRef params) const {
    return InvokeImpl(env, params);
  }
  static Variant InvokeImpl(VariableEnvironment &env, CArrRef params);
  static Variant Invoker(void *extra, CArrRef params) {
    return InvokeImpl(*((VariableEnvironment*)extra), params);
  }
  const CallInfo *getCallInfo() const {
    return &s_ci;
  }
  static CallInfo s_ci;
private:
  static hphp_const_char_imap<bool> s_blacklist;
};
hphp_const_char_imap<bool> EvalFunctionExists::s_blacklist;
CallInfo EvalFunctionExists::s_ci((void*)EvalFunctionExists::Invoker, NULL, 0,
    0, 0);

EvalOverrides::EvalOverrides() {
  m_functions["extract"] = new EvalExtract();
  m_functions["define"] = new EvalDefine();
  m_functions["func_get_arg"] = new EvalFuncGetArg();
  m_functions["func_get_args"] = new EvalFuncGetArgs();
  m_functions["func_num_args"] = new EvalFuncNumArgs();
  m_functions["compact"] = new EvalCompact();
  m_functions["create_function"] = new EvalCreateFunction();
  m_functions["assert"] = new EvalAssert();
  m_functions["function_exists"] = new EvalFunctionExists();
  m_functions["get_defined_vars"] = new EvalGetDefinedVars();
  m_functions["hphp_get_class_info"] = new EvalHphpGetClassInfo();
}
EvalOverrides::~EvalOverrides() {
  for (hphp_const_char_imap<const Function*>::iterator it =
         m_functions.begin(); it != m_functions.end(); ++it) {
    delete it->second;
  }
  m_functions.clear();
}

const Function *EvalOverrides::findFunction(const char *name) const {
  hphp_const_char_imap<const Function*>::const_iterator it =
    m_functions.find(name);
  if (it != m_functions.end()) {
    return it->second;
  }
  return NULL;
}

EvalOverrides evalOverrides;

//////////////////////////////////////////////////////////////////////////////
///// Invoke definitions

static Variant invalid_function_call(const char *func) {
  raise_warning("(1) call the function without enough arguments OR "
                "(2) Unable to find function \"%s\" OR "
                "(3) function was not in invoke table OR "
                "(4) function was renamed to something else.", func);
  return null;
}

Variant EvalExtract::InvokeImpl(VariableEnvironment &env,
                                CArrRef params) {
  int size = params.size();
  switch (size) {
  case 1: return extract(&env,params.rvalAt(0));
  case 2: return extract(&env,params.rvalAt(0), params.rvalAt(1));
  case 3: return extract(&env,params.rvalAt(0), params.rvalAt(1),
                         params.rvalAt(2));
  default: return invalid_function_call("extract");
  }
}
Variant EvalDefine::InvokeImpl(VariableEnvironment &env,
                               CArrRef params) {
  int size = params.size();
  switch (size) {
  case 2:
  case 3:
    {
      Variant n = params.rvalAt(0);
      if (!f_defined(n)) {
        return RequestEvalState::declareConstant(n, params.rvalAt(1));
      } else {
        raise_notice("Constant %s already defined", n.toString().data());
        return false;
      }
    }
  default: return invalid_function_call("define");
  }
}

Variant EvalFuncGetArg::InvokeImpl(VariableEnvironment &env,
                                   CArrRef params) {
  int size = params.size();
  switch (size) {
  case 1: {
    int n = params.rvalAt(0);
    if (ObjectData *cont = env.getContinuation()) {
      return cont->o_invoke("get_arg", CREATE_VECTOR1(n));
    }
    if (n >= 0 && n < env.getParams().size()) {
      return env.getParams().rvalAt(n);
    }
    return false;
  }
  default: return invalid_function_call("func_get_arg");
  }
}

Variant EvalFuncGetArgs::InvokeImpl(VariableEnvironment &env,
                                    CArrRef params) {
  int size = params.size();
  switch (size) {
  case 0: {
    if (ObjectData *cont = env.getContinuation()) {
      return cont->o_invoke("get_args", Array::Create());
    }
    Array res = Array::Create();
    for (ArrayIter iter(env.getParams()); !iter.end(); iter.next()) {
      res.append(iter.second());
    }
    return res;
  }
  default: return invalid_function_call("func_get_args");
  }
}

Variant EvalFuncNumArgs::InvokeImpl(VariableEnvironment &env,
                                    CArrRef params) {
  if (ObjectData *cont = env.getContinuation()) {
    return cont->o_invoke("num_args", Array::Create());
  }
  return env.getParams().size();
}

Variant EvalCompact::InvokeImpl(VariableEnvironment &env,
                                CArrRef params) {
  int size = params.size();
  if (size == 0) return invalid_function_call("compact");
  return compact(&env, params.size(), params.rvalAt(0),
                 params.slice(1, params.size() - 1, false));
}

Variant EvalCreateFunction::InvokeImpl(VariableEnvironment &env,
                                       CArrRef params) {
  int size = params.size();
  if (size != 2) return invalid_function_call("create_function");
  Variant var = params.rvalAt(0);
  Variant body = params.rvalAt(1);

  vector<StaticStatementPtr> statics;
  Block::VariableIndices variableIndices;
  ostringstream fnStream;
  string id(RequestEvalState::unique());
  fnStream << "<?php function lambda_" << id << "(" << var.toString().data() <<
    ") {" << body.toString().data() << "}\n";
  StatementPtr bodyAst = Parser::ParseString(fnStream.str().c_str(), statics,
                                             variableIndices);
  if (!bodyAst) return false;
  ostringstream nameStream;
  nameStream << "$lambda_" << id;
  FunctionStatementPtr f = bodyAst->cast<StatementListStatement>()->stmts()[0];
  ASSERT(f);
  f->changeName(nameStream.str());
  SmartPtr<CodeContainer> cc(new StringCodeContainer(bodyAst));
  RequestEvalState::addCodeContainer(cc);
  f->eval(env);
  return f->name();
}

Variant EvalAssert::InvokeImpl(VariableEnvironment &env,
                               CArrRef params) {
  Variant assertion = params.rvalAt(0);
  if (assertion.isString()) {
    // Todo: eval this assertion
    return null;
  }
  return f_assert(assertion);
}

Variant EvalGetDefinedVars::InvokeImpl(VariableEnvironment &env,
                                       CArrRef params) {
  return env.getDefinedVariables();
}


Variant EvalHphpGetClassInfo::Invoke(CArrRef params) {
  String cname = params.rvalAt(0);
  if (!f_class_exists(cname) && !f_interface_exists(cname)) {
    eval_try_autoload(cname.data());
  }
  return f_hphp_get_class_info(cname);
}

EvalFunctionExists::EvalFunctionExists() {
  s_blacklist["fb_get_derived_classes"] = true;
}

Variant EvalFunctionExists::InvokeImpl(VariableEnvironment &env,
                                       CArrRef params) {

  if (params.size() != 1) {
    return invalid_function_call("function_exists");
  }
  String fn = params.rvalAt(0).toString();
  if (s_blacklist.find(fn.data()) != s_blacklist.end()) return false;
  return f_function_exists(fn);
}

///////////////////////////////////////////////////////////////////////////////
}
}
