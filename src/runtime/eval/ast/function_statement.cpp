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
#include <runtime/eval/ast/closure_expression.h>
#include <runtime/eval/ast/lval_expression.h>
#include <runtime/eval/ast/scalar_expression.h>
#include <runtime/eval/strict_mode.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/intercept.h>
#include <system/gen/php/classes/closure.h>

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
    if (parser->haveFunc()) {
      m_fnName = parser->peekFunc()->fullName();
    }

    const TypePtrMap &types = GetTypeHintTypes();
    TypePtrMap::const_iterator iter;
    if ((iter = types.find(type)) != types.end()) {
      m_kind = iter->second;
      ASSERT(m_kind != KindOfUninit);
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
        ASSERT(dtype != KindOfUninit);
        m_nullDefault = dtype == KindOfNull;
        if (m_kind == KindOfObject) {
          correct = m_nullDefault;
        } else if (m_kind == KindOfArray) {
          correct = (dtype == m_kind || dtype == KindOfNull);
        } else {
          correct = dtype == m_kind;
        }
      } else {
        ArrayExpressionPtr a = m_defVal->unsafe_cast<ArrayExpression>();
        correct = a && m_kind == KindOfArray;
      }
      if (!correct) {
        if (m_kind == KindOfArray) {
          parser->error("Default value with array type hint can only be "
                        "an array or NULL");
        } else if (m_kind == KindOfObject) {
          parser->error("Default value with a class type hint can only be "
                        "NULL");
        } else {
          ASSERT(RuntimeOption::EnableHipHopSyntax);
          parser->error("Default value need to have the same type as "
                        "the type hint");
        }
      }
    }
  }
}

