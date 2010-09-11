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

#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/ast/statement_list_statement.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/ast/function_call_expression.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

MethodStatement::MethodStatement(STATEMENT_ARGS, const string &name,
                                 const ClassStatement *cls, int modifiers,
                                 const string &doc)
  : FunctionStatement(STATEMENT_PASS, name, doc), m_class(cls),
    m_modifiers(modifiers) {
  m_fullName = cls->name() + "::" + name;
  if ((m_modifiers & (ClassStatement::Public | ClassStatement::Protected |
                      ClassStatement::Private)) == 0) {
    m_modifiers |= ClassStatement::Public;
  }
}

const string &MethodStatement::fullName() const {
  return m_fullName;
}

void MethodStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  // register with reflection, invoke, etc.
}

LVariableTable *MethodStatement::getStaticVars(VariableEnvironment &env)
  const {
  return &RequestEvalState::getMethodStatics(this, env.currentClass());
}

Variant MethodStatement::invokeInstance(CObjRef obj, CArrRef params,
    bool check /* = true */) const {
  if (getModifiers() & ClassStatement::Static) {
    return invokeStatic(obj->o_getClassName(), params);
  }
  if (check) attemptAccess(FrameInjection::GetClassName(false));
  // The debug frame should have been pushed at ObjectMethodExpression
  DECLARE_THREAD_INFO
  RECURSION_INJECTION
  REQUEST_TIMEOUT_INJECTION
#ifdef HOTPROFILER
  ProfilerInjection pi(info, m_fullName.c_str());
#endif
  MethScopeVariableEnvironment env(this, params.size());
  env.setCurrentObject(obj);
  String clsName(m_class->name().c_str(), m_class->name().size(),
                 AttachLiteral);
  EvalFrameInjection fi(clsName, m_fullName.c_str(), env,
                        loc()->file, obj.get());
  if (m_ref) {
    return ref(invokeImpl(env, params));
  }
  return invokeImpl(env, params);
}

Variant MethodStatement::
invokeInstanceDirect(CObjRef obj, VariableEnvironment &env,
                     const FunctionCallExpression *caller) const {
  if (getModifiers() & ClassStatement::Static) {
    return invokeStaticDirect(obj->o_getClassName(), env, caller);
  }
  attemptAccess(FrameInjection::GetClassName(false));
  DECLARE_THREAD_INFO
  RECURSION_INJECTION
  REQUEST_TIMEOUT_INJECTION
#ifdef HOTPROFILER
  ProfilerInjection pi(info, m_fullName.c_str());
#endif
  MethScopeVariableEnvironment fenv(this, 0);
  directBind(env, caller, fenv);
  fenv.setCurrentObject(obj);
  String clsName(m_class->name().c_str(), m_class->name().size(),
                 AttachLiteral);
  EvalFrameInjection fi(clsName, m_fullName.c_str(), fenv,
                        loc()->file, obj.get());
  if (m_ref) {
    return ref(evalBody(fenv));
  }
  return evalBody(fenv);
}

Variant MethodStatement::invokeStatic(const char* cls, CArrRef params,
    bool check /* = true */) const {
  if (check) attemptAccess(FrameInjection::GetClassName(false));
  DECLARE_THREAD_INFO
  RECURSION_INJECTION
  REQUEST_TIMEOUT_INJECTION
#ifdef HOTPROFILER
  ProfilerInjection pi(info, m_fullName.c_str());
#endif
  MethScopeVariableEnvironment env(this, params.size());
  env.setCurrentClass(cls);
  String clsName(m_class->name().c_str(), m_class->name().size(),
                 AttachLiteral);
  EvalFrameInjection fi(clsName, m_fullName.c_str(), env, loc()->file);
  if (m_ref) {
    return ref(invokeImpl(env, params));
  }
  return invokeImpl(env, params);
}

Variant MethodStatement::
invokeStaticDirect(const char* cls, VariableEnvironment &env,
                   const FunctionCallExpression *caller)
  const {
  attemptAccess(FrameInjection::GetClassName(false));
  MethScopeVariableEnvironment fenv(this, 0);
  directBind(env, caller, fenv);
  fenv.setCurrentClass(cls);
  DECLARE_THREAD_INFO
  RECURSION_INJECTION
  REQUEST_TIMEOUT_INJECTION
#ifdef HOTPROFILER
  ProfilerInjection pi(info, m_fullName.c_str());
#endif
  String clsName(m_class->name().c_str(), m_class->name().size(),
                 AttachLiteral);
  EvalFrameInjection fi(clsName, m_fullName.c_str(), fenv,
                        loc()->file);
  if (m_ref) {
    return ref(evalBody(fenv));
  }
  return evalBody(fenv);
}

Variant MethodStatement::evalBody(VariableEnvironment &env) const {
  if (isAbstract()) {
    raise_error("Cannot call abstract method %s()", m_fullName.c_str());
  }
  if (m_ref) {
    return ref(FunctionStatement::evalBody(env));
  } else {
    return FunctionStatement::evalBody(env);
  }
}

void MethodStatement::getInfo(ClassInfo::MethodInfo &info) const {
  FunctionStatement::getInfo(info);
  int attr = info.attribute == ClassInfo::IsNothing ? 0 : info.attribute;
  if (m_modifiers & ClassStatement::Abstract) attr |= ClassInfo::IsAbstract;
  if (m_modifiers & ClassStatement::Final) attr |= ClassInfo::IsFinal;
  if (m_modifiers & ClassStatement::Protected) attr |= ClassInfo::IsProtected;
  if (m_modifiers & ClassStatement::Private) attr |= ClassInfo::IsPrivate;
  if (m_modifiers & ClassStatement::Static) attr |= ClassInfo::IsStatic;
  if (!(attr & ClassInfo::IsProtected || attr & ClassInfo::IsPrivate)) {
    attr |= ClassInfo::IsPublic;
  }
  info.attribute = (ClassInfo::Attribute)attr;
}

void MethodStatement::attemptAccess(const char *context) const {
  int mods = getModifiers();
  const ClassStatement *cs = getClass();
  ClassStatement::Modifier level = ClassStatement::Public;
  if (mods & ClassStatement::Private) level = ClassStatement::Private;
  else if (mods & ClassStatement::Protected) level = ClassStatement::Protected;
  bool access = true;
  if (level == ClassStatement::Protected) {
    while (!cs->hasAccess(context, level)) {
      while ((cs = cs->parentStatement())) {
        if (cs->findMethod(m_name.c_str())) {
          break;
        }
      }
      if (!cs) {
        access = false;
        break;
      }
    }
  } else {
    access = cs->hasAccess(context, level);
  }
  if (!access) {
    const char *mod = "protected";
    if (level == ClassStatement::Private) mod = "private";
    throw FatalErrorException(0, "Attempt to call %s %s::%s()%s%s",
                              mod, getClass()->name().c_str(), m_name.c_str(),
                              context[0] ? " from " : "",
                              context[0] ? context : "");
  }
}

bool MethodStatement::isAbstract() const {
  return getModifiers() & ClassStatement::Abstract ||
    m_class->getModifiers() & ClassStatement::Interface;
}

///////////////////////////////////////////////////////////////////////////////
}
}

