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

#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/runtime/variable_environment.h>
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

#include <runtime/ext/ext_closure.h>
#include <system/lib/systemlib.h>

#include <util/parser/parser.h>
#include <util/logger.h>
#include <tbb/concurrent_hash_map.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

typedef tbb::concurrent_hash_map<StringData *, int,
                                 StringDataHashICompare> UserFunctionIdMap;
static UserFunctionIdMap s_userFunctionIdMap;

static int s_id = -1;

#define INITIAL_FUNC_ID_TABLE_SIZE 4096
#define INC_FUNC_ID_TABLE_SIZE 1024

IMPLEMENT_REQUEST_LOCAL(UserFunctionIdTable,
                        UserFunctionIdTable::s_userFunctionIdTable);

UserFunctionIdTable::UserFunctionIdTable() : m_size(0) {
  m_funcStmts = (const FunctionStatement **)
    calloc(INITIAL_FUNC_ID_TABLE_SIZE, sizeof(FunctionStatement *));
  if (m_funcStmts) {
    m_size = m_alloc = INITIAL_FUNC_ID_TABLE_SIZE;
  }
}

void UserFunctionIdTable::requestInit() {
  ASSERT(s_id <= RuntimeOption::MaxUserFunctionId);
  memset(m_funcStmts, 0, sizeof(FunctionStatement *) * m_size);
  grow(atomic_add(s_id, 0) + 1);
}

bool UserFunctionIdTable::grow(int size) {
  ASSERT(size <= RuntimeOption::MaxUserFunctionId + 1);
  int inc = size - m_size;
  if (inc <= 0) return false;
  if (size >= m_alloc) {
    m_alloc += INC_FUNC_ID_TABLE_SIZE;
    if (inc > INC_FUNC_ID_TABLE_SIZE) {
      m_alloc += inc - inc % INC_FUNC_ID_TABLE_SIZE;
    }
    m_funcStmts = (const FunctionStatement **)
      realloc(m_funcStmts, m_alloc * sizeof(FunctionStatement *));
  }
  memset(m_funcStmts + m_size, 0, sizeof(FunctionStatement *) * inc);
  m_size = size;
  return true;
}

int UserFunctionIdTable::getUserFunctionId(CStrRef func) {
  ASSERT(func->isStatic());
  UserFunctionIdMap::accessor acc;
  if (s_userFunctionIdMap.insert(acc, func.get())) {
    int id = atomic_inc(s_id);
    acc->second = id;
    grow(id + 1);
  }
  return acc->second;
}

int UserFunctionIdTable::GetUserFunctionId(CStrRef func) {
  if (s_id >= RuntimeOption::MaxUserFunctionId) return -1;
  if (ParserBase::IsClosureName(func.data())) return -1;
  if (strncmp(func.data(), "lambda_", 7) == 0) return -1; // create_function
  return s_userFunctionIdTable->getUserFunctionId(func);
}

bool UserFunctionIdTable::declareUserFunction(
  const FunctionStatement *funcStmt) {
  int id = funcStmt->getId();
  if (id == -1 || id >= m_size) return false;
  ASSERT(id >= 0);
  m_funcStmts[id] = funcStmt;
  return true;
}

bool UserFunctionIdTable::DeclareUserFunction(
  const FunctionStatement *funcStmt) {
  return s_userFunctionIdTable->declareUserFunction(funcStmt);
}

const FunctionStatement *UserFunctionIdTable::getUserFunction(int id) {
  if (id == -1 || id >= m_size) return NULL;
  ASSERT(id >= 0);
  return m_funcStmts[id];
}

const FunctionStatement *UserFunctionIdTable::GetUserFunction(int id) {
  return s_userFunctionIdTable->getUserFunction(id);
}

