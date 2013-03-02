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
  HPHP::VM::Class* c_##cls::s_cls = nullptr;                            \

#define IMPLEMENT_CLASS(cls)                                            \
  IMPLEMENT_CLASS_COMMON(cls)                                           \
  IMPLEMENT_OBJECT_ALLOCATION(c_##cls)                                  \

#define IMPLEMENT_CLASS_NO_DEFAULT_SWEEP(cls)                           \
  IMPLEMENT_CLASS_COMMON(cls)                                           \
  IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(c_##cls)                 \

#define DECLARE_METHOD_INVOKE_HELPERS(methname)


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

#define CT_ASSERT_DESCENDENT_OF_OBJECTDATA(T)   \
  do {                                          \
    if (false) {                                \
      ObjectData * dummy = nullptr;                \
      if (static_cast<T*>(dummy)) {}            \
    }                                           \
  } while(0)                                    \

//////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_MACROS_H__
