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

#include "util/assertions.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// class macros

#define LITSTR_INIT(str)    (true ? (str) : ("" str "")), (sizeof(str)-1)
#define LITSTR(index, str)  (literalStrings[index])
#define NAMSTR(nam, str)    (nam)
#define NAMVAR(nam, str)    (nam)

#define GET_THIS()         fi.getThis()
#define GET_THIS_TYPED(T)  p_##T(fi.getThis())
#define GET_THIS_VALID()   fi.getThisForArrow()
#define GET_THIS_ARROW()   fi.getThisForArrow()->

#define FORWARD_DECLARE_CLASS(cls)              \
  class c_##cls;                                \
  typedef SmartObject<c_##cls> p_##cls;         \

#define FORWARD_DECLARE_CLASS_BUILTIN(cls)      \
  FORWARD_DECLARE_CLASS(cls)                    \
  extern ObjectData *coo_##cls();               \
  extern const ObjectStaticCallbacks cw_##cls;

#define FORWARD_DECLARE_INTERFACE(cls)                  \
  class c_##cls;                                        \
  typedef SmartInterface<c_##cls> p_##cls               \

#define FORWARD_DECLARE_GENERIC_INTERFACE(cls)          \
  class c_##cls;                                        \
  typedef Object               p_##cls                  \

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

#define INVOKE_FEW_ARGS_PASS3  a0, a1, a2
#define INVOKE_FEW_ARGS_PASS6  INVOKE_FEW_ARGS_PASS3, a3, a4, a5
#define INVOKE_FEW_ARGS_PASS10 INVOKE_FEW_ARGS_PASS6, a6, a7, a8, a9

#define INVOKE_FEW_ARGS_PASS_ARR3 args[0], args[1], args[2]
#define INVOKE_FEW_ARGS_PASS_ARR6 INVOKE_FEW_ARGS_PASS_ARR3, \
    args[3], args[4], args[5]
#define INVOKE_FEW_ARGS_PASS_ARR10 INVOKE_FEW_ARGS_PASS_ARR6, \
    args[6], args[7], args[8], args[9]

