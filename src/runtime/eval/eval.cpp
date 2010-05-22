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
  String code_str2 = prepend_php ? concat("<?php ", code_str) : code_str;
  Eval::StatementPtr s = Eval::Parser::parseString(code_str2.data(), statics);
  Block blk(statics);
  StatementListStatementPtr sl = s->cast<StatementListStatement>();
  if (sl) {
    const std::vector<Eval::StatementPtr> &stmts = sl->stmts();
    std::vector<Eval::ClassStatementPtr> cls;
    std::vector<Eval::FunctionStatementPtr> funcs;
    for (std::vector<Eval::StatementPtr>::const_iterator it = stmts.begin();
         it != stmts.end(); ++it) {
      Eval::ClassStatementPtr cl = (*it)->cast<Eval::ClassStatement>();
      if (cl) {
        cls.push_back(cl);
      } else {
        Eval::FunctionStatementPtr fn = (*it)->cast<Eval::FunctionStatement>();
        if (fn) {
          funcs.push_back(fn);
        } else {
          // Classes and funcs are all at the start
          break;
        }
      }
    }
    if (cls.size() > 0 || funcs.size() > 0) {
      // install string code container to globals
      StringCodeContainer *scc =
        new StringCodeContainer(cls, funcs);
      RequestEvalState::addCodeContainer(scc);
    }
  }
  // todo: pass in params
  NestedVariableEnvironment env(vars, blk, Array(), self);
  s->eval(env);
  return true;
}

bool eval_invoke_hook(Variant &res, const char *s, CArrRef params, int64 hash) {
  const Eval::Function *fs = Eval::RequestEvalState::findFunction(s);
  if (fs) {
    res = ref(fs->invoke(params));
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
bool eval_try_autoload(const char *s) {
  const Eval::Function *fn =
    Eval::RequestEvalState::findFunction("__autoload");
  if (fn) {
    fn->invoke(CREATE_VECTOR1(String(s, AttachLiteral)));
    return true;
  }
  return false;
}
bool eval_invoke_static_method_hook(Variant &res, const char *s,
                                    const char* method, CArrRef params,
                                    bool &foundClass) {
  const MethodStatement *ms = Eval::RequestEvalState::findMethod(s, method,
                                                                 foundClass,
                                                                 true);
  if (ms) {
    res = ref(ms->invokeStatic(s, params));
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
  while (cls) {
    LVariableTable *statics = Eval::RequestEvalState::getClassStatics(cls);
    if (!statics) return false;
    if (statics->exists(prop)) {
      cls->attemptPropertyAccess(NULL, prop,
                                 FrameInjection::GetClassName(false));
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
///////////////////////////////////////////////////////////////////////////////
}
