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

#ifndef __HPHP_MACROS_H__
#define __HPHP_MACROS_H__

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// class macros

#define LITSTR_INIT(str)    (true ? (str) : ("" str "")), (sizeof(str)-1)
#define LITSTR(index, str)  (literalStrings[index])
#define NAMSTR(nam, str)    (nam)

#define GET_THIS()         fi.getThis()
#define GET_THIS_TYPED(T)  p_##T(fi.getThis())
#define GET_THIS_VALID()   fi.getThisForArrow()
#define GET_THIS_ARROW()   fi.getThisForArrow()->

#define FORWARD_DECLARE_CLASS(cls)                      \
  class c_##cls;                                        \
  typedef SmartObject<c_##cls> p_##cls;                 \

#define FORWARD_DECLARE_INTERFACE(cls)                  \
  class c_##cls;                                        \
  typedef SmartInterface<c_##cls> p_##cls               \

#define FORWARD_DECLARE_GENERIC_INTERFACE(cls)          \
  class c_##cls;                                        \
  typedef Object               p_##cls                  \

#define BEGIN_CLASS_MAP(cls)                            \
  public:                                               \
  virtual bool o_instanceof(CStrRef s) const {          \
    if (strcasecmp(s.data(), #cls) == 0) return true;   \

#define PARENT_CLASS(parent)                            \
    if (strcasecmp(s.data(), #parent) == 0) return true;\

#define CLASS_MAP_REDECLARED()                          \
    if (parent->o_instanceof(s)) return true;           \

#define RECURSIVE_PARENT_CLASS(parent)                  \
    if (strcasecmp(s.data(), #parent) == 0) return true;\
    if (c_##parent::o_instanceof(s)) return true;       \

#define END_CLASS_MAP(cls)                              \
    return false;                                       \
  }                                                     \

#define INVOKE_FEW_ARGS_COUNT 6

#define INVOKE_FEW_ARGS_DECL3                                           \
                            CVarRef a0 = null_variant,                  \
                            CVarRef a1 = null_variant,                  \
                            CVarRef a2 = null_variant
#define INVOKE_FEW_ARGS_DECL6                                           \
                            INVOKE_FEW_ARGS_DECL3,                      \
                            CVarRef a3 = null_variant,                  \
                            CVarRef a4 = null_variant,                  \
                            CVarRef a5 = null_variant
#define INVOKE_FEW_ARGS_DECL10                                          \
                            INVOKE_FEW_ARGS_DECL6,                      \
                            CVarRef a6 = null_variant,                  \
                            CVarRef a7 = null_variant,                  \
                            CVarRef a8 = null_variant,                  \
                            CVarRef a9 = null_variant
#define INVOKE_FEW_ARGS_IMPL3                                           \
                            CVarRef a0, CVarRef a1, CVarRef a2
#define INVOKE_FEW_ARGS_IMPL6                                           \
                            INVOKE_FEW_ARGS_IMPL3, CVarRef a3, CVarRef a4, \
                            CVarRef a5
#define INVOKE_FEW_ARGS_IMPL10                                          \
                            INVOKE_FEW_ARGS_IMPL6, CVarRef a6, CVarRef a7, \
                            CVarRef a8, CVarRef a9

#define INVOKE_FEW_ARGS_PASS3 a0, a1, a2
#define INVOKE_FEW_ARGS_PASS6 INVOKE_FEW_ARGS_PASS3, a3, a4, a5
#define INVOKE_FEW_ARGS_PASS10 INVOKE_FEW_PARGS_PASS6, a6, a7, a8, a9

#if INVOKE_FEW_ARGS_COUNT == 3
#define INVOKE_FEW_ARGS_DECL_ARGS INVOKE_FEW_ARGS_DECL3
#define INVOKE_FEW_ARGS_PASS_ARGS INVOKE_FEW_ARGS_PASS3
#define INVOKE_FEW_ARGS_IMPL_ARGS INVOKE_FEW_ARGS_IMPL3
#elif INVOKE_FEW_ARGS_COUNT == 6
#define INVOKE_FEW_ARGS_DECL_ARGS INVOKE_FEW_ARGS_DECL6
#define INVOKE_FEW_ARGS_PASS_ARGS INVOKE_FEW_ARGS_PASS6
#define INVOKE_FEW_ARGS_IMPL_ARGS INVOKE_FEW_ARGS_IMPL6
#elif INVOKE_FEW_ARGS_COUNT == 10
#define INVOKE_FEW_ARGS_DECL_ARGS INVOKE_FEW_ARGS_DECL10
#define INVOKE_FEW_ARGS_PASS_ARGS INVOKE_FEW_ARGS_PASS10
#define INVOKE_FEW_ARGS_IMPL_ARGS INVOKE_FEW_ARGS_IMPL10
#else
#error Bad INVOKE_FEW_ARGS_COUNT
#endif

#define DECLARE_STATIC_PROP_OPS                                         \
  public:                                                               \
  static void os_static_initializer();                                  \
  static Variant os_getInit(CStrRef s);                                 \
  static Variant os_get(CStrRef s);                                     \
  static Variant &os_lval(CStrRef s);                                   \
  static Variant os_constant(const char *s);                            \

#define DECLARE_INSTANCE_PROP_OPS                                       \
  public:                                                               \
  virtual Variant *o_realProp(CStrRef prop, int flags,                  \
                        CStrRef context = null_string) const;           \
  Variant *o_realPropPrivate(CStrRef s, int flags) const;               \
  virtual void o_getArray(Array &props, bool pubOnly = false) const;    \
  virtual void o_setArray(CArrRef props);                               \

#define DECLARE_INSTANCE_PUBLIC_PROP_OPS                                \
  public:                                                               \
  virtual Variant *o_realPropPublic(CStrRef s, int flags) const;        \

#define DECLARE_COMMON_INVOKES                                          \
  static bool os_get_call_info(MethodCallPackage &mcp, int64 hash = -1); \
  virtual bool o_get_call_info(MethodCallPackage &mcp, int64 hash = -1);\

#define DECLARE_INVOKE_EX(cls, originalName, parent)                    \
  virtual bool o_get_call_info_ex(const char *clsname,               \
      MethodCallPackage &mcp, int64 h) {                                \
    if (clsname && strcasecmp(clsname, #originalName) == 0) {           \
      return c_##cls::o_get_call_info(mcp, h);                          \
    }                                                                   \
    return c_##parent::o_get_call_info_ex(clsname, mcp, h);             \
  }                                                                     \

#define DECLARE_INVOKE_EX_WITH_INDEX(cls, originalName, parent)         \
  virtual bool o_get_call_info_with_index_ex(const char *clsname,       \
      MethodCallPackage &mcp, MethodIndex mi, int64 h) {                \
    if (clsname && strcasecmp(clsname, #originalName) == 0) {           \
      return c_##cls::o_get_call_info_with_index(mcp, mi, h);           \
    }                                                                   \
    return c_##parent::o_get_call_info_with_index_ex(clsname, mcp, mi, h);\
  }                                                                     \

#define DECLARE_CLASS_COMMON(cls, originalName) \
  DECLARE_OBJECT_ALLOCATION(c_##cls)                                    \
  protected:                                                            \
  ObjectData *cloneImpl();                                              \
  void cloneSet(ObjectData *cl);                                        \
  public:                                                               \
  static const char *GetClassName() { return #originalName; }           \
  static StaticString s_class_name;                                     \
  virtual CStrRef o_getClassName() const { return s_class_name; }       \
  static c_##cls *createDummy(p_##cls &pobj);                           \

#define DECLARE_CLASS(cls, originalName, parent)                        \
  DECLARE_CLASS_COMMON(cls, originalName)                               \
  DECLARE_STATIC_PROP_OPS                                               \
  DECLARE_INSTANCE_PROP_OPS                                             \
  DECLARE_INSTANCE_PUBLIC_PROP_OPS                                      \
  DECLARE_COMMON_INVOKES                                                \
  DECLARE_INVOKE_EX(cls, originalName, parent)                          \
  public:                                                               \

#define DECLARE_DYNAMIC_CLASS(cls, originalName, parent)                \
  DECLARE_CLASS_COMMON(cls, originalName)                               \
  DECLARE_STATIC_PROP_OPS                                               \
  DECLARE_INSTANCE_PROP_OPS                                             \
  DECLARE_COMMON_INVOKES                                                \
  DECLARE_INVOKE_EX(cls, originalName, parent)                          \
  public:                                                               \

#define CLASS_CHECK(exp) (checkClassExists(s, g), (exp))

#define IMPLEMENT_CLASS_COMMON(cls)                                     \
  StaticString c_##cls::s_class_name(c_##cls::GetClassName());          \
  c_##cls *c_##cls::createDummy(p_##cls &pobj) {                        \
    pobj = NEWOBJ(c_##cls)();                                              \
    pobj->init();                                                       \
    pobj->setDummy();                                                   \
    return pobj.get();                                                  \
  }                                                                     \

#define IMPLEMENT_CLASS(cls)                                            \
  IMPLEMENT_CLASS_COMMON(cls)                                           \
  IMPLEMENT_OBJECT_ALLOCATION(c_##cls)                                  \

#define IMPLEMENT_CLASS_NO_DEFAULT_SWEEP(cls)                           \
  IMPLEMENT_CLASS_COMMON(cls)                                           \
  IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(c_##cls)                 \

#define DECLARE_METHOD_INVOKE_HELPERS(methname)                         \
  static CallInfo ci_##methname;                                        \
  static Variant ifa_##methname(MethodCallPackage &mcp,                 \
                                int count, INVOKE_FEW_ARGS_IMPL_ARGS);  \
  static Variant i_##methname(MethodCallPackage &mcp, CArrRef params);  \

#define DECLARE_METHOD_INVOKE_HELPERS_NOPARAM(methname)                 \
  static CallInfo ci_##methname;                                        \
  static Variant ifa_##methname(MethodCallPackage &mcp,                 \
                                int count, INVOKE_FEW_ARGS_IMPL_ARGS);  \
  static Variant i_##methname(MethodCallPackage &mcp, CArrRef params) { \
    return ((CallInfo::MethInvoker0Args)ifa_##methname)(mcp, 0);        \
  }                                                                     \

//////////////////////////////////////////////////////////////////////////////
// jump table entries

#define HASH_GUARD(code, f)                                             \
  if (hash == code && !strcasecmp(s, #f))
#define HASH_GUARD_LITSTR(code, str)                                    \
  if (hash == code && (str.data() == s || !strcasecmp(s, str.data())))
#define HASH_GUARD_STRING(code, f)                                      \
  if (hash == code && !strcasecmp(s.data(), #f))
#define HASH_EXISTS_STRING(code, str, len)                              \
  if (hash == code && s.length() == len &&                              \
      memcmp(s.data(), str, len) == 0) return true
#define HASH_REALPROP_STRING(code, str, len, prop)                      \
  if (hash == code && s.length() == len &&                              \
      memcmp(s.data(), str, len) == 0)                                  \
    return const_cast<Variant*>(&m_##prop)
#define HASH_REALPROP_TYPED_STRING(code, str, len, prop)                \
  if (!(flags&(RealPropCreate|RealPropWrite)) &&                        \
      hash == code && s.length() == len &&                              \
      memcmp(s.data(), str, len) == 0)                                  \
    return g->__realPropProxy = m_##prop,&g->__realPropProxy
#define HASH_INITIALIZED(code, name, str)                               \
  if (hash == code && strcmp(s, str) == 0)                              \
    return isInitialized(name)
#define HASH_INITIALIZED_STRING(code, name, str, len)                   \
  if (hash == code && s.length() == len &&                              \
      memcmp(s.data(), str, len) == 0) return isInitialized(name)
#define HASH_INITIALIZED_LITSTR(code, index, name, len)                 \
do { \
  const char *s1 = s.data();                                            \
  const char *s2 = literalStrings[index].data();                        \
  if ((s1 == s2) ||                                                     \
      (hash == code && s.length() == len &&                             \
      memcmp(s1, s2, len) == 0)) return isInitialized(name);            \
} while (0)
#define HASH_INITIALIZED_NAMSTR(code, str, name, len)                   \
do { \
  const char *s1 = s.data();                                            \
  const char *s2 = str.data();                                          \
  if ((s1 == s2) ||                                                     \
      (hash == code && s.length() == len &&                             \
      memcmp(s1, s2, len) == 0)) return isInitialized(name);            \
} while (0)
#define HASH_RETURN(code, name, str)                                    \
  if (hash == code && strcmp(s, str) == 0) return name
#define HASH_RETURN_STRING(code, name, str, len)                        \
  if (hash == code && s.length() == len &&                              \
      memcmp(s.data(), str, len) == 0) return name
#define HASH_RETURN_LITSTR(code, index, name, len)                      \
do { \
  const char *s1 = s.data();                                            \
  const char *s2 = literalStrings[index].data();                        \
  if ((s1 == s2) ||                                                     \
      (hash == code && s.length() == len &&                             \
      memcmp(s1, s2, len) == 0)) return name;                           \
} while (0)
#define HASH_RETURN_NAMSTR(code, str, name, len)                        \
do { \
  const char *s1 = s.data();                                            \
  const char *s2 = str.data();                                          \
  if ((s1 == s2) ||                                                     \
      (hash == code && s.length() == len &&                             \
      memcmp(s1, s2, len) == 0)) return name;                           \
} while (0)

#define HASH_SET_STRING(code, name, str, len)                           \
  if (hash == code && s.length() == len &&                              \
      memcmp(s.data(), str, len) == 0) { name = v; return null; }
#define HASH_INDEX(code, str, index)                                    \
  if (hash == code && strcmp(s, #str) == 0) { return index;}

#define HASH_INVOKE(code, f)                                            \
  if (hash == code && !strcasecmp(s, #f)) return i_ ## f(NULL, params)
#define HASH_INVOKE_REDECLARED(code, f)                                 \
  if (hash == code && !strcasecmp(s, #f)) return g->i_ ## f(NULL, params)
#define HASH_INVOKE_METHOD(code, f)                                     \
  if (hash == code && !strcasecmp(s, #f)) return o_i_ ## f(params)
#define HASH_INVOKE_CONSTRUCTOR(code, f, id)                            \
  if (hash == code && !strcasecmp(s, #f)) return o_i_ ## id(params)
#define HASH_GET_OBJECT_STATIC_CALLBACKS(code, f)                       \
  if (hash == code && !strcasecmp(s, #f)) return &cw_ ## f
#define HASH_GET_OBJECT_STATIC_CALLBACKS_VOLATILE(code, f)              \
  if (hash == code && !strcasecmp(s, #f))                               \
    return CLASS_CHECK(&cw_ ## f)
#define HASH_CALL_INFO_STATIC_METHOD(code, f)                           \
  if (hash == code && !strcasecmp(s->data(), #f))                       \
    return cw_ ## f.os_get_call_info(mcp, -1)
#define HASH_CALL_INFO_STATIC_METHOD_VOLATILE(code, f)                  \
  if (hash == code && !strcasecmp(s->data(), #f))                       \
    return CLASS_CHECK(cw_ ## f.os_get_call_info(mcp, -1))
#define HASH_CALL_INFO_STATIC_METHOD_REDECLARED(code, f)                \
  if (hash == code && !strcasecmp(s->data(), #f))                       \
    return CLASS_CHECK(g->cso_ ## f->os_get_call_info(mcp, -1))
#define HASH_GET_OBJECT_STATIC_CALLBACKS_REDECLARED(code, f)            \
  if (hash == code && !strcasecmp(s, #f))                               \
    return CLASS_CHECK(g->cwo_ ## f)
#define HASH_CALL_INFO_STATIC_METHOD_WITH_INDEX(code, f)                \
  if (hash == code && !strcasecmp(s->data(), #f))                       \
    return cw_ ## f.os_get_call_info_with_index(mcp, mi, -1)
#define HASH_CALL_INFO_STATIC_METHOD_WITH_INDEX_VOLATILE(code, f)       \
  if (hash == code && !strcasecmp(s->data(), #f))                       \
    return CLASS_CHECK(cw_ ## f.os_get_call_info_with_index(mcp, mi, -1))
#define HASH_CALL_INFO_STATIC_METHOD_WITH_INDEX_REDECLARED(code, f)     \
  if (hash == code && !strcasecmp(s->data(), #f))                       \
    return CLASS_CHECK(g->cso_ ## f->os_get_call_info_with_index(mcp, mi, -1))
#define HASH_GET_CLASS_VAR_INIT(code, f)                                \
  if (hash == code && !strcasecmp(s, #f))                               \
    return cw_ ## f.os_getInit(var)
#define HASH_GET_CLASS_VAR_INIT_VOLATILE(code, f)                       \
  if (hash == code && !strcasecmp(s, #f))                               \
    return CLASS_CHECK(cw_ ## f.os_getInit(var))
#define HASH_GET_CLASS_VAR_INIT_REDECLARED(code, f)                     \
  if (hash == code && !strcasecmp(s, #f))                               \
    return CLASS_CHECK(g->cso_ ## f->os_getInit(var))
#define HASH_CREATE_OBJECT(code, f)                                     \
  if (hash == code && !strcasecmp(s, #f)) return co_ ## f(params, init)
#define HASH_CREATE_OBJECT_VOLATILE(code, f)                            \
  if (hash == code && !strcasecmp(s, #f))                               \
    return CLASS_CHECK(co_ ## f(params, init))
#define HASH_CREATE_OBJECT_REDECLARED(code, f)                          \
  if (hash == code && !strcasecmp(s, #f))                               \
    return CLASS_CHECK(g->cso_ ## f->create(params, init, root))
#define HASH_CREATE_OBJECT_ONLY(code, f)                                     \
  if (hash == code && !strcasecmp(s, #f)) return coo_ ## f()
#define HASH_CREATE_OBJECT_ONLY_VOLATILE(code, f)                            \
  if (hash == code && !strcasecmp(s, #f))                               \
    return CLASS_CHECK(coo_ ## f())
#define HASH_CREATE_OBJECT_ONLY_REDECLARED(code, f)                     \
  if (hash == code && !strcasecmp(s, #f))                               \
    return CLASS_CHECK(g->cso_ ## f->createOnly(root))
#define HASH_INCLUDE(code, file, fun)                                   \
  if (hash == code && !strcmp(file, s.c_str())) {                       \
    return pm_ ## fun(once, variables);                                 \
  }
#define HASH_INSTANCEOF(code, str)                                      \
  if ((s.data() == str.data()) ||                                       \
      (hash == code &&                                                  \
       strcasecmp(s.data(), str.data()) == 0)) return true;             \

///////////////////////////////////////////////////////////////////////////////
// global variable macros

#define FVF_PREFIX "fvf_"

#define GV(s)   gvm_  ## s
#define GCI(s)  cim_  ## s
#define CDEC(s) cdec_ ## s
#define FVF(s)  fvf_  ## s

///////////////////////////////////////////////////////////////////////////////
// code instrumentation or injections

#define DECLARE_THREAD_INFO                      \
  ThreadInfo *info ATTRIBUTE_UNUSED = \
    ThreadInfo::s_threadInfo.getNoCheck();       \
  int lc ATTRIBUTE_UNUSED = 0;        \

#define DECLARE_THREAD_INFO_NOINIT               \
  int lc ATTRIBUTE_UNUSED = 0;        \

#ifdef INFINITE_LOOP_DETECTION
#define LOOP_COUNTER(n)
#define LOOP_COUNTER_CHECK(n)                                           \
  if ((++lc & 1023) == 0) {                                             \
    check_request_timeout_ex(fi, lc);                                   \
  }
#define LOOP_COUNTER_CHECK_INFO(n)                                      \
  if ((++lc & 1023) == 0) {                                             \
    check_request_timeout_info(info, lc);                               \
  }
#else
#define LOOP_COUNTER(n)
#define LOOP_COUNTER_CHECK(n)
#define LOOP_COUNTER_CHECK_INFO(n)
#endif

#ifdef EXECUTION_PROFILER
#define EXECUTION_PROFILER_INJECTION(n) \
  ExecutionProfiler ep(fi.getThreadInfo(), n);
#else
#define EXECUTION_PROFILER_INJECTION(n)
#endif

#ifdef ENABLE_FULL_SETLINE
#define LINE(n, e) (set_line(n), e)
#else
#define LINE(n, e) (fi.setLine(n), e)
#endif

// Get global variables from thread info.
#define DECLARE_GLOBAL_VARIABLES_INJECTION(g)       \
  GlobalVariables *g ATTRIBUTE_UNUSED =  \
    fi.getThreadInfo()->m_globals;
#define DECLARE_SYSTEM_GLOBALS_INJECTION(g)         \
  SystemGlobals *g ATTRIBUTE_UNUSED =    \
    (SystemGlobals *)fi.getThreadInfo()->m_globals;

#define CHECK_ONCE(n)                             \
  {                                               \
    bool &alreadyRun = g->run_ ## n;              \
    if (alreadyRun) { if (incOnce) return true; } \
    else alreadyRun = true;                       \
    if (!variables) variables = g;                \
  }

// Stack frame injection is also for correctness, and cannot be disabled.

#define FRAME_INJECTION_FUNCTION_MEM(n) \
  FIFunctionMem fi(#n);

#define FRAME_INJECTION_FUNCTION_NOMEM(n) \
  FIFunctionNoMem fi(#n);

#define FRAME_INJECTION_FUNCTION_FS(n, fs) \
  FIFunctionFS fi(#n, fs);

#define FRAME_INJECTION_STATIC_METHOD_MEM(n) \
  FIStaticMethodMem fi(#n);

#define FRAME_INJECTION_STATIC_METHOD_NOMEM(n) \
  FIStaticMethodNoMem fi(#n);

// For classes that might have redeclaring subclasse
#define FRAME_INJECTION_OBJECT_METHOD_MEM(n) \
  FIObjectMethodMem fi(#n, this->getRoot());

#define FRAME_INJECTION_OBJECT_METHOD_NOMEM(n) \
  FIObjectMethodNoMem fi(#n, this->getRoot());

// For classes that do not have redeclaring subclasses
#define FRAME_INJECTION_OBJECT_METHOD_ROOTLESS_MEM(n) \
  FIObjectMethodMem fi(#n, this);

#define FRAME_INJECTION_OBJECT_METHOD_ROOTLESS_NOMEM(n) \
  FIObjectMethodNoMem fi(#n, this);

#define FRAME_INJECTION_NO_PROFILE(n) \
  FIFunctionNP fi(#n);

// code injected into beginning of every function/method
#define FUNCTION_INJECTION(n)                                       \
  DECLARE_THREAD_INFO_NOINIT                                        \
  FRAME_INJECTION_FUNCTION_MEM(n)                                   \
  DECLARE_GLOBAL_VARIABLES_INJECTION(g)                             \
  EXECUTION_PROFILER_INJECTION(false)                               \

#define FUNCTION_INJECTION_NOMEM(n)                                 \
  DECLARE_THREAD_INFO_NOINIT                                        \
  FRAME_INJECTION_FUNCTION_NOMEM(n)                                 \
  DECLARE_GLOBAL_VARIABLES_INJECTION(g)                             \
  EXECUTION_PROFILER_INJECTION(false)                               \

#define FUNCTION_INJECTION_FS(n, fs)                                \
  DECLARE_THREAD_INFO_NOINIT                                        \
  FRAME_INJECTION_FUNCTION_FS(n, fs)                                \
  DECLARE_GLOBAL_VARIABLES_INJECTION(g)                             \
  EXECUTION_PROFILER_INJECTION(false)                               \

#define STATIC_METHOD_INJECTION(c, n)                               \
  DECLARE_THREAD_INFO_NOINIT                                        \
  FRAME_INJECTION_STATIC_METHOD_MEM(n)                              \
  DECLARE_GLOBAL_VARIABLES_INJECTION(g)                             \
  EXECUTION_PROFILER_INJECTION(false)                               \

#define STATIC_METHOD_INJECTION_NOMEM(c, n)                         \
  DECLARE_THREAD_INFO_NOINIT                                        \
  FRAME_INJECTION_STATIC_METHOD_NOMEM(n)                            \
  DECLARE_GLOBAL_VARIABLES_INJECTION(g)                             \
  EXECUTION_PROFILER_INJECTION(false)                               \

#define INSTANCE_METHOD_INJECTION(c, n)                             \
  DECLARE_THREAD_INFO_NOINIT                                        \
  FRAME_INJECTION_OBJECT_METHOD_MEM(n)                              \
  DECLARE_GLOBAL_VARIABLES_INJECTION(g)                             \
  EXECUTION_PROFILER_INJECTION(false)                               \

#define INSTANCE_METHOD_INJECTION_NOMEM(c, n)                       \
  DECLARE_THREAD_INFO_NOINIT                                        \
  FRAME_INJECTION_OBJECT_METHOD_NOMEM(n)                            \
  DECLARE_GLOBAL_VARIABLES_INJECTION(g)                             \
  EXECUTION_PROFILER_INJECTION(false)                               \

#define INSTANCE_METHOD_INJECTION_ROOTLESS(c, n)                    \
  DECLARE_THREAD_INFO_NOINIT                                        \
  FRAME_INJECTION_OBJECT_METHOD_ROOTLESS_MEM(n)                     \
  DECLARE_GLOBAL_VARIABLES_INJECTION(g)                             \
  EXECUTION_PROFILER_INJECTION(false)                               \

#define INSTANCE_METHOD_INJECTION_ROOTLESS_NOMEM(c, n)              \
  DECLARE_THREAD_INFO_NOINIT                                        \
  FRAME_INJECTION_OBJECT_METHOD_ROOTLESS_NOMEM(n)                   \
  DECLARE_GLOBAL_VARIABLES_INJECTION(g)                             \
  EXECUTION_PROFILER_INJECTION(false)                               \

#define PSEUDOMAIN_INJECTION(n, esc)                                \
  DECLARE_THREAD_INFO_NOINIT                                        \
  GlobalVariables *g = (GlobalVariables *)globals;                  \
  CHECK_ONCE(esc)                                                   \
  FRAME_INJECTION_FUNCTION_FS(n, FrameInjection::PseudoMain)        \
  EXECUTION_PROFILER_INJECTION(false)                               \

// code injected into every profiled builtin function/method
#define FUNCTION_INJECTION_BUILTIN(n)                                 \
  DECLARE_THREAD_INFO_NOINIT                                          \
  FRAME_INJECTION_FUNCTION_FS(n, FrameInjection::BuiltinFunction)     \
  DECLARE_SYSTEM_GLOBALS_INJECTION(g)                                 \
  EXECUTION_PROFILER_INJECTION(true);                                 \

// code injected into every unprofiled builtin function/method
#define FUNCTION_NOPROFILE_BUILTIN(n)                                 \
  DECLARE_THREAD_INFO_NOINIT                                          \
  FRAME_INJECTION_NO_PROFILE(n)                                       \
  DECLARE_SYSTEM_GLOBALS_INJECTION(g)                                 \

// for frame injection with a real class name
#define STATIC_METHOD_INJECTION_BUILTIN(c, n)                         \
  DECLARE_THREAD_INFO_NOINIT                                          \
  FRAME_INJECTION_STATIC_METHOD_MEM(n)                                \
  DECLARE_SYSTEM_GLOBALS_INJECTION(g)                                 \
  EXECUTION_PROFILER_INJECTION(true);                                 \

#define INSTANCE_METHOD_INJECTION_BUILTIN(c, n)                       \
  if (!o_id) throw_instance_method_fatal(#n);                         \
  DECLARE_THREAD_INFO_NOINIT                                          \
  FRAME_INJECTION_OBJECT_METHOD_MEM(n)                                \
  DECLARE_SYSTEM_GLOBALS_INJECTION(g)                                 \
  EXECUTION_PROFILER_INJECTION(true);                                 \

#define PSEUDOMAIN_INJECTION_BUILTIN(n, esc)                          \
  SystemGlobals *g = (SystemGlobals *)globals;                        \
  CHECK_ONCE(esc)                                                     \
  DECLARE_THREAD_INFO_NOINIT                                          \
  FRAME_INJECTION_FUNCTION_FS(n, FrameInjection::PseudoMain)          \
  EXECUTION_PROFILER_INJECTION(true);                                 \

#define INTERCEPT_INJECTION_ALWAYS(name, func, args, rr)                \
  static char intercepted = -1;                                         \
  if (intercepted) {                                                    \
    Variant r, h = get_intercept_handler(name, &intercepted);           \
    if (!h.isNull() && handle_intercept(h, func, args, r)) return rr;   \
  }                                                                     \

#ifdef ENABLE_INTERCEPT
#define INTERCEPT_INJECTION(func, args, rr)       \
  INTERCEPT_INJECTION_ALWAYS(func, func, args, rr)
#else
#define INTERCEPT_INJECTION(func, args, rr)
#endif

#ifdef ENABLE_LATE_STATIC_BINDING

#define STATIC_CLASS_NAME_CALL(s, exp)                                 \
  (FrameInjection::StaticClassNameHelper(fi.getThreadInfo(), s), exp)
#define STATIC_CLASS_INVOKE_CALL(s, exp)                               \
  (FrameInjection::StaticClassNameHelper(fi.getThreadInfo(), s), exp)

#define BIND_CLASS_DOT  bindClass(fi.getThreadInfo()).
#define BIND_CLASS_ARROW(T) bindClass<c_##T>(fi.getThreadInfo())->

#else

#define STATIC_CLASS_NAME_CALL(s, exp) exp
#define STATIC_CLASS_INVOKE_CALL(s, exp) exp
#define BIND_CLASS_DOT
#define BIND_CLASS_ARROW(T)

#endif

// for collecting function/method parameter type information at runtime
#define RTTI_INJECTION(v, id)                   \
  do {                                          \
    unsigned int *counter = getRTTICounter(id); \
    if (counter) {                              \
      counter[getDataTypeIndex(v.getType())]++; \
    }                                           \
  } while (0)

// causes a division by zero error at compile time if the assertion fails
// NOTE: use __LINE__, instead of __COUNTER__, for better compatibility
#define CT_CONCAT_HELPER(a, b) a##b
#define CT_CONCAT(a, b) CT_CONCAT_HELPER(a, b)
#define CT_ASSERT(cond) \
  enum { CT_CONCAT(compile_time_assert_, __LINE__) = 1/(!!(cond)) }

#define CT_ASSERT_DESCENDENT_OF_OBJECTDATA(T)   \
  do {                                          \
    if (false) {                                \
      ObjectData * dummy = NULL;                \
      if (static_cast<T*>(dummy)) {}            \
    }                                           \
  } while(0)                                    \

//////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_MACROS_H__