#define INVOKE_FEW_ARGS_HELPER(kind,num) kind##num
#define INVOKE_FEW_ARGS(kind,num) \
  INVOKE_FEW_ARGS_HELPER(INVOKE_FEW_ARGS_##kind,num)


#define INVOKE_FEW_ARGS_DECL_ARGS INVOKE_FEW_ARGS(DECL,INVOKE_FEW_ARGS_COUNT)
#define INVOKE_FEW_ARGS_PASS_ARGS INVOKE_FEW_ARGS(PASS,INVOKE_FEW_ARGS_COUNT)
#define INVOKE_FEW_ARGS_PASS_ARR_ARGS \
  INVOKE_FEW_ARGS(PASS_ARR,INVOKE_FEW_ARGS_COUNT)
#define INVOKE_FEW_ARGS_IMPL_ARGS INVOKE_FEW_ARGS(IMPL,INVOKE_FEW_ARGS_COUNT)

#define DECLARE_CLASS_COMMON_NO_SWEEP(cls, originalName)                \
  DECLARE_OBJECT_ALLOCATION_NO_SWEEP(c_##cls)                           \
  public:                                                               \
  static const char *GetClassName() { return #originalName; }           \
  static StaticString s_class_name;                                     \
  static HPHP::VM::Class* s_cls;                                        \

#define DECLARE_CLASS_COMMON_NO_ALLOCATION(cls, originalName)           \
  public:                                                               \
  static void *ObjAllocatorInitSetup;                                   \
  static const char *GetClassName() { return #originalName; }           \
  static StaticString s_class_name;                                     \
  static HPHP::VM::Class* s_cls;                                        \

#define DECLARE_CLASS_COMMON(cls, originalName)                         \
  DECLARE_OBJECT_ALLOCATION(c_##cls)                                    \
  public:                                                               \
  static const char *GetClassName() { return #originalName; }           \
  static StaticString s_class_name;                                     \
  static HPHP::VM::Class* s_cls;                                        \

#define DECLARE_CLASS_NO_SWEEP(cls, originalName, parent)               \
  DECLARE_CLASS_COMMON_NO_SWEEP(cls, originalName)                      \
  public:                                                               \

#define DECLARE_CLASS_NO_ALLOCATION(cls, originalName, parent)          \
  DECLARE_CLASS_COMMON_NO_ALLOCATION(cls, originalName)                 \
  public:                                                               \

#define DECLARE_CLASS(cls, originalName, parent)                        \
  DECLARE_CLASS_COMMON(cls, originalName)                               \
  public:                                                               \

#define DECLARE_DYNAMIC_CLASS(cls, originalName, parent)                \
  DECLARE_CLASS_COMMON_NO_SWEEP(cls, originalName)                      \
  public:                                                               \

#define IMPLEMENT_CLASS_COMMON(cls)                                     \
  StaticString c_##cls::s_class_name(c_##cls::GetClassName());          \
  HPHP::VM::Class* c_##cls::s_cls = NULL;                               \

#define IMPLEMENT_CLASS(cls)                                            \
  IMPLEMENT_CLASS_COMMON(cls)                                           \
  IMPLEMENT_OBJECT_ALLOCATION(c_##cls)                                  \

#define IMPLEMENT_CLASS_NO_DEFAULT_SWEEP(cls)                           \
  IMPLEMENT_CLASS_COMMON(cls)                                           \
  IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(c_##cls)                 \

#define DECLARE_METHOD_INVOKE_HELPERS(methname)                         \
  static Variant ifa_##methname(MethodCallPackage &mcp,                 \
                                int count, INVOKE_FEW_ARGS_IMPL_ARGS);  \
  static Variant i_##methname(MethodCallPackage &mcp, CArrRef params);  \

#define DECLARE_METHOD_INVOKE_WRAPPER_HELPERS(methname)                 \
  static Variant iwfa_##methname(void *self,                            \
                                 int count, INVOKE_FEW_ARGS_IMPL_ARGS); \
  static Variant iw_##methname(void *self, CArrRef params);             \

//////////////////////////////////////////////////////////////////////////////
// jump table entries

#define HASH_GUARD(code, f)                                             \
  if (hash == code && !strcasecmp(s, #f))
#define HASH_GUARD_STRING(code, f)                                      \
  if (hash == code && !strcasecmp(s.data(), #f))
#define HASH_INITIALIZED(code, name, str)                               \
  if (hash == code && strcmp(s, str) == 0)                              \
    return isInitialized(name)
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
    check_request_timeout_ex(lc);                                       \
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
  ExecutionProfiler ep(ThreadInfo::s_threadInfo.getNoCheck(), n);
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
    ThreadInfo::s_threadInfo->m_globals;
#define DECLARE_SYSTEM_GLOBALS_INJECTION(g)         \
  SystemGlobals *g ATTRIBUTE_UNUSED =    \
    (SystemGlobals *)ThreadInfo::s_threadInfo->m_globals;

#define CHECK_ONCE(n)                             \
  {                                               \
    bool &alreadyRun = g->run_ ## n;              \
    if (alreadyRun) { if (incOnce) return true; } \
    else alreadyRun = true;                       \
  }

// Stack frame injection is also for correctness, and cannot be disabled.
#ifdef HHVM

#define FRAME_INJECTION_FUNCTION_MEM(n) \
  FrameInjectionVM fi;
#define FRAME_INJECTION_FUNCTION_NOMEM(n) \
  FrameInjectionVM fi;
#define FRAME_INJECTION_FUNCTION_FS(n, fs) \
  FrameInjectionVM fi;
#define FRAME_INJECTION_STATIC_METHOD_MEM(n) \
  FrameInjectionVM fi;
#define FRAME_INJECTION_STATIC_METHOD_NOMEM(n) \
  FrameInjectionVM fi;
#define FRAME_INJECTION_OBJECT_METHOD_MEM(n) \
  FrameInjectionVM fi;
#define FRAME_INJECTION_OBJECT_METHOD_NOMEM(n) \
  FrameInjectionVM fi;
#define FRAME_INJECTION_OBJECT_METHOD_BUILTIN(n) \
  FrameInjectionVM fi;
#define FRAME_INJECTION_OBJECT_METHOD_ROOTLESS_MEM(n) \
  FrameInjectionVM fi;
#define FRAME_INJECTION_OBJECT_METHOD_ROOTLESS_NOMEM(n) \
  FrameInjectionVM fi;
#define FRAME_INJECTION_NO_PROFILE(n) \
  FrameInjectionVM fi;

#else

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

#define FRAME_INJECTION_OBJECT_METHOD_BUILTIN(n) \
  FIObjectMethodMem fi(#n, this->getBuiltinRoot());

// For classes that do not have redeclaring subclasses
#define FRAME_INJECTION_OBJECT_METHOD_ROOTLESS_MEM(n) \
  FIObjectMethodMem fi(#n, this);

#define FRAME_INJECTION_OBJECT_METHOD_ROOTLESS_NOMEM(n) \
  FIObjectMethodNoMem fi(#n, this);

#define FRAME_INJECTION_NO_PROFILE(n) \
  FIFunctionNP fi(#n);

#endif // #ifdef HHVM

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
  FRAME_INJECTION_OBJECT_METHOD_BUILTIN(n)                            \
  DECLARE_SYSTEM_GLOBALS_INJECTION(g)                                 \
  EXECUTION_PROFILER_INJECTION(true);                                 \

#define PSEUDOMAIN_INJECTION_BUILTIN(n, esc)                          \
  SystemGlobals *g = (SystemGlobals *)globals;                        \
  CHECK_ONCE(esc)                                                     \
  DECLARE_THREAD_INFO_NOINIT                                          \
  FRAME_INJECTION_FUNCTION_FS(n, FrameInjection::PseudoMain)          \
  EXECUTION_PROFILER_INJECTION(true);                                 \

#define INTERCEPT_INJECTION_ALWAYS(name, func, args, rr)        \
  static char intercepted = -1;                                 \
  if (UNLIKELY(intercepted)) {                                  \
    Variant *h = get_intercept_handler(name, &intercepted);     \
    if (h) {                                                    \
      Variant r;                                                \
      if (handle_intercept(*h, func, args, r)) return rr;       \
    }                                                           \
  }

#ifdef ENABLE_INTERCEPT
#define INTERCEPT_INJECTION(func, args, rr)       \
  INTERCEPT_INJECTION_ALWAYS(func, func, args, rr)
#else
#define INTERCEPT_INJECTION(func, args, rr)
#endif

#define STATIC_CLASS_NAME_CALL(s, exp)                                 \
  (FrameInjection::StaticClassNameHelper(fi.getThreadInfo(), s), exp)
#define STATIC_CLASS_INVOKE_CALL(s, exp)                               \
  (FrameInjection::StaticClassNameHelper(fi.getThreadInfo(), s), exp)

#define BIND_CLASS_DOT  bindClass(fi.getThreadInfo()).
#define BIND_CLASS_ARROW(T) bindClass<c_##T>(fi.getThreadInfo())->

// for collecting function/method parameter type information at runtime
#define RTTI_INJECTION(v, id)                   \
  do {                                          \
    unsigned int *counter = getRTTICounter(id); \
    if (counter) {                              \
      counter[getDataTypeIndex(v.getType())]++; \
    }                                           \
  } while (0)

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
