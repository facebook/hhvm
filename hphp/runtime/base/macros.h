/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_MACROS_H_
#define incl_HPHP_MACROS_H_

#include "hphp/util/assertions.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// class macros

#define LITSTR_INIT(str)    (true ? (str) : ("" str "")), (sizeof(str)-1)

#define FORWARD_DECLARE_CLASS(cls)              \
  class c_##cls;                                \
  typedef SmartObject<c_##cls> p_##cls;         \

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

#define NEWOBJ(T) new (HPHP::MM().smartMallocSizeLogged(sizeof(T))) T

#define DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(T)                         \
  public:                                                               \
  ALWAYS_INLINE void operator delete(void* p) {                         \
    static_assert(std::is_base_of<ResourceData,T>::value, "");          \
    assert(sizeof(T) <= kMaxSmartSize);                                 \
    MM().smartFreeSizeLogged(p, sizeof(T));                             \
  }

#define DECLARE_RESOURCE_ALLOCATION(T)                                  \
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(T)                               \
  void sweep() FOLLY_OVERRIDE;

#define DECLARE_OBJECT_ALLOCATION(T)                                    \
  static void typeCheck() {                                             \
    static_assert(std::is_base_of<ObjectData,T>::value, "");            \
  }                                                                     \
  virtual void sweep() FOLLY_OVERRIDE;

#define IMPLEMENT_OBJECT_ALLOCATION(T)          \
  void HPHP::T::sweep() { this->~T(); }

/**
InstantStatic allows defining a static in-class variable that is
initialized during program startup, without actually needing to define
it anywhere. When defining the static, just specify its type (T), the
type that T's constructor will receive (TInit), and the name of the
function that will be called for construction (init). One copy of
static data is generated per T/init.
 */
template <class T, class TInit, TInit init()>
struct InstantStatic {
  static T value;
};

template <class T, class TInit, TInit init()>
T InstantStatic<T, TInit, init>::value { init() };

#define CLASSNAME_IS(str)                                               \
  static const char *GetClassName() { return str; }                     \
  static const StaticString& classnameof() {                            \
    return InstantStatic<const StaticString, const char*, GetClassName> \
      ::value;                                                          \
  }

#define DECLARE_CLASS_NO_SWEEP(originalName)                    \
  public:                                                       \
  CLASSNAME_IS(#originalName)                                   \
  friend ObjectData* new_##originalName##_Instance(Class*);     \
  friend void delete_##originalName(ObjectData*, const Class*); \
  static inline HPHP::Class*& classof() {                       \
    static HPHP::Class* result;                                 \
    return result;                                              \
  }

#define DECLARE_CLASS_NO_ALLOCATION(originalName)   \
  DECLARE_CLASS_NO_SWEEP(originalName)              \
  static void *ObjAllocatorInitSetup;               \

/**
 * By this declaration a class introduced with DECLARE_CLASS can only
 * be smart-allocated.
 */
#define DECLARE_CLASS(cls)                      \
  DECLARE_OBJECT_ALLOCATION(c_##cls)            \
  DECLARE_CLASS_NO_SWEEP(cls)

#define IMPLEMENT_CLASS_NO_SWEEP(cls)

#define IMPLEMENT_CLASS(cls)                    \
  IMPLEMENT_OBJECT_ALLOCATION(c_##cls)

///////////////////////////////////////////////////////////////////////////////
// code instrumentation or injections

#define DECLARE_THREAD_INFO                     \
  ThreadInfo *info ATTRIBUTE_UNUSED =           \
    ThreadInfo::s_threadInfo.getNoCheck();      \
  int lc ATTRIBUTE_UNUSED = 0;

//////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_MACROS_H_
