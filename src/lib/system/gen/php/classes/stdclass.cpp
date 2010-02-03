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

#include <php/classes/stdclass.h>
#include <cpp/ext/ext.h>
#include <cpp/eval/eval.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* preface starts */
/* preface finishes */
/* SRC: classes/stdclass.php line 4 */
Variant c_stdclass::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c_stdclass::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c_stdclass::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c_stdclass::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c_stdclass::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c_stdclass::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c_stdclass::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c_stdclass::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(stdclass)
ObjectData *c_stdclass::cloneImpl() {
  c_stdclass *obj = NEW(c_stdclass)();
  cloneSet(obj);
  return obj;
}
void c_stdclass::cloneSet(c_stdclass *clone) {
  ObjectData::cloneSet(clone);
}
Variant c_stdclass::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c_stdclass::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c_stdclass::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c_stdclass::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c_stdclass::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw_stdclass$os_get(const char *s) {
  return c_stdclass::os_get(s, -1);
}
Variant &cw_stdclass$os_lval(const char *s) {
  return c_stdclass::os_lval(s, -1);
}
Variant cw_stdclass$os_constant(const char *s) {
  return c_stdclass::os_constant(s);
}
Variant cw_stdclass$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c_stdclass::os_invoke(c, s, params, -1, fatal);
}
void c_stdclass::init() {
}
/* SRC: classes/stdclass.php line 8 */
Variant c___php_incomplete_class::os_get(const char *s, int64 hash) {
  return c_ObjectData::os_get(s, hash);
}
Variant &c___php_incomplete_class::os_lval(const char *s, int64 hash) {
  return c_ObjectData::os_lval(s, hash);
}
void c___php_incomplete_class::o_get(ArrayElementVec &props) const {
  c_ObjectData::o_get(props);
}
bool c___php_incomplete_class::o_exists(CStrRef s, int64 hash) const {
  return c_ObjectData::o_exists(s, hash);
}
Variant c___php_incomplete_class::o_get(CStrRef s, int64 hash) {
  return c_ObjectData::o_get(s, hash);
}
Variant c___php_incomplete_class::o_set(CStrRef s, int64 hash, CVarRef v,bool forInit /* = false */) {
  return c_ObjectData::o_set(s, hash, v, forInit);
}
Variant &c___php_incomplete_class::o_lval(CStrRef s, int64 hash) {
  return c_ObjectData::o_lval(s, hash);
}
Variant c___php_incomplete_class::os_constant(const char *s) {
  return c_ObjectData::os_constant(s);
}
IMPLEMENT_CLASS(__php_incomplete_class)
ObjectData *c___php_incomplete_class::cloneImpl() {
  c___php_incomplete_class *obj = NEW(c___php_incomplete_class)();
  cloneSet(obj);
  return obj;
}
void c___php_incomplete_class::cloneSet(c___php_incomplete_class *clone) {
  ObjectData::cloneSet(clone);
}
Variant c___php_incomplete_class::o_invoke(const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::o_invoke(s, params, hash, fatal);
}
Variant c___php_incomplete_class::o_invoke_few_args(const char *s, int64 hash, int count, CVarRef a0, CVarRef a1, CVarRef a2, CVarRef a3, CVarRef a4, CVarRef a5) {
  return c_ObjectData::o_invoke_few_args(s, hash, count, a0, a1, a2, a3, a4, a5);
}
Variant c___php_incomplete_class::os_invoke(const char *c, const char *s, CArrRef params, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke(c, s, params, hash, fatal);
}
Variant c___php_incomplete_class::o_invoke_from_eval(const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::o_invoke_from_eval(s, env, caller, hash, fatal);
}
Variant c___php_incomplete_class::os_invoke_from_eval(const char *c, const char *s, Eval::VariableEnvironment &env, const Eval::FunctionCallExpression *caller, int64 hash, bool fatal) {
  return c_ObjectData::os_invoke_from_eval(c, s, env, caller, hash, fatal);
}
Variant cw___php_incomplete_class$os_get(const char *s) {
  return c___php_incomplete_class::os_get(s, -1);
}
Variant &cw___php_incomplete_class$os_lval(const char *s) {
  return c___php_incomplete_class::os_lval(s, -1);
}
Variant cw___php_incomplete_class$os_constant(const char *s) {
  return c___php_incomplete_class::os_constant(s);
}
Variant cw___php_incomplete_class$os_invoke(const char *c, const char *s, CArrRef params, bool fatal /* = true */) {
  return c___php_incomplete_class::os_invoke(c, s, params, -1, fatal);
}
void c___php_incomplete_class::init() {
}
Object co_stdclass(CArrRef params, bool init /* = true */) {
  return Object(p_stdclass(NEW(c_stdclass)())->dynCreate(params, init));
}
Object co___php_incomplete_class(CArrRef params, bool init /* = true */) {
  return Object(p___php_incomplete_class(NEW(c___php_incomplete_class)())->dynCreate(params, init));
}
Variant pm_php$classes$stdclass_php(bool incOnce /* = false */, LVariableTable* variables /* = NULL */) {
  FUNCTION_INJECTION(run_init::classes/stdclass.php);
  {
    DECLARE_SYSTEM_GLOBALS(g);
    bool &alreadyRun = g->run_pm_php$classes$stdclass_php;
    if (alreadyRun) { if (incOnce) return true;}
    else alreadyRun = true;
    if (!variables) variables = g;
  }
  DECLARE_SYSTEM_GLOBALS(g);
  LVariableTable *gVariables __attribute__((__unused__)) = get_variable_table();
  return true;
} /* function */

///////////////////////////////////////////////////////////////////////////////
}