void Parameter::bind(VariableEnvironment &env, CVarRef val,
                     bool ref /* = false */) const {
  if (m_kind != KindOfNull) {
    DataType otype = val.getType();
    if (otype == KindOfInt32) otype = KindOfInt64;
    else if (otype == KindOfStaticString) otype = KindOfString;
    ASSERT(otype != KindOfUninit);
    if (!(m_nullDefault && otype == KindOfNull ||
          otype == m_kind &&
          (m_kind != KindOfObject ||
           m_kind == KindOfObject &&
           val.toObject().instanceof(m_type.c_str())))) {
      throw_unexpected_argument_type(m_argNum, (m_fnName + "()").c_str(),
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

std::string Parameter::name() const {
  return m_name->get().data();
}

String Parameter::getName() const {
  return m_name->get();
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
  info.name = m_name->get().c_str();
  info.type = m_type.c_str();
  info.value = NULL;
  info.valueText = NULL; // would be great to have the original PHP code
  if (m_defVal) {
    Variant v;
    try {
      v = m_defVal->eval(env);
    } catch (FatalErrorException e) {
      std::string msg = e.getMessage();
      v = Object((NEWOBJ(c_stdClass)())->create());
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
    m_maybeIntercepted(-1), m_yieldCount(0), m_closure(NULL),
    m_docComment(doc),
    m_callInfo((void*)Invoker, (void*)InvokerFewArgs, 0, 0, 0) {
}

FunctionStatement::~FunctionStatement() {
  unregister_intercept_flag(&m_maybeIntercepted);
}

void FunctionStatement::init(void *parser, bool ref,
                             const vector<ParameterPtr> &params,
                             StatementListStatementPtr body,
                             bool has_call_to_get_args) {
  m_ref = ref;
  m_params = params;
  m_body = body;
  m_hasCallToGetArgs = has_call_to_get_args;

  bool seenOptional = false;
  set<string> names;
  m_callInfo.m_argCount = m_params.size();
  for (unsigned int i = 0; i < m_params.size(); i++) {
    ParameterPtr param = m_params[i];

    std::string name = param->name();
    if (names.find(name) != names.end()) {
      raise_notice("%s:%d %s() has 2 parameters with the same name: $%s",
                   m_loc.file, m_loc.line0, m_name.c_str(), name.c_str());
    } else {
      names.insert(name);
    }

    if (!seenOptional) {
      if (param->isOptional()) {
        seenOptional = true;
      }
    } else if (!param->isOptional()) {
/*
      raise_notice("%s:%d %s() has required parameter after optional one: $%s",
                   m_loc.file, m_loc.line0, m_name.c_str(), name.c_str());
*/
      param->addNullDefault(parser);
    }
    if (param->isRef()) m_callInfo.m_refFlags |= 1 << i;

    if (param->getIdx() == -1) {
      param->setIdx(declareVariable(name));
    }
  }
}

String FunctionStatement::fullName() const {
  return m_name;
}

void FunctionStatement::changeName(const std::string &name) {
  m_name = name;
}

const CallInfo *FunctionStatement::getCallInfo() const {
  return &m_callInfo;
}

void FunctionStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  ENTER_STMT;
  // register with function_exists, invoke, etc.
  RequestEvalState::declareFunction(this);
}

Variant FunctionStatement::invoke(CArrRef params) const {
  DECLARE_THREAD_INFO_NOINIT
  if (m_closure) {
    if (m_ref) {
      return ref(invokeClosure(params));
    }
    return invokeClosure(params);
  }
  FuncScopeVariableEnvironment env(this);
  EvalFrameInjection fi(empty_string, m_name.c_str(), env, loc()->file);
  if (m_ref) {
    return ref(invokeImpl(env, params));
  }
  return invokeImpl(env, params);
}

void FunctionStatement::directBind(VariableEnvironment &env,
                                   const FunctionCallExpression *caller,
                                   FuncScopeVariableEnvironment &fenv,
                                   int start /* = 0 */) const {
  vector<ParameterPtr>::const_iterator piter = m_params.begin();
  const vector<ExpressionPtr> &args = caller->params();
  vector<ExpressionPtr>::const_iterator it = args.begin() + start;
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
      if ((*piter)->hasTypeHint()) {
        const string &t = (*piter)->type();
        throw_missing_typed_argument(fullName().c_str(),
                                     (t == "array" ? 0 : t.c_str()),
                                     (*piter)->argNum());
      } else {
        throw_missing_arguments(fullName().c_str(), (*piter)->argNum());
      }
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
    restart:
    try {
      m_body->eval(env);
    } catch (GotoException &e) {
      goto restart;
    } catch (UnlimitedGotoException &e) {
      goto restart;
    }
    if (env.isGotoing()) {
      throw FatalErrorException(0, "Unable to reach goto label %s",
                                env.getGoto().c_str());
    }
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
  DECLARE_THREAD_INFO_NOINIT
  FuncScopeVariableEnvironment fenv(this);
  directBind(env, caller, fenv);
  EvalFrameInjection fi(empty_string, m_name.c_str(), fenv, loc()->file);
  if (m_ref) {
    return ref(evalBody(fenv));
  } else {
    return evalBody(fenv);
  }
}

// env is caller's env
Variant FunctionStatement::invokeClosure(CObjRef closure,
                                         VariableEnvironment &env,
                                         const FunctionCallExpression *caller,
                                         int start /* = 0 */)
  const {
  DECLARE_THREAD_INFO_NOINIT
  FuncScopeVariableEnvironment fenv(this);
  directBind(env, caller, fenv, start);

  p_Closure c = closure.getTyped<c_Closure>();
  const std::vector<ParameterPtr> &vars =
    ((ClosureExpression *)m_closure)->getVars();
  for (ArrayIter iter(c->m_vars); iter; ++iter) {
    int i = iter.first();
    CVarRef var = iter.secondRef();
    Parameter *param = vars[i].get();
    if (param->isRef()) {
      fenv.get(param->getName()) = ref(var);
    } else {
      fenv.get(param->getName()) = var;
    }
  }

  EvalFrameInjection fi(empty_string, "{closure}", fenv, loc()->file);
  if (m_ref) {
    return ref(evalBody(fenv));
  } else {
    return evalBody(fenv);
  }
}

Variant FunctionStatement::invokeClosure(CArrRef params) const {
  FuncScopeVariableEnvironment fenv(this);
  bindParams(fenv, params);

  DECLARE_THREAD_INFO;
  FrameInjection *fi;
  for (fi = info->m_top; fi; fi= fi->getPrev()) {
    if (fi->isEvalFrame()) {
      break;
    }
  }
  EvalFrameInjection *efi = static_cast<EvalFrameInjection*>(fi);
  c_Closure *closure = (c_Closure *) efi->getEnv().getClosure();
  const std::vector<ParameterPtr> &vars =
    ((ClosureExpression *)m_closure)->getVars();
  for (ArrayIter iter(closure->m_vars); iter; ++iter) {
    int i = iter.first();
    CVarRef var = iter.secondRef();
    Parameter *param = vars[i].get();
    if (param->isRef()) {
      fenv.get(param->getName()) = ref(var);
    } else {
      fenv.get(param->getName()) = var;
    }
  }

  EvalFrameInjection efiLocal(empty_string, "{closure}", fenv, loc()->file);
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

void FunctionStatement::bindParams(FuncScopeVariableEnvironment &fenv,
                                   CArrRef params) const {
  VariantStack &as = RequestEvalState::argStack();

  for (ArrayIter iter(params); !iter.end(); iter.next()) {
    as.push(iter.second());
    fenv.incArgc();
  }

  vector<ParameterPtr>::const_iterator piter = m_params.begin();
  for (ArrayIter iter(params); !iter.end() && piter != m_params.end();
       ++piter, iter.next()) {
    if ((*piter)->isRef()) {
      (*piter)->bind(fenv, iter.secondRef(), true);
    } else {
      (*piter)->bind(fenv, iter.second());
    }
  }

  // more params than actual args
  for (; piter != m_params.end(); ++piter) {
    if (!(*piter)->isOptional()) {
      if ((*piter)->hasTypeHint()) {
        const string &t = (*piter)->type();
        throw_missing_typed_argument(fullName().c_str(),
                                     (t == "array" ? 0 : t.c_str()),
                                     (*piter)->argNum());
      } else {
        throw_missing_arguments(fullName().c_str(), (*piter)->argNum());
      }
    }
    (*piter)->bindDefault(fenv);
  }
}

Variant FunctionStatement::invokeImpl(FuncScopeVariableEnvironment &fenv,
                                      CArrRef params) const {
  bindParams(fenv, params);

  if (m_ref) {
    return ref(evalBody(fenv));
  } else {
    return evalBody(fenv);
  }
}

void FunctionStatement::dumpHeader(std::ostream &out) const {
  out << "function " << (m_ref ? "&" : "");
  if (name()[0] != '0') {
    out << m_name.c_str();
  }
  out << "(";
  dumpVector(out, m_params);
  out << ")";
}

void FunctionStatement::dumpBody(std::ostream &out) const {
  if (m_body) {
    out << " {\n";
    m_body->dump(out);
    out << "}";
  } else {
    out << ";";
  }
  out << "\n";
}

void FunctionStatement::dump(std::ostream &out) const {
  dumpHeader(out);
  dumpBody(out);
}

void FunctionStatement::getInfo(ClassInfo::MethodInfo &info) const {
  int attr = m_ref ? ClassInfo::IsReference : ClassInfo::IsNothing;
  if (m_hasCallToGetArgs) attr |= ClassInfo::VariableArguments;
  info.attribute = (ClassInfo::Attribute)attr;

  info.name = m_name;
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
  for (StringMap<ExpressionPtr>::const_iterator it = m_staticStmts.begin();
       it != m_staticStmts.end(); ++it) {
    ClassInfo::ConstantInfo *ci = new ClassInfo::ConstantInfo;
    ci->name = it->first;
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