bool Parameter::checkTypeHint(DataType hint, DataType type) const {
  switch (hint) {
  case KindOfObject:
    return type == KindOfNull;
  case KindOfArray:
    return (type == hint || type == KindOfNull);
  case KindOfInt64:
    return (type == KindOfInt64 ||
            type == KindOfInt32 ||
            type == KindOfNull);
  case KindOfString:
    return (type == KindOfString ||
            type == KindOfStaticString ||
            type == KindOfNull);
  case KindOfBoolean:
    return type == KindOfBoolean || type == KindOfNull;
  case KindOfDouble:
    return type == KindOfDouble || type == KindOfNull;
  default:
    ASSERT(false);
    break;
  }
  return hint == type;
}

Parameter::Parameter(CONSTRUCT_ARGS, const string &type,
                     const string &name, int idx, bool ref,
                     ExpressionPtr defVal, int argNum)
  : Construct(CONSTRUCT_PASS), m_type(type),
    m_name(Name::fromString(CONSTRUCT_PASS, name)), m_defVal(defVal),
    m_idx(idx), m_kind(KindOfNull), m_argNum(argNum),
    m_ref(ref), m_nullDefault(false), m_correct(false) {
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
      DummyVariableEnvironment env;
      Variant v;
      if (m_defVal->evalStaticScalar(env, v)) {
        DataType dtype = v.getType();
        ASSERT(dtype != KindOfUninit);
        m_nullDefault = (dtype == KindOfNull &&
                         m_defVal->unsafe_cast<ScalarExpression>());
        if (!checkTypeHint(m_kind, dtype)) {
          reportTypeHintError(NULL, type);
        } else if (m_defVal->unsafe_cast<ScalarExpression>() ||
                   m_defVal->unsafe_cast<ArrayExpression>()) {
          m_correct = true;
        }
      }
    }
  }
}

Parameter *Parameter::optimize(VariableEnvironment &env) {
  Eval::optimize(env, m_defVal);
  return NULL;
}

Variant *Parameter::getParam(FuncScopeVariableEnvironment &fenv) const {
  ASSERT(fenv.getIdx(m_idx) == NULL);
  CStrRef s = m_name->get(fenv);
  AssocList &alist = fenv.getAssocList();
  ASSERT(!alist.getPtr(s));
  Variant *v = &alist.append(s);
  fenv.setIdx(m_idx, v);
  return v;
}

void Parameter::bind(FuncScopeVariableEnvironment &fenv, CVarRef val,
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
  Variant *vp = getParam(fenv);
  if (ref) {
    vp->assignRef(val);
  } else {
    vp->assignVal(val);
  }
}

