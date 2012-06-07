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
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/ast/statement_list_statement.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/ast/function_call_expression.h>

namespace HPHP {
namespace Eval {

///////////////////////////////////////////////////////////////////////////////

static StaticString s___construct("__construct");
static StaticString s___invoke("__invoke");

MethodStatement::MethodStatement(STATEMENT_ARGS, const string &name,
                                 const ClassStatement *cls, int modifiers,
                                 const string &doc)
  : FunctionStatement(STATEMENT_PASS, name, doc), m_class(cls),
    m_modifiers(modifiers),
    m_fullName(StringData::GetStaticString(
               string(cls->name().c_str()) + "::" + name)) {
  if ((m_modifiers & (ClassStatement::Public | ClassStatement::Protected |
                      ClassStatement::Private)) == 0) {
    m_modifiers |= ClassStatement::Public;
  }
  m_callInfo.m_invoker = (void*)MethInvoker;
  m_callInfo.m_invokerFewArgs = (void*)MethInvokerFewArgs;
  if (m_modifiers & ClassStatement::Static) {
    m_callInfo.m_flags |= CallInfo::StaticMethod;
  } else {
    m_callInfo.m_flags |= CallInfo::Method;
  }
  if (m_modifiers & ClassStatement::Protected) {
    m_callInfo.m_flags |= CallInfo::Protected;
  } else if (m_modifiers & ClassStatement::Private) {
    m_callInfo.m_flags |= CallInfo::Private;
  }
}

void MethodStatement::setPublic() {
  m_modifiers = ClassStatement::Public;
}

void MethodStatement::setConstructor() {
  m_modifiers |= ClassStatement::Constructor;
}

String MethodStatement::fullName() const {
  if (m_class->isTrait()) {
    String fName = StringData::GetStaticString(
      FrameInjection::GetClassName(false) + "::" + name());
    return fName;
  }
  return m_fullName;
}

void MethodStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  ENTER_STMT;
  // register with reflection, invoke, etc.
}

LVariableTable *MethodStatement::getStaticVars(VariableEnvironment &env)
  const {
  return &RequestEvalState::getMethodStatics(this, env.currentClass(),
                                             env.currentAlias());
}

static String get_current_alias() {
  DECLARE_THREAD_INFO;
  FrameInjection *fi;
  for (fi = info->m_top; fi; fi= fi->getPrev()) {
    if (fi->isEvalFrame()) {
      break;
    }
  }
  EvalFrameInjection *efi = static_cast<EvalFrameInjection*>(fi);
  return (efi) ? efi->getEnv().getCalleeAlias() : String();
}

Variant MethodStatement::invokeInstance(CObjRef obj, CArrRef params,
  const MethodStatementWrapper *msw, bool check /* = true */) const {
  ASSERT(msw->m_methodStatement == this);
  if (getModifiers() & ClassStatement::Static) {
    return invokeStatic(obj->o_getClassName(), params, msw, check);
  }
  if (check) attemptAccess(FrameInjection::GetClassName(false), msw);
  // The debug frame should have been pushed at ObjectMethodExpression
  DECLARE_THREAD_INFO_NOINIT
  MethScopeVariableEnvironment env(this);
  env.setCurrentObject(obj);
  env.setCurrentAlias(get_current_alias());
  EvalFrameInjection fi(msw->m_className, m_fullName->data(), env,
                        loc()->file, obj.get(), FrameInjection::ObjectMethod);
  if (m_ref) {
    return strongBind(invokeImpl(env, params));
  }
  return invokeImpl(env, params);
}

Variant MethodStatement::invokeInstanceFewArgs(CObjRef obj, int count,
  INVOKE_FEW_ARGS_IMPL_ARGS, const MethodStatementWrapper *msw, bool check)
  const {
  ASSERT(msw->m_methodStatement == this);
  if (getModifiers() & ClassStatement::Static) {
    return invokeStaticFewArgs(obj->o_getClassName(), count,
                               INVOKE_FEW_ARGS_PASS_ARGS, msw, check);
  }
  if (check) attemptAccess(FrameInjection::GetClassName(false), msw);
  // The debug frame should have been pushed at ObjectMethodExpression
  DECLARE_THREAD_INFO_NOINIT
  MethScopeVariableEnvironment env(this);
  env.setCurrentObject(obj);
  env.setCurrentAlias(get_current_alias());
  EvalFrameInjection fi(msw->m_className, m_fullName->data(), env,
                        loc()->file, obj.get(), FrameInjection::ObjectMethod);
  if (m_ref) {
    return strongBind(invokeImplFewArgs(env, count, INVOKE_FEW_ARGS_PASS_ARGS));
  }
  return invokeImplFewArgs(env, count, INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant MethodStatement::
invokeInstanceDirect(CObjRef obj, CStrRef alias, VariableEnvironment &env,
                     const FunctionCallExpression *caller,
                     const MethodStatementWrapper *msw,
                     bool check /* = true */) const {
  ASSERT(msw->m_methodStatement == this);
  if (getModifiers() & ClassStatement::Static) {
    return invokeStaticDirect(obj->o_getClassName(), alias, env,
                              caller, false, msw, check);
  }
  if (check) attemptAccess(FrameInjection::GetClassName(false), msw);
  DECLARE_THREAD_INFO_NOINIT
  MethScopeVariableEnvironment fenv(this);
  directBind(env, caller, fenv);
  fenv.setCurrentObject(obj);
  fenv.setCurrentAlias(alias);
  EvalFrameInjection::EvalStaticClassNameHelper helper(obj);
  EvalFrameInjection fi(msw->m_className, m_fullName->data(), fenv,
                        loc()->file, obj.get(), FrameInjection::ObjectMethod);
  if (m_ref) {
    return strongBind(evalBody(fenv));
  }
  return evalBody(fenv);
}

Variant MethodStatement::invokeStatic(CStrRef cls, CArrRef params,
  const MethodStatementWrapper *msw, bool check /* = true */) const {
  ASSERT(msw->m_methodStatement == this);
  if (check) attemptAccess(FrameInjection::GetClassName(false), msw);
  DECLARE_THREAD_INFO_NOINIT
  MethScopeVariableEnvironment env(this);
  env.setCurrentClass(cls);
  env.setCurrentAlias(get_current_alias());
  EvalFrameInjection fi(msw->m_className, m_fullName->data(), env, loc()->file,
                        NULL, FrameInjection::StaticMethod);
  if (m_ref) {
    return strongBind(invokeImpl(env, params));
  }
  return invokeImpl(env, params);
}

Variant MethodStatement::invokeStaticFewArgs(CStrRef cls, int count,
  INVOKE_FEW_ARGS_IMPL_ARGS, const MethodStatementWrapper *msw, bool check)
  const {
  ASSERT(msw->m_methodStatement == this);
  if (check) attemptAccess(FrameInjection::GetClassName(false), msw);
  DECLARE_THREAD_INFO_NOINIT
  MethScopeVariableEnvironment env(this);
  env.setCurrentClass(cls);
  env.setCurrentAlias(get_current_alias());
  EvalFrameInjection fi(msw->m_className, m_fullName->data(), env, loc()->file,
                        NULL, FrameInjection::StaticMethod);
  if (m_ref) {
    return strongBind(invokeImplFewArgs(env, count, INVOKE_FEW_ARGS_PASS_ARGS));
  }
  return invokeImplFewArgs(env, count, INVOKE_FEW_ARGS_PASS_ARGS);
}

Variant MethodStatement::
invokeStaticDirect(CStrRef cls, CStrRef alias, VariableEnvironment &env,
                   const FunctionCallExpression *caller, bool sp,
                   const MethodStatementWrapper *msw,
                   bool check /* = true */)
  const {
  ASSERT(msw->m_methodStatement == this);
  if (check) attemptAccess(FrameInjection::GetClassName(false), msw);
  MethScopeVariableEnvironment fenv(this);
  directBind(env, caller, fenv);
  fenv.setCurrentClass(cls);
  fenv.setCurrentAlias(alias);
  EvalFrameInjection::EvalStaticClassNameHelper helper(cls, sp);
  DECLARE_THREAD_INFO_NOINIT
  EvalFrameInjection fi(msw->m_className, m_fullName->data(), fenv,
                        loc()->file, NULL, FrameInjection::StaticMethod);
  if (m_ref) {
    return strongBind(evalBody(fenv));
  }
  return evalBody(fenv);
}

Variant MethodStatement::evalBody(VariableEnvironment &env) const {
  if (isAbstract()) {
    raise_error("Cannot call abstract method %s()", m_fullName->data());
  }
  if (m_ref) {
    return strongBind(FunctionStatement::evalBody(env));
  } else {
    return FunctionStatement::evalBody(env);
  }
}

void MethodStatement::getInfo(ClassInfo::MethodInfo &info,
                              int access /* = 0 */) const {
  FunctionStatement::getInfo(info);
  int attr = info.attribute == ClassInfo::IsNothing ? 0 : info.attribute;
  int mods = m_modifiers;
  if (access) {
    mods &= ~ClassStatement::AccessMask;
    mods |= access;
  }
  if (mods & ClassStatement::Abstract) attr |= ClassInfo::IsAbstract;
  if (mods & ClassStatement::Final) attr |= ClassInfo::IsFinal;
  if (mods & ClassStatement::Protected) attr |= ClassInfo::IsProtected;
  if (mods & ClassStatement::Private) attr |= ClassInfo::IsPrivate;
  if (mods & ClassStatement::Static) attr |= ClassInfo::IsStatic;
  if (!(attr & ClassInfo::IsProtected || attr & ClassInfo::IsPrivate)) {
    attr |= ClassInfo::IsPublic;
  }
  if (mods & ClassStatement::Constructor) attr |= ClassInfo::IsConstructor;
  info.attribute = (ClassInfo::Attribute)attr;
}

void MethodStatement::attemptAccess(CStrRef context,
  const MethodStatementWrapper *msw) const {
  ASSERT(msw->m_methodStatement == this);
  if (g_context->getDebuggerBypassCheck()) {
    return;
  }
  int mods = msw->m_access ? msw->m_access : getModifiers();
  const ClassStatement *cs = m_class;
  if (cs->isTrait()) {
    cs = RequestEvalState::findClass(msw->m_className);
  }
  ClassStatement::Modifier level = ClassStatement::Public;
  if (mods & ClassStatement::Private) level = ClassStatement::Private;
  else if (mods & ClassStatement::Protected) level = ClassStatement::Protected;
  bool ok = true;
  if (level == ClassStatement::Protected) {
    while (!cs->hasAccess(context, level)) {
      while ((cs = cs->parentStatement())) {
        if (cs->findMethod(m_name)) {
          break;
        }
      }
      if (!cs) {
        ok = false;
        break;
      }
    }
  } else {
    ok = cs->hasAccess(context, level);
  }

  if (!ok) {
    const char *mod = "protected";
    if (level == ClassStatement::Private) mod = "private";
    throw FatalErrorException(0, "Attempt to call %s %s::%s()%s%s",
                              mod, getClass()->name().c_str(), m_name->data(),
                              context->data()[0] ? " from " : "",
                              context->data()[0] ? context->data() : "");
  }
}

bool MethodStatement::isAbstract() const {
  return getModifiers() & ClassStatement::Abstract ||
    m_class->getModifiers() & ClassStatement::Interface;
}

Variant MethodStatement::MethInvoker(MethodCallPackage &mcp, CArrRef params) {
  const MethodStatementWrapper *msw = (const MethodStatementWrapper*)mcp.extra;
  const MethodStatement *ms = msw->m_methodStatement;
  bool check = !ms->m_name->isame(s___invoke.get());
  bool isStatic = ms->getModifiers() & ClassStatement::Static;
  if (isStatic || !mcp.obj) {
    String cn;
    if (UNLIKELY(!isStatic && mcp.isObj && mcp.obj == NULL)) {
      // this is needed for continuations where
      // we are passed the dummy object
      cn = ms->getClass()->name();
    } else {
      cn = mcp.getClassName();
    }
    if (ms->refReturn()) {
      return strongBind(ms->invokeStatic(cn, params, msw, check));
    } else {
      return ms->invokeStatic(cn, params, msw, check);
    }
  } else {
    if (ms->refReturn()) {
      return strongBind(ms->invokeInstance(mcp.rootObj, params, msw, check));
    } else {
      return ms->invokeInstance(mcp.rootObj, params, msw, check);
    }
  }
}

Variant MethodStatement::MethInvokerFewArgs(MethodCallPackage &mcp,
    int count, INVOKE_FEW_ARGS_IMPL_ARGS) {
  const MethodStatementWrapper *msw = (const MethodStatementWrapper*)mcp.extra;
  const MethodStatement *ms = msw->m_methodStatement;
  bool check = !ms->m_name->isame(s___invoke.get());
  bool isStatic = ms->getModifiers() & ClassStatement::Static;
  if (isStatic || !mcp.obj) {
    String cn;
    if (UNLIKELY(!isStatic && mcp.isObj && mcp.obj == NULL)) {
      // this is needed for continuations where
      // we are passed the dummy object
      cn = ms->getClass()->name();
    } else {
      cn = mcp.getClassName();
    }
    if (ms->refReturn()) {
      return strongBind(ms->invokeStaticFewArgs(cn, count,
        INVOKE_FEW_ARGS_PASS_ARGS, msw, check));
    } else {
      return ms->invokeStaticFewArgs(cn, count,
        INVOKE_FEW_ARGS_PASS_ARGS, msw, check);
    }
  } else {
    if (ms->refReturn()) {
      return strongBind(ms->invokeInstanceFewArgs(mcp.rootObj, count,
        INVOKE_FEW_ARGS_PASS_ARGS, msw, check));
    } else {
      return ms->invokeInstanceFewArgs(mcp.rootObj, count,
        INVOKE_FEW_ARGS_PASS_ARGS, msw, check);
    }
  }
}

void MethodStatement::dump(std::ostream &out) const {
  ClassStatement::dumpModifiers(out, m_modifiers, false);
  FunctionStatement::dump(out);
}

///////////////////////////////////////////////////////////////////////////////
}
}

