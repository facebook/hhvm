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
#include <runtime/eval/ast/scalar_expression.h>
#include <runtime/eval/strict_mode.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/intercept.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

Parameter::Parameter(CONSTRUCT_ARGS, const string &type,
                     const string &name, int idx, bool ref,
                     ExpressionPtr defVal, int argNum)
  : Construct(CONSTRUCT_PASS), m_type(type),
    m_name(Name::fromString(CONSTRUCT_PASS, name)), m_defVal(defVal),
    m_idx(idx), m_kind(KindOfNull), m_argNum(argNum),
    m_ref(ref), m_nullDefault(false) {
  if (!type.empty()) {
    m_fnName = parser->peekFunc()->fullName() + "()";
    if (strcasecmp(type.c_str(), "array") == 0) {
      m_kind = KindOfArray;
    } else {
      m_kind = KindOfObject;
      if (strcasecmp(type.c_str(), "self") == 0 && parser->haveClass()) {
        m_type = parser->peekClass()->name();
      }
    }
    if (m_defVal) {
      ScalarExpressionPtr s = m_defVal->unsafe_cast<ScalarExpression>();
      bool correct = false;
      if (s) {
        DataType dtype = s->getValue().getType();
        correct = m_nullDefault = dtype == KindOfNull;
      } else {
        ArrayExpressionPtr a = m_defVal->unsafe_cast<ArrayExpression>();
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
      throw_unexpected_argument_type(m_argNum, m_fnName.c_str(),
                                     m_type.c_str(), val);
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

void Parameter::dump(std::ostream &out) const {
  if (!m_type.empty()) {
    out << m_type << " ";
  }
  if (m_ref) {
    out << "&";
  }
  out << "$";
  m_name->dump(out);

  if (m_defVal) {
    out << " = ";
    m_defVal->dump(out);
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
  info.valueText = NULL; // would be great to have the original PHP code
  if (m_defVal) {
    Variant v;
    try {
      v = m_defVal->eval(env);
    } catch (FatalErrorException e) {
      std::string msg = e.getMessage();
      v = Object((NEW(c_stdClass)())->create());
      v.o_set("msg", String(msg.c_str(), msg.size(), CopyString));
    }
    String s = f_serialize(v);
    info.value = strdup(s);
  }
}

bool Parameter::isOptional() const {
  return m_defVal;
}

void Parameter::addNullDefault(void *parser) {
  ASSERT(!m_defVal);
  m_defVal = ScalarExpressionPtr(new ScalarExpression((Parser *)parser));
}

FunctionStatement::FunctionStatement(STATEMENT_ARGS, const string &name,
                                     const string &doc)
  : Statement(STATEMENT_PASS), m_name(name),
    m_lname(Util::toLower(m_name)), m_maybeIntercepted(-1), m_docComment(doc),
    m_callInfo((void*)Invoker, (void*)InvokerFewArgs, 0, 0, 0) {
}

FunctionStatement::~FunctionStatement() {
  unregister_intercept_flag(&m_maybeIntercepted);
}

void FunctionStatement::init(void *parser, bool ref,
                             const vector<ParameterPtr> params,
                             StatementListStatementPtr body,
                             bool has_call_to_get_args) {
  m_ref = ref;
  m_params = params;
  m_body = body;
  m_hasCallToGetArgs = has_call_to_get_args;

  bool seenOptional = false;
  for (unsigned int i = 0; i < m_params.size(); i++) {
    if (!seenOptional) {
      if (m_params[i]->isOptional()) {
        seenOptional = true;
      }
    } else if (!m_params[i]->isOptional()) {
      m_params[i]->addNullDefault(parser);
    }
    if (m_params[i]->isRef()) m_callInfo.m_refFlags |= 1 << i;
  }
}

const string &FunctionStatement::fullName() const {
  return m_name;
}

void FunctionStatement::changeName(const std::string &name) {
  m_name = name;
  m_lname = Util::toLower(name);
}

const CallInfo *FunctionStatement::getCallInfo() const {
  return &m_callInfo;
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
  FuncScopeVariableEnvironment env(this, params.size());
  EvalFrameInjection fi(empty_string, m_name.c_str(), env, loc()->file);
  if (m_ref) {
    return ref(invokeImpl(env, params));
  }
  return invokeImpl(env, params);
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
      as.pushRef(v);
    } else {
      v = (*it)->eval(env);
      (*piter)->bind(fenv, v);
      as.push(v);
    }
    fenv.incArgc();
  }
  // more parameters than actual arguments
  for (; piter != m_params.end(); ++piter) {
    if (!(*piter)->isOptional()) {
      throw_missing_arguments(fullName().c_str(), (*piter)->argNum());
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
  Variant &ret = env.getRet();

  if (m_maybeIntercepted) {
    Variant handler = get_intercept_handler(fullName(), &m_maybeIntercepted);
    if (!handler.isNull() &&
        handle_intercept(handler, fullName(), env.getParams(), ret)) {
      if (m_ref) {
        ret.setContagious();
      }
      return ret;
    }
  }

  if (m_body) {
    m_body->eval(env);
    if (env.isReturning()) {
      if (m_ref) {
        ret.setContagious();
      }
      return ret;
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
  FuncScopeVariableEnvironment fenv(this, 0);
  directBind(env, caller, fenv);
  EvalFrameInjection fi(empty_string, m_name.c_str(), fenv, loc()->file);
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
      throw_missing_arguments(fullName().c_str(), (*piter)->argNum());
    }
    (*piter)->bindDefault(env);
  }

  if (m_ref) {
    return ref(evalBody(env));
  } else {
    return evalBody(env);
  }
}

void FunctionStatement::dump(std::ostream &out) const {
  out << "function " << (m_ref ? "&" : "") << m_name << "(";
  dumpVector(out, m_params);
  out << ")";
  if (m_body) {
    out << " {\n";
    m_body->dump(out);
    out << "}";
  } else {
    out << ";";
  }
  out << "\n";
}

void FunctionStatement::getInfo(ClassInfo::MethodInfo &info) const {
  int attr = m_ref ? ClassInfo::IsReference : ClassInfo::IsNothing;
  if (m_hasCallToGetArgs) attr |= ClassInfo::VariableArguments;
  info.attribute = (ClassInfo::Attribute)attr;

  info.name = m_name.c_str();
  info.file = m_loc.file;
  info.line1 = m_loc.line0;
  info.line2 = m_loc.line1;
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
      ci->setValue(it->second->eval(env));
    }
    info.staticVariables.push_back(ci);
  }
}

Variant FunctionStatement::Invoker(void *extra, CArrRef params) {
  const Function *f = (const Function*)extra;
  const FunctionStatement *ms = static_cast<const FunctionStatement*>(f);
  if (ms->refReturn()) {
    return ref(ms->invoke(params));
  }
  return ms->invoke(params);
}

Variant FunctionStatement::InvokerFewArgs(void *extra, int count,
    INVOKE_FEW_ARGS_IMPL_ARGS) {
  return Invoker(extra,
                 collect_few_args_ref(count, INVOKE_FEW_ARGS_PASS_ARGS));
}

///////////////////////////////////////////////////////////////////////////////
}
}

