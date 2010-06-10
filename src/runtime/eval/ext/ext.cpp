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
    Variant invokeImpl(VariableEnvironment &env,                       \
                       CArrRef params) const;                          \
  };


#define EVAL_EXT_DYN(name)                                             \
  class Eval##name : public ExtFunction {                              \
  public:                                                              \
    Variant invokeImpl(VariableEnvironment &env,                       \
                       CArrRef params) const {                         \
      return invoke(params);                                           \
    }                                                                  \
    Variant invoke(CArrRef params) const;                              \
  };

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
EVAL_EXT_DYN(ClassExists);
EVAL_EXT_DYN(InterfaceExists);
#undef EVAL_EXT

class EvalFunctionExists : public ExtFunction {
public:
  EvalFunctionExists();
  Variant invokeImpl(VariableEnvironment &env,
                     CArrRef params) const;
private:
  hphp_const_char_imap<bool> m_blacklist;
};

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
  m_functions["class_exists"] = new EvalClassExists();
  m_functions["interface_exists"] = new EvalInterfaceExists();
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

Variant EvalExtract::invokeImpl(VariableEnvironment &env,
                                CArrRef params) const {
  int size = params.size();
  switch (size) {
  case 1: return extract(&env,params.rvalAt(0));
  case 2: return extract(&env,params.rvalAt(0), params.rvalAt(1));
  case 3: return extract(&env,params.rvalAt(0), params.rvalAt(1),
                         params.rvalAt(2));
  default: throw InvalidFunctionCallException("extract");
  }
}
Variant EvalDefine::invokeImpl(VariableEnvironment &env,
                               CArrRef params) const {
  int size = params.size();
  switch (size) {
  case 2:
  case 3:
    {
      Variant n = params.rvalAt(0);
      if (!f_defined(n)) {
        return RequestEvalState::declareConstant(n, params.rvalAt(1));
      } else {
        return false;
      }
    }
  default: throw InvalidFunctionCallException("define");
  }
}

Variant EvalFuncGetArg::invokeImpl(VariableEnvironment &env,
                                   CArrRef params) const {
  int size = params.size();
  switch (size) {
  case 1: return env.getParams().rvalAt(params.rvalAt(0));
  default: throw InvalidFunctionCallException("func_get_arg");
  }
}

Variant EvalFuncGetArgs::invokeImpl(VariableEnvironment &env,
                                    CArrRef params) const {
  int size = params.size();
  switch (size) {
  case 0: {
    Array res = Array::Create();
    for (ArrayIter iter(env.getParams()); !iter.end(); iter.next()) {
      res.append(iter.second());
    }
    return res;
  }
  default: throw InvalidFunctionCallException("func_get_args");
  }
}

Variant EvalFuncNumArgs::invokeImpl(VariableEnvironment &env,
                                    CArrRef params) const {
  int size = params.size();
  if (size != 0) throw InvalidFunctionCallException("func_num_args");
  return env.getParams().size();
}

Variant EvalCompact::invokeImpl(VariableEnvironment &env,
                                CArrRef params) const {
  int size = params.size();
  if (size == 0) throw InvalidFunctionCallException("compact");
  return compact(&env, params.size(), params.rvalAt(0),
                 params.slice(1, params.size() - 1, false));
}

Variant EvalCreateFunction::invokeImpl(VariableEnvironment &env,
                                       CArrRef params) const {
  int size = params.size();
  if (size != 2) throw InvalidFunctionCallException("create_function");
  Variant var = params.rvalAt(0);
  Variant body = params.rvalAt(1);

  vector<StaticStatementPtr> statics;
  ostringstream fnStream;
  int64 id = RequestEvalState::unique();
  fnStream << "<?php function lambda_" << id << "(" << var.toString().data() <<
    ") {" << body.toString().data() << "}\n";
  StatementPtr bodyAst = Parser::parseString(fnStream.str().c_str(), statics);
  if (!bodyAst) return false;
  ostringstream nameStream;
  nameStream << "$lambda_" << id;
  FunctionStatementPtr f = bodyAst->cast<StatementListStatement>()->stmts()[0];
  ASSERT(f);
  f->changeName(nameStream.str());
  vector<FunctionStatementPtr> fs;
  fs.push_back(f);
  vector<ClassStatementPtr> cls;
  StringCodeContainer *cc = new StringCodeContainer(cls, fs);
  RequestEvalState::addCodeContainer(cc);
  f->eval(env);
  return String(f->name().c_str(), f->name().size(), AttachLiteral);
}

Variant EvalAssert::invokeImpl(VariableEnvironment &env,
                               CArrRef params) const {
  Variant assertion = params.rvalAt(0);
  if (assertion.isString()) {
    // Todo: eval this assertion
    return null;
  }
  return f_assert(assertion);
}

Variant EvalClassExists::invoke(CArrRef params) const {
  String cname = params.rvalAt(0);
  if (!f_class_exists(cname, false)) {
    if ((params.size() == 1 || params.rvalAt(1).toBoolean()) &&
        eval_try_autoload(cname.data())) {
      return f_class_exists(cname, false);
    }
    return false;
  }
  return true;
}

Variant EvalInterfaceExists::invoke(CArrRef params) const {
  String cname = params.rvalAt(0);
  if (!f_interface_exists(cname, false)) {
    if ((params.size() == 1 || params.rvalAt(1).toBoolean()) &&
        eval_try_autoload(cname.data())) {
      return f_interface_exists(cname, false);
    }
    return false;
  }
  return true;
}

Variant EvalGetDefinedVars::invokeImpl(VariableEnvironment &env,
                                       CArrRef params) const {
  return env.getDefinedVariables();
}


Variant EvalHphpGetClassInfo::invoke(CArrRef params) const {
  String cname = params.rvalAt(0);
  if (!f_class_exists(cname)) {
    eval_try_autoload(cname.data());
  }
  return f_hphp_get_class_info(cname);
}

EvalFunctionExists::EvalFunctionExists() {
  m_blacklist["fb_get_derived_classes"] = true;
}

Variant EvalFunctionExists::invokeImpl(VariableEnvironment &env,
                                       CArrRef params) const {

  if (params.size() != 1)
    throw InvalidFunctionCallException("function_exists");
  String fn = params.rvalAt(0).toString();
  if (m_blacklist.find(fn.data()) != m_blacklist.end()) return false;
  return f_function_exists(fn);
}

///////////////////////////////////////////////////////////////////////////////
}
}