void Parameter::bindDefault(FuncScopeVariableEnvironment &fenv) const {
  if (m_defVal) {
    Variant v = m_defVal->eval(fenv);
    if (hasTypeHint() && !m_correct) {
      DataType dtype = v.getType();
      ASSERT(dtype != KindOfUninit);
      if (!checkTypeHint(m_kind, dtype)) {
        reportTypeHintError(NULL, m_type);
      }
    }
    Variant *vp = getParam(fenv);
    vp->assignVal(v);
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

bool Parameter::getSuperGlobal(SuperGlobal &sg) {
  return m_name->getSuperGlobal(sg);
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
      v = Object(SystemLib::AllocStdClassObject());
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
  : Statement(STATEMENT_PASS),
    m_invalid(0), m_maybeIntercepted(-1), m_yieldCount(0),
    m_name(StringData::GetStaticString(name)), m_closure(NULL),
    m_docComment(doc),
    m_callInfo((void*)Invoker, (void*)InvokerFewArgs, 0, 0, 0),
    m_closureCallInfo((void*)FSInvoker, (void*)FSInvokerFewArgs, 0, 0, 0) {
  m_id = UserFunctionIdTable::GetUserFunctionId(m_name);
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
  const CallInfo* cit1;
  void* vt1;
  if (get_call_info_no_eval(cit1, vt1, m_name)) {
    m_invalid =
      get_call_info_builtin(cit1, vt1, m_name->data(), m_name->hash()) ? -1 : 1;
  }

  bool seenOptional = false;
  set<String> names;
  m_callInfo.m_argCount = m_closureCallInfo.m_argCount = m_params.size();
  for (unsigned int i = 0; i < m_params.size(); i++) {
    ParameterPtr param = m_params[i];

    String name = param->getName();
    if (names.find(name) != names.end()) {
      raise_notice("%s:%d %s() has 2 parameters with the same name: $%s",
                   m_loc.file, m_loc.line0, m_name->data(), name.c_str());
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
    if (param->isRef()) {
      m_callInfo.m_refFlags        |= 1 << i;
      m_closureCallInfo.m_refFlags |= 1 << i;
    }

    if (param->getIdx() == -1) {
      param->setIdx(declareVariable(name));
    }
  }

  m_injectionName = computeInjectionName();
}

String FunctionStatement::fullName() const {
  return m_name;
}

void FunctionStatement::changeName(const std::string &name) {
  m_name = StringData::GetStaticString(name);
}

const CallInfo *FunctionStatement::getCallInfo() const {
  return &m_callInfo;
}

const CallInfo *FunctionStatement::getClosureCallInfo() const {
  return &m_closureCallInfo;
}

std::string
FunctionStatement::computeInjectionName() const {
  string injectionName;
  if (getGeneratorFunc()) {
    injectionName = isClosure() ?
      "{closure}" :
      String(m_name) + "{continuation}";
  } else if (getOrigGeneratorFunc()) {
    injectionName = getOrigGeneratorFunc()->isClosure() ?
      "{closureGen}" :
      getOrigGeneratorFunc()->name();
  } else {
    injectionName = string(m_name->data());
  }
  return injectionName;
}

void optimize(VariableEnvironment &env, ParameterPtr &param) {
  if (!param) return;
  if (ParameterPtr optParam = param->optimize(env)) {
    param = optParam;
  }
}

Statement *FunctionStatement::optimize(VariableEnvironment &env) {
  for (unsigned int i = 0; i < m_params.size(); i++) {
    Eval::optimize(env, m_params[i]);
  }
  Eval::optimize(env, m_body);
  return NULL;
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
      return strongBind(invokeClosure(params));
    }
    return invokeClosure(params);
  }
  FuncScopeVariableEnvironment env(this);
  EvalFrameInjection fi(empty_string, m_injectionName.c_str(), env,
                        loc()->file);
  if (m_ref) {
    return strongBind(invokeImpl(env, params));
  }
  return invokeImpl(env, params);
}

Variant FunctionStatement::invokeFewArgs(
  int count, INVOKE_FEW_ARGS_IMPL_ARGS) const {
  DECLARE_THREAD_INFO_NOINIT
  if (m_closure) {
    if (m_ref) {
      return strongBind(invokeClosureFewArgs(count, INVOKE_FEW_ARGS_PASS_ARGS));
    }
    return invokeClosureFewArgs(count, INVOKE_FEW_ARGS_PASS_ARGS);
  }
  FuncScopeVariableEnvironment env(this);
  EvalFrameInjection fi(empty_string, m_injectionName.c_str(), env,
                        loc()->file);
  if (m_ref) {
    return strongBind(invokeImplFewArgs(env, count, INVOKE_FEW_ARGS_PASS_ARGS));
  }
  return invokeImplFewArgs(env, count, INVOKE_FEW_ARGS_PASS_ARGS);
}

void FunctionStatement::directBind(VariableEnvironment &env,
                                   const FunctionCallExpression *caller,
                                   FuncScopeVariableEnvironment &fenv,
                                   int start /* = 0 */) const {
  vector<ParameterPtr>::const_iterator piter = m_params.begin();
  const vector<ExpressionPtr> &args = caller->params();
  vector<ExpressionPtr>::const_iterator it = args.begin() + start;
  VariantStack &as = RequestEvalState::argStack();
  ASSERT(!hasYield());
  for (; it != args.end() && piter != m_params.end(); ++it, ++piter) {
    if ((*piter)->isRef() || (*it)->isRefParam()) {
      Variant v;
      // should throw if it's ref and not lval
      v.assignRef((*it)->refval(env));
      (*piter)->bind(fenv, v, true);
      as.pushRef(v);
    } else {
      CVarRef v = (*it)->eval(env);
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
    CVarRef v = (*it)->eval(env);
    as.push(v);
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
        return strongBind(ret);
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
        return strongBind(ret);
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
  EvalFrameInjection fi(empty_string, m_injectionName.c_str(), fenv,
                        loc()->file);
  if (m_ref) {
    return strongBind(evalBody(fenv));
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

  p_GeneratorClosure c = closure.getTyped<c_GeneratorClosure>();
  return invokeClosureCommon(c.get(), fenv);
}

inline Variant FunctionStatement::invokeClosureCommon(
  c_GeneratorClosure *closure, FuncScopeVariableEnvironment &fenv) const {
  const std::vector<ParameterPtr> &vars =
    ((ClosureExpression *)m_closure)->getVars();
  for (ArrayIter iter(closure->m_vars); iter; ++iter) {
    int i = iter.first();
    CVarRef var = iter.secondRef();
    Parameter *param = vars[i].get();
    if (param->isRef()) {
      fenv.get(param->getName()).assignRef(var);
    } else {
      fenv.get(param->getName()).assignVal(var);
    }
  }

  EvalFrameInjection efiLocal(empty_string, "{closure}", fenv, loc()->file);
  if (m_ref) {
    return strongBind(evalBody(fenv));
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
  c_GeneratorClosure *closure = (c_GeneratorClosure*)efi->getEnv().getClosure();
  return invokeClosureCommon(closure, fenv);
}

Variant FunctionStatement::invokeClosureFewArgs(
  ObjectData *closure, int count, INVOKE_FEW_ARGS_IMPL_ARGS) const {
  ASSERT(closure);
  ASSERT(m_closure);
  FuncScopeVariableEnvironment fenv(this);
  fenv.setClosure(closure);
  bindFewArgs(fenv, count, INVOKE_FEW_ARGS_PASS_ARGS);
  p_GeneratorClosure c(closure);
  return invokeClosureCommon(c.get(), fenv);
}

Variant FunctionStatement::invokeClosureFewArgs(
  int count, INVOKE_FEW_ARGS_IMPL_ARGS) const {
  FuncScopeVariableEnvironment fenv(this);
  bindFewArgs(fenv, count, INVOKE_FEW_ARGS_PASS_ARGS);
  DECLARE_THREAD_INFO;
  FrameInjection *fi;
  for (fi = info->m_top; fi; fi= fi->getPrev()) {
    if (fi->isEvalFrame()) {
      break;
    }
  }
  EvalFrameInjection *efi = static_cast<EvalFrameInjection*>(fi);
  c_GeneratorClosure *closure = (c_GeneratorClosure*)efi->getEnv().getClosure();
  return invokeClosureCommon(closure, fenv);
}

Variant FunctionStatement::invokeClosure(ObjectData *closure,
                                         CArrRef params) const {
  ASSERT(closure);
  ASSERT(m_closure);
  FuncScopeVariableEnvironment fenv(this);
  fenv.setClosure(closure);
  bindParams(fenv, params);

  p_GeneratorClosure c(closure);
  return invokeClosureCommon(c.get(), fenv);
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

void FunctionStatement::bindFewArgs(
  FuncScopeVariableEnvironment &fenv, int count, INVOKE_FEW_ARGS_IMPL_ARGS)
  const {
  VariantStack &as = RequestEvalState::argStack();
  const Variant *argp[INVOKE_FEW_ARGS_COUNT];
  ASSERT(count <= 6);
  if (count > 0) argp[0] = &a0;
  if (count > 1) argp[1] = &a1;
  if (count > 2) argp[2] = &a2;
  if (count > 3) argp[3] = &a3;
  if (count > 4) argp[4] = &a4;
  if (count > 5) argp[5] = &a5;
  for (int i = 0; i < count; i++) {
    as.push(*argp[i]);
    fenv.incArgc();
  }
  vector<ParameterPtr>::const_iterator piter = m_params.begin();
  for (int i = 0; i < count && piter != m_params.end();
       ++piter, i++) {
    if ((*piter)->isRef()) {
      (*piter)->bind(fenv, *argp[i], true);
    } else {
      (*piter)->bind(fenv, Variant(*argp[i]));
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
    return strongBind(evalBody(fenv));
  } else {
    return evalBody(fenv);
  }
}

Variant FunctionStatement::invokeImplFewArgs(
  FuncScopeVariableEnvironment &fenv, int count, INVOKE_FEW_ARGS_IMPL_ARGS)
  const {
  bindFewArgs(fenv, count, INVOKE_FEW_ARGS_PASS_ARGS);

  if (m_ref) {
    return strongBind(evalBody(fenv));
  } else {
    return evalBody(fenv);
  }
}

void FunctionStatement::dumpHeader(std::ostream &out) const {
  out << "function " << (m_ref ? "&" : "");
  if (!ParserBase::IsClosureOrContinuationName(m_name->data())) {
    out << m_name->data();
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

Variant FunctionStatement::FSInvoker(void *extra, CArrRef params) {
  ASSERT(extra);
  c_Closure *c = (c_Closure*) extra;
  const FunctionStatement *ms = (const FunctionStatement*) c->extraData();
  if (ms->refReturn()) {
    return strongBind(ms->invokeClosure(c, params));
  }
  return ms->invokeClosure(c, params);
}

Variant FunctionStatement::FSInvokerFewArgs(void *extra, int count,
  INVOKE_FEW_ARGS_IMPL_ARGS) {
  c_Closure *c = (c_Closure*) extra;
  const FunctionStatement *ms = (const FunctionStatement*) c->extraData();
  if (ms->refReturn()) {
    return strongBind(ms->invokeClosureFewArgs(c, count,
                                               INVOKE_FEW_ARGS_PASS_ARGS));
  }
  return ms->invokeClosureFewArgs(c, count, INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant FunctionStatement::Invoker(void *extra, CArrRef params) {
  const Function *f = (const Function*)extra;
  const FunctionStatement *ms = static_cast<const FunctionStatement*>(f);
  if (ms->refReturn()) {
    return strongBind(ms->invoke(params));
  }
  return ms->invoke(params);
}

Variant FunctionStatement::InvokerFewArgs(void *extra, int count,
    INVOKE_FEW_ARGS_IMPL_ARGS) {
  const Function *f = (const Function*)extra;
  const FunctionStatement *ms = static_cast<const FunctionStatement*>(f);
  if (ms->refReturn()) {
    return strongBind(ms->invokeFewArgs(count, INVOKE_FEW_ARGS_PASS_ARGS));
  }
  return ms->invokeFewArgs(count, INVOKE_FEW_ARGS_PASS_ARGS);
}

void Parameter::error(Parser *parser, const char *fmt, ...) const {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  if (parser) {
    parser->error(msg);
  } else {
    raise_error(msg);
  }
}

void Parameter::reportTypeHintError(Parser *parser, const string &hintType)
  const {
  if (m_kind == KindOfArray) {
    error(parser,
          "Default value with array type hint can only be an array or NULL");
  } else if (m_kind == KindOfObject) {
    bool isHipHopTypeHint = false;
    bool isHipHopExperimentalTypeHint = false;
    if (GetHipHopTypeHintTypes().find(hintType) !=
        GetHipHopTypeHintTypes().end()) {
      isHipHopTypeHint = true;
    } else if (GetHipHopExperimentalTypeHintTypes().find(hintType) !=
               GetHipHopExperimentalTypeHintTypes().end()) {
      isHipHopExperimentalTypeHint = true;
    }
    if (!RuntimeOption::EnableHipHopSyntax && isHipHopTypeHint) {
      error(parser,
            "HipHop type hint %s is not enabled", hintType.c_str());
    } else if (!RuntimeOption::EnableHipHopExperimentalSyntax &&
               isHipHopExperimentalTypeHint) {
      error(parser, "HipHop experimental type hint %s is not enabled",
            hintType.c_str());
    } else {
      error(parser, "Default value with a class type hint can only be NULL");
    }
  } else {
    ASSERT(RuntimeOption::EnableHipHopSyntax ||
           RuntimeOption::EnableHipHopExperimentalSyntax);
    error(parser, "Default value need to have the same type as the type hint");
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}

