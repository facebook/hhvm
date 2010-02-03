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

#ifndef __HPHP_MACROS_H__
#define __HPHP_MACROS_H__

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// class macros

#define FORWARD_DECLARE_CLASS(cls)                      \
  class c_##cls;                                        \
  typedef SmartObject<c_##cls> p_##cls;                 \

#define FORWARD_DECLARE_REDECLARED_CLASS(cls)           \
  class cs_##cls;                                       \

#define BEGIN_CLASS_MAP(cls)                            \
  public:                                               \
  virtual bool o_instanceof(const char *s) const {      \
    if (!s || !*s) return false;                        \
    if (strcasecmp(s, #cls) == 0) return true;          \

#define PARENT_CLASS(parent)                            \
    if (strcasecmp(s, #parent) == 0) return true;       \

#define END_CLASS_MAP(cls)                              \
    return false;                                       \
  }                                                     \

#define INVOKE_FEW_ARGS_COUNT 6

#define INVOKE_FEW_ARGS_DECL3                                           \
  virtual Variant o_invoke_few_args(const char *s, int64 h, int count,  \
                            CVarRef a0 = null_variant,                  \
                            CVarRef a1 = null_variant,                  \
                            CVarRef a2 = null_variant)
#define INVOKE_FEW_ARGS_DECL6                                           \
  virtual Variant o_invoke_few_args(const char *s, int64 h, int count,  \
                            CVarRef a0 = null_variant,                  \
                            CVarRef a1 = null_variant,                  \
                            CVarRef a2 = null_variant,                  \
                            CVarRef a3 = null_variant,                  \
                            CVarRef a4 = null_variant,                  \
                            CVarRef a5 = null_variant)
#define INVOKE_FEW_ARGS_DECL10                                          \
  virtual Variant o_invoke_few_args(const char *s, int64 h, int count,  \
                            CVarRef a0 = null_variant,                  \
                            CVarRef a1 = null_variant,                  \
                            CVarRef a2 = null_variant,                  \
                            CVarRef a3 = null_variant,                  \
                            CVarRef a4 = null_variant,                  \
                            CVarRef a5 = null_variant,                  \
                            CVarRef a6 = null_variant,                  \
                            CVarRef a7 = null_variant,                  \
                            CVarRef a8 = null_variant,                  \
                            CVarRef a9 = null_variant)
#if INVOKE_FEW_ARGS_COUNT == 3
#define INVOKE_FEW_ARGS_DECL INVOKE_FEW_ARGS_DECL3
#elif INVOKE_FEW_ARGS_COUNT == 6
#define INVOKE_FEW_ARGS_DECL INVOKE_FEW_ARGS_DECL6
#elif INVOKE_FEW_ARGS_COUNT == 10
#define INVOKE_FEW_ARGS_DECL INVOKE_FEW_ARGS_DECL10
#endif

#define DECLARE_CLASS(cls, originalName, parent)                        \
  DECLARE_OBJECT_ALLOCATION(c_##cls);                                   \
  public:                                                               \
  static void os_static_initializer();                                  \
  static Variant os_get(const char *s, int64 hash);                     \
  static Variant &os_lval(const char *s, int64 hash);                   \
  static Variant os_constant(const char *s);                            \
  static Variant os_invoke(const char *c, const char *s,                \
                           CArrRef ps, int64 h, bool f = true);         \
  virtual const char *o_getClassName() const { return #originalName;}   \
  virtual bool o_exists(CStrRef s, int64 hash) const;                   \
  virtual void o_get(ArrayElementVec &props) const;                     \
  virtual Variant o_get(CStrRef s, int64 hash);                         \
  virtual Variant o_set(CStrRef s, int64 hash, CVarRef v,               \
                        bool forInit = false);                          \
  virtual Variant &o_lval(CStrRef s, int64 hash);                       \
  virtual Variant o_invoke(const char *s, CArrRef ps, int64 h,          \
                           bool f =true);                               \
  virtual Variant o_invoke_ex(const char *clsname, const char *s,       \
                              CArrRef ps, int64 h, bool f = true) {     \
    if (clsname && strcasecmp(clsname, #cls) == 0) {                    \
      return c_##cls::o_invoke(s, ps, h, f);                            \
    }                                                                   \
    return c_##parent::o_invoke_ex(clsname, s, ps, h, f);               \
  }                                                                     \
  INVOKE_FEW_ARGS_DECL;                                                 \
  protected:                                                            \
  ObjectData *cloneImpl();                                              \
  void cloneSet(c_##cls *cl);                                           \
  public:                                                               \

#define DECLARE_DYNAMIC_CLASS(cls, originalName)                        \
  DECLARE_OBJECT_ALLOCATION(c_##cls);                                   \
  public:                                                               \
  static void os_static_initializer();                                  \
  static Variant os_get(const char *s, int64 hash);                     \
  static Variant &os_lval(const char *s, int64 hash);                   \
  static Variant os_invoke(const char *c, const char *s,                \
                           CArrRef ps, int64 h, bool f = true);         \
  static Variant os_constant(const char *s);                            \
  virtual const char *o_getClassName() const { return #originalName;}   \
  virtual Variant o_invoke(const char *s, CArrRef ps, int64 h,          \
                           bool f = true);                              \
  virtual Variant o_invoke_ex(const char *clsname, const char *s,       \
                              CArrRef ps, int64 h, bool f = true) {     \
    if (clsname && strcasecmp(clsname, #cls) == 0) {                    \
      return o_invoke(s, ps, h, f);                                     \
    }                                                                   \
    return parent->o_invoke_ex(clsname, s, ps, h, f);                   \
  }                                                                     \
  INVOKE_FEW_ARGS_DECL;                                                 \
  protected:                                                            \
  ObjectData *cloneImpl();                                              \
  void cloneSet(c_##cls *cl);                                           \
  public:                                                               \

#define DECLARE_INVOKES_FROM_EVAL                                       \
  static Variant os_invoke_from_eval(const char *c, const char *s,      \
                                     Eval::VariableEnvironment &env,    \
                                     const Eval::FunctionCallExpression *call,\
                                     int64 hash,                        \
                                     bool fatal /* = true */);          \
  Variant o_invoke_from_eval(const char *s,                             \
                             Eval::VariableEnvironment &env,            \
                             const Eval::FunctionCallExpression *call,  \
                             int64 hash,                                \
                             bool fatal /* = true */);

#define IMPLEMENT_CLASS(cls)                                            \
  IMPLEMENT_OBJECT_ALLOCATION(c_##cls);                                 \

#define HASH_GUARD(code, f)                                             \
  if (hash == code && !strcasecmp(s, #f))
#define HASH_EXISTS(code, str)                                          \
  if (hash == code && strcmp(s, #str) == 0) return true
#define HASH_EXISTS_STRING(code, str, len)                              \
  if (hash == code && s.length() == len &&                              \
      memcmp(s.data(), #str, len) == 0) return true
#define HASH_INITIALIZED(code, name, str)                               \
  if (hash == code && strcmp(s, #str) == 0)                             \
    return isInitialized(name)
#define HASH_RETURN(code, name, str)                                    \
  if (hash == code && strcmp(s, #str) == 0) return name
#define HASH_RETURN_STRING(code, name, str, len)                        \
  if (hash == code && s.length() == len &&                              \
      memcmp(s.data(), #str, len) == 0) return name
#define HASH_SET(code, name, str)                                       \
  if (hash == code && strcmp(s, #str) == 0) { name = v; return null;}
#define HASH_SET_STRING(code, name, str, len)                           \
  if (hash == code && s.length() == len &&                              \
      memcmp(s.data(), #str, len) == 0) { name = v; return null; }
#define HASH_INDEX(code, str, index)                                    \
  if (hash == code && strcmp(s, #str) == 0) { return index;}

#define HASH_INVOKE(code, f)                                            \
  if (hash == code && !strcasecmp(s, #f)) return i_ ## f(params)
#define HASH_INVOKE_REDECLARED(code, f)                                 \
  if (hash == code && !strcasecmp(s, #f)) return g->i_ ## f(params)
#define HASH_INVOKE_METHOD(code, f)                                     \
  if (hash == code && !strcasecmp(s, #f)) return o_i_ ## f(params)
#define HASH_INVOKE_CONSTRUCTOR(code, f, id)                            \
  if (hash == code && !strcasecmp(s, #f)) return o_i_ ## id(params)
#define HASH_INVOKE_STATIC_METHOD(code, f)                              \
  if (hash == code && !strcasecmp(s, #f)) \
    return cw_ ## f ## $os_invoke(#f, method, params, fatal)
#define HASH_INVOKE_STATIC_METHOD_REDECLARED(code, f)                   \
  if (hash == code && !strcasecmp(s, #f)) \
    return g->cso_ ## f->os_invoke(#f, method, params, -1, fatal)
#define HASH_GET_STATIC_PROPERTY(code, f)                               \
  if (hash == code && !strcasecmp(s, #f)) return cw_ ## f ## $os_get(prop)
#define HASH_GET_STATIC_PROPERTY_REDECLARED(code, f)                    \
  if (hash == code && !strcasecmp(s, #f)) return g->cso_ ## f->os_get(prop, -1)
#define HASH_GET_STATIC_PROPERTY_LV(code, f)                            \
  if (hash == code && !strcasecmp(s, #f)) return &cw_ ## f ## $os_lval(prop)
#define HASH_GET_STATIC_PROPERTY_LV_REDECLARED(code, f)                 \
  if (hash == code && !strcasecmp(s, #f))                               \
    return &g->cso_ ## f->os_lval(prop, -1)
#define HASH_GET_CLASS_CONSTANT(code, f)                                \
  if (hash == code && !strcasecmp(s, #f))                               \
    return cw_ ## f ## $os_constant(constant)
#define HASH_GET_CLASS_CONSTANT_REDECLARED(code, f)                     \
  if (hash == code && !strcasecmp(s, #f))                               \
    return g->cso_ ## f->os_constant(constant)
#define HASH_CREATE_OBJECT(code, f)                                     \
  if (hash == code && !strcasecmp(s, #f)) return co_ ## f(params, init)
#define HASH_CREATE_OBJECT_REDECLARED(code, f)                          \
  if (hash == code && !strcasecmp(s, #f))                               \
    return g->cso_ ## f->create(params, init, root)
#define HASH_INCLUDE(code, file, fun)                                   \
  if (hash == code && !strcmp(file, s.c_str())) {                       \
    return pm_ ## fun(once, variables);                                 \
  }
#define HASH_INVOKE_FROM_EVAL(code, f)                                  \
  if (hash == code && !strcasecmp(s, #f)) return ei_ ## f(env, caller)
#define HASH_INVOKE_REDECLARED_FROM_EVAL(code, f)                       \
  if (hash == code && !strcasecmp(s, #f)) return g->ei_ ## f(env_caller)

#define INCALL_HELPER(name)                                             \
  class incall {                                                        \
  public:                                                               \
    incall(ObjectData *obj, CStrRef name) : m_obj(obj) {                \
      m_obj->setInCall(name);                                           \
    }                                                                   \
    ~incall() {                                                         \
      m_obj->clearInCall();                                             \
    }                                                                   \
  private:                                                              \
    ObjectData *m_obj;                                                  \
  } incall_helper(this, name);                                          \

///////////////////////////////////////////////////////////////////////////////
// global variable macros

#ifdef DIRECT_GLOBAL_VARIABLES

#define BEGIN_GVS()
#define GVS(s) gv_##s;
#define END_GVS(c)
#define GV(s) s

#else

#define BEGIN_GVS() enum _gv_enums_ {
#define GVS(s) gv_##s,
#define END_GVS(c) }; Variant gv[c];
#define GV(s) gv[GlobalVariables::gv_##s]

#endif

///////////////////////////////////////////////////////////////////////////////
// code instrumentation or injections

#ifdef INFINITE_LOOP_DETECTION
#define LOOP_COUNTER(n) int lc##n = 0;
#define LOOP_COUNTER_CHECK(n) \
  if (++lc##n > 1000000) throw_infinite_loop_exception();
#else
#define LOOP_COUNTER(n)
#define LOOP_COUNTER_CHECK(n)
#endif

#ifdef INFINITE_RECURSION_DETECTION
#define RECURSION_INJECTION RecursionInjection ri;
#else
#define RECURSION_INJECTION
#endif

#ifdef REQUEST_TIMEOUT_DETECTION
#define REQUEST_TIMEOUT_INJECTION RequestInjection ti;
#else
#define REQUEST_TIMEOUT_INJECTION
#endif

#ifdef HOTPROFILER
#define HOTPROFILER_INJECTION(n) ProfilerInjection pi(#n);
#ifndef HOTPROFILER_NO_BUILTIN
#define HOTPROFILER_INJECTION_BUILTIN(n) ProfilerInjection pi(#n);
#else
#define HOTPROFILER_INJECTION_BUILTIN(n)
#endif
#else
#define HOTPROFILER_INJECTION(n)
#define HOTPROFILER_INJECTION_BUILTIN(n)
#endif

#ifdef STACK_FRAME_INJECTION
#define FRAME_INJECTION(c, n) FrameInjection fi(#c, #n);
#define FRAME_INJECTION_WITH_THIS(c, n) FrameInjection fi(#c, #n, this);
#define LINE(n, e) (set_ln(fi.line, n), e)
#else
#define FRAME_INJECTION(c, n)
#define FRAME_INJECTION_WITH_THIS(c, n)
#define LINE(n, e) e
#endif

// code injected into beginning of every function/method
#define FUNCTION_INJECTION(n)                   \
  RECURSION_INJECTION                           \
  REQUEST_TIMEOUT_INJECTION                     \
  HOTPROFILER_INJECTION(n)                      \
  FRAME_INJECTION(, n)                          \

#define STATIC_METHOD_INJECTION(c, n)           \
  RECURSION_INJECTION                           \
  REQUEST_TIMEOUT_INJECTION                     \
  HOTPROFILER_INJECTION(n)                      \
  FRAME_INJECTION(c, n)                         \

#define INSTANCE_METHOD_INJECTION(c, n)         \
  RECURSION_INJECTION                           \
  REQUEST_TIMEOUT_INJECTION                     \
  HOTPROFILER_INJECTION(n)                      \
  FRAME_INJECTION_WITH_THIS(c, n)               \

// code injected into every builtin function/method
#define FUNCTION_INJECTION_BUILTIN(n)           \
  RECURSION_INJECTION                           \
  REQUEST_TIMEOUT_INJECTION                     \
  HOTPROFILER_INJECTION_BUILTIN(n)              \
  FRAME_INJECTION(, n)                          \

#define STATIC_METHOD_INJECTION_BUILTIN(c, n)   \
  RECURSION_INJECTION                           \
  REQUEST_TIMEOUT_INJECTION                     \
  HOTPROFILER_INJECTION_BUILTIN(n)              \
  FRAME_INJECTION(c, n)                         \

#define INSTANCE_METHOD_INJECTION_BUILTIN(c, n) \
  RECURSION_INJECTION                           \
  REQUEST_TIMEOUT_INJECTION                     \
  HOTPROFILER_INJECTION_BUILTIN(n)              \
  FRAME_INJECTION_WITH_THIS(c, n)               \


// for collecting function/method parameter type information at runtime
#define RTTI_INJECTION(v, id)                   \
  do {                                          \
    unsigned int *counter = getRTTICounter(id); \
    if (counter) {                              \
      counter[v.getType()]++;                   \
    }                                           \
  } while (0)

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_MACROS_H__
