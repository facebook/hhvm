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

#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/statement_list_statement.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/ast/static_statement.h>
#include <runtime/eval/parser/parser.h>
#include <runtime/eval/ast/scalar_expression.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/ast/array_expression.h>

#include <runtime/eval/ast/name.h>
#include <runtime/eval/ast/function_call_expression.h>
#include <runtime/eval/ast/lval_expression.h>
#include <runtime/eval/strict_mode.h>
#include <runtime/base/runtime_option.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

Parameter::Parameter(CONSTRUCT_ARGS, const string &type,
                     const string &name, int idx, bool ref,
                     ExpressionPtr defVal, int argNum)
  : Construct(CONSTRUCT_PASS), m_type(type),
    m_name(Name::fromString(CONSTRUCT_PASS, name)), m_defVal(defVal),
    m_fnName(NULL), m_idx(idx), m_kind(KindOfNull), m_argNum(argNum),
    m_ref(ref), m_nullDefault(false) {
  if (!type.empty()) {
    m_fnName = parser->peekFunc()->name().c_str();
    if (strcasecmp(type.c_str(), "array") == 0) {
      m_kind = KindOfArray;
    } else {
      m_kind = KindOfObject;
      if (strcasecmp(type.c_str(), "self") == 0 && parser->haveClass()) {
        m_type = parser->peekClass()->name();
      }
    }
    if (m_defVal) {
      ScalarExpressionPtr s = m_defVal->cast<ScalarExpression>();
      bool correct = false;
      if (s) {
        DataType dtype = s->getValue().getType();
        correct = m_nullDefault = dtype == KindOfNull;
      } else {
        ArrayExpressionPtr a = m_defVal->cast<ArrayExpression>();
        correct = a && m_kind == KindOfArray;
      }
      if (!correct) {
        if (m_kind == KindOfArray) {
          throw_fatal("Default value for parameters with array type hint can "
                      "only be an array or NULL");
        } else {
          throw_fatal("Default value for parameters with a class type hint can"
                      " only be NULL");
        }
      }
    }
  }
}

void Parameter::bind(VariableEnvironment &env, CVarRef val,
                     bool ref /* = false */) const {
  if (m_kind != KindOfNull) {
    DataType otype = val.getType();
    if (!(m_nullDefault && otype == KindOfNull ||
          otype == m_kind &&
          (m_kind != KindOfObject ||
           m_kind == KindOfObject &&
           val.toObject().instanceof(m_type.c_str())))) {
      throw_unexpected_argument_type(m_argNum, m_fnName, m_type.c_str(), val);
    }
  }
  if (ref) val.setContagious();
  env.getIdx(m_idx) = val;
}

void Parameter::bindDefault(VariableEnvironment &env) const {
  if (m_defVal) {
    Variant val = m_defVal->eval(env);
    env.getIdx(m_idx) = val;
  }
}

void Parameter::dump() const {
  if (!m_type.empty()) {
    printf("%s ", m_type.c_str());
  }
  if (m_ref) {
    printf("&");
  }
  m_name->dump();

  if (m_defVal) {
    printf(" = ");
    m_defVal->dump();
  }
}

void Parameter::getInfo(ClassInfo::ParameterInfo &info,
                        VariableEnvironment &env) const {
  int attr = 0;
  if (m_ref) {
    attr |= ClassInfo::IsReference;
  }
  if (m_defVal) {
    attr |= ClassInfo::IsOptional;
  }
  if (attr == 0) {
    attr = ClassInfo::IsNothing;
  }
  info.attribute = (ClassInfo::Attribute)attr;
  info.name = m_name->getStatic().c_str();
  info.type = m_type.c_str();
  info.value = NULL;
  if (m_defVal) {
    Variant v = m_defVal->eval(env);
    String s = f_serialize(v);
    info.value = strdup(s);
  }
}

bool Parameter::isOptional() const {
  return m_defVal;
}

void Parameter::dropDefault() {
  m_defVal.reset();
}

FunctionStatement::FunctionStatement(STATEMENT_ARGS, const string &name,
                                     const string &doc)
  : Statement(STATEMENT_PASS), m_name(name),
    m_lname(Util::toLower(m_name)), m_docComment(doc) {
}
FunctionStatement::~FunctionStatement() {}

void FunctionStatement::init(bool ref, const vector<ParameterPtr> params,
                             StatementListStatementPtr body,
                             bool has_call_to_get_args) {
  m_ref = ref;
  m_params = params;
  m_body = body;
  m_hasCallToGetArgs = has_call_to_get_args;

  bool seenNonOptional = false;
  for (int i = m_params.size() - 1; i >= 0; --i) {
    if (!seenNonOptional) {
      if (!m_params[i]->isOptional()) {
        seenNonOptional = true;
      }
    } else if (m_params[i]->isOptional()) {
      m_params[i]->dropDefault();
    }
  }
}

const string &FunctionStatement::fullName() const {
  return m_name;
}

void FunctionStatement::changeName(const std::string &name) {
  m_name = name;
  m_lname = Util::toLower(name);
}

void FunctionStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  // register with function_exists, invoke, etc.
  RequestEvalState::declareFunction(this);
}

