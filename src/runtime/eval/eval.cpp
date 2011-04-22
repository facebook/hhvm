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

#include <runtime/eval/eval.h>
#include <runtime/base/hphp_system.h>
#include <runtime/eval/base/function.h>
#include <runtime/eval/ast/statement_list_statement.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/parser/parser.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/ast/static_statement.h>

namespace HPHP {

using namespace Eval;
using namespace std;
///////////////////////////////////////////////////////////////////////////////

Variant eval(LVariableTable *vars, CObjRef self, CStrRef code_str,
             bool prepend_php /* = true */) {
  vector<StaticStatementPtr> statics;
  Block::VariableIndices variableIndices;
  String code_str2 = prepend_php ? concat("<?php ", code_str) : code_str;
  Eval::StatementPtr s = Eval::Parser::ParseString(code_str2.data(), statics,
                                                   variableIndices);
  Block blk(statics, variableIndices);
  // install string code container to globals
  SmartPtr<CodeContainer> scc = new StringCodeContainer(s);
  RequestEvalState::addCodeContainer(scc);
  // todo: pass in params
  NestedVariableEnvironment env(vars, blk, Array(), self);
  EvalFrameInjection fi(empty_string, "_", env, NULL, NULL,
                        FrameInjection::PseudoMain);
  s->eval(env);
  if (env.isReturning()) {
    return env.getRet();
  }
  return true;
}

bool eval_invoke_hook(Variant &res, const char *s, CArrRef params, int64 hash) {
  const Eval::Function *fs = Eval::RequestEvalState::findFunction(s);
  if (fs) {
    res.assignRef(fs->invoke(params));
    ref(res);
    return true;
  }
  return false;
}
bool eval_get_class_var_init_hook(Variant &res, const char *s,
                                  const char *var) {
  Eval::ClassEvalState *ce = Eval::RequestEvalState::findClassState(s, true);
  if (ce) {
    const Eval::ClassVariable *v = ce->getClass()->findVariable(var, true);
    if (v) {
      DummyVariableEnvironment env;
      v->eval(env, res);
      return true;
    }
  }
  return false;
}
bool eval_create_object_hook(Variant &res, const char *s, CArrRef params,
                             bool init, ObjectData *root) {
  Eval::ClassEvalState *ce = Eval::RequestEvalState::findClassState(s, true);
  if (ce) {
    res = ce->getClass()->create(*ce, params, init, root);
    return true;
  }
  return false;
}
bool eval_create_object_only_hook(Variant &res, const char *s,
    ObjectData *root) {
  Eval::ClassEvalState *ce = Eval::RequestEvalState::findClassState(s, true);
  if (ce) {
    res = ce->getClass()->create(*ce, null_array, false, root);
    return true;
  }
  return false;
}
bool eval_try_autoload(const char *s) {
  return AutoloadHandler::s_instance->invokeHandler(String(s, CopyString),
                                                    true);
}
bool eval_invoke_static_method_hook(Variant &res, const char *s,
                                    const char* method, CArrRef params,
                                    bool &foundClass) {
  const MethodStatement *ms = Eval::RequestEvalState::findMethod(s, method,
                                                                 foundClass,
                                                                 true);
  if (ms) {
    res.assignRef(ms->invokeStatic(s, params));
    ref(res);
    return true;
  }
  return false;
}
bool eval_get_static_property_hook(Variant &res, const char *s,
                                   const char* prop) {
  Variant *v;
  if (eval_get_static_property_lv_hook(v, s, prop)) {
    res = *v;
    return true;
  }
  return false;
}
bool eval_get_static_property_lv_hook(Variant *&res, const char *s,
                                      const char *prop) {
  const Eval::ClassStatement *cls = Eval::RequestEvalState::findClass(s, true);
  const Eval::ClassStatement *starter = cls;
  while (cls) {
    LVariableTable *statics = Eval::RequestEvalState::getClassStatics(cls);
    if (!statics) return false;
    if (statics->exists(prop)) {
      const char *context = FrameInjection::GetClassName(false);
      int mods;
      if (!starter->attemptPropertyAccess(prop, context, mods)) {
        starter->failPropertyAccess(prop, context, mods);
      }
      res = &statics->get(prop);
      return true;
    }
    cls = cls->parentStatement();
  }
  return false;
}
bool eval_get_class_constant_hook(Variant &res, const char *s,
                                  const char* constant) {
  const Eval::ClassStatement *cls = Eval::RequestEvalState::findClass(s, true);
  if (cls) {
    return cls->getConstant(res, constant, true);
  }
  return false;
}
bool eval_constant_hook(Variant &res, CStrRef name) {
  return Eval::RequestEvalState::findConstant(name, res);
}
bool eval_invoke_file_hook(Variant &res, CStrRef path, bool once,
                           LVariableTable* variables, const char *currentDir) {
  return RequestEvalState::includeFile(res, path, once, variables, currentDir);
}

void eval_get_method_static_variables(Array &arr) {
  Eval::RequestEvalState::GetMethodStaticVariables(arr);
}
void eval_get_class_static_variables(Array &arr) {
  Eval::RequestEvalState::GetClassStaticVariables(arr);
}
void eval_get_dynamic_constants(Array &arr) {
  Eval::RequestEvalState::GetDynamicConstants(arr);
}

bool eval_get_call_info_hook(const CallInfo *&ci, void *&extra, const char *s,
                             int64 hash /* = -1 */) {
  const Eval::Function *fs = Eval::RequestEvalState::findFunction(s);
  if (fs) {
    ci = fs->getCallInfo();
    if (extra) {
      DECLARE_THREAD_INFO;
      FrameInjection *fi;
      for (fi = info->m_top; fi; fi= fi->getPrev()) {
        if (fi->isEvalFrame()) {
          break;
        }
      }
      EvalFrameInjection *efi = static_cast<EvalFrameInjection*>(fi);
      efi->getEnv().setClosure(extra);
    }
    extra = (void*)fs;
    return true;
  }
  return false;
}

bool eval_get_call_info_static_method_hook(MethodCallPackage &info,
                                           bool &foundClass) {
  const char *s __attribute__((__unused__)) = info.rootCls->data();
  const MethodStatement *ms = Eval::RequestEvalState::findMethod(s,
      info.name->data(), foundClass, true);
  if (ms) {
    info.ci = ms->getCallInfo();
    info.extra = (void*)ms;
    return true;
  }

  if (foundClass) {
    return ObjectData::os_get_call_info(info);
  }
  return false;
}

String Eval::location_to_string(const Location *loc) {
  StringBuffer buf;
  buf.printf("%s:%d:%d", loc->file, loc->line1, loc->char1);
  return buf.detach();
}

///////////////////////////////////////////////////////////////////////////////
}