Variant FunctionStatement::invoke(CArrRef params) const {
  DECLARE_THREAD_INFO
  RECURSION_INJECTION
  REQUEST_TIMEOUT_INJECTION
#ifdef HOTPROFILER
  ProfilerInjection pi(info, m_name.c_str());
#endif
  FuncScopeVariableEnvironment env(this, params.size());
  EvalFrameInjection fi("", m_name.c_str(), env, loc()->file);
  if (m_ref) {
    return ref(invokeImpl(env, params));
  }

  Variant r = invokeImpl(env, params);
  return r;
}

void FunctionStatement::directBind(VariableEnvironment &env,
                                   const FunctionCallExpression *caller,
                                   FuncScopeVariableEnvironment &fenv) const {
  vector<ParameterPtr>::const_iterator piter = m_params.begin();
  const vector<ExpressionPtr> &args = caller->params();
  vector<ExpressionPtr>::const_iterator it = args.begin();
  VariantStack &as = RequestEvalState::argStack();
  for (; it != args.end() && piter != m_params.end(); ++it, ++piter) {
    Variant v;
    if ((*piter)->isRef() || (*it)->isRefParam()) {
      // should throw if it's ref and not lval
      v = ref((*it)->refval(env));
      (*piter)->bind(fenv, v, true);
    } else {
      v = (*it)->eval(env);
      (*piter)->bind(fenv, v);
    }
    as.push(v);
    fenv.incArgc();
  }
  // more parameters than actual arguments
  for (; piter != m_params.end(); ++piter) {
    if (!(*piter)->isOptional()) {
      throw_missing_argument(fullName().c_str(), (*piter)->argNum());
    }
    (*piter)->bindDefault(fenv);
  }
  // more arguments than parameters
  for (; it != args.end(); ++it) {
    if (RuntimeOption::EnableStrict && !m_hasCallToGetArgs) {
      throw_strict(TooManyArgumentsException(name().c_str()),
                   StrictMode::StrictBasic);
    }
    as.push((*it)->eval(env));
    fenv.incArgc();
  }
}

Variant FunctionStatement::evalBody(VariableEnvironment &env) const {
  if (m_body) {
    m_body->eval(env);
    if (env.isReturning()) {
      if (m_ref) {
        env.getRet().setContagious();
      }
      return env.getRet();
    } else if (env.isBreaking()) {
      throw FatalErrorException("Cannot break/continue out of function");
    }
  }
  return Variant();
}

// env is caller's env
Variant FunctionStatement::directInvoke(VariableEnvironment &env,
                                        const FunctionCallExpression *caller)
  const {
  DECLARE_THREAD_INFO
  RECURSION_INJECTION
  REQUEST_TIMEOUT_INJECTION
#ifdef HOTPROFILER
  ProfilerInjection pi(info, m_name.c_str());
#endif
  FuncScopeVariableEnvironment fenv(this, 0);
  directBind(env, caller, fenv);
  EvalFrameInjection fi("", m_name.c_str(), fenv, loc()->file);
  if (m_ref) {
    return ref(evalBody(fenv));
  } else {
    return evalBody(fenv);
  }
}

LVariableTable *FunctionStatement::getStaticVars(VariableEnvironment &env)
  const {
  return &RequestEvalState::getFunctionStatics(this);
}

Variant FunctionStatement::invokeImpl(VariableEnvironment &env,
                                      CArrRef params) const {
  VariantStack &as = RequestEvalState::argStack();

  for (ArrayIter iter(params); !iter.end(); iter.next()) {
    as.push(iter.second());
  }

  vector<ParameterPtr>::const_iterator piter = m_params.begin();
  for (ArrayIter iter(params); !iter.end() && piter != m_params.end();
       ++piter, iter.next()) {
    if ((*piter)->isRef()) {
      (*piter)->bind(env, iter.secondRef(), true);
    } else {
      (*piter)->bind(env, iter.second());
    }
  }

  // more params than actual args
  for (; piter != m_params.end(); ++piter) {
    if (!(*piter)->isOptional()) {
      throw_missing_argument(fullName().c_str(), (*piter)->argNum());
    }
    (*piter)->bindDefault(env);
  }

  if (m_ref) {
    return ref(evalBody(env));
  } else {
    return evalBody(env);
  }
}

void FunctionStatement::dump() const {
  printf("function %s%s(", m_ref ? "&" : "", m_name.c_str());
  dumpVector(m_params, ", ");
  printf(") {");
  if (m_body) m_body->dump();
  printf("}");
}

void FunctionStatement::getInfo(ClassInfo::MethodInfo &info) const {
  info.attribute = m_ref ? ClassInfo::IsReference : ClassInfo::IsNothing;
  info.name = m_name.c_str();
  if (!m_docComment.empty()) {
    info.docComment = m_docComment.c_str();
  }
  info.invokeFn = NULL;
  info.invokeFailedFn = NULL;
  DummyVariableEnvironment env;
  for (vector<ParameterPtr>::const_iterator it = m_params.begin();
       it != m_params.end(); ++it) {
    ClassInfo::ParameterInfo *pi = new ClassInfo::ParameterInfo;
    (*it)->getInfo(*pi, env);
    info.parameters.push_back(pi);
  }
  for (map<string, ExpressionPtr>::const_iterator it = m_staticStmts.begin();
       it != m_staticStmts.end(); ++it) {
    ClassInfo::ConstantInfo *ci = new ClassInfo::ConstantInfo;
    ci->name = it->first.c_str();
    ci->valueLen = 12;
    ci->valueText = "unsupported";
    if (it->second) {
      ci->value = it->second->eval(env);
    }
    info.staticVariables.push_back(ci);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}

