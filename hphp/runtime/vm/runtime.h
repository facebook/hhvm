/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_VM_RUNTIME_H_
#define incl_HPHP_VM_RUNTIME_H_

#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/stats.h"

namespace HPHP {

struct HhbcExtFuncInfo;
struct HhbcExtClassInfo;

StringData* concat_is(int64_t v1, StringData* v2);
StringData* concat_si(StringData* v1, int64_t v2);
StringData* concat_ss(StringData* v1, StringData* v2);
StringData* concat_s3(StringData* v1, StringData* v2, StringData* v3);
StringData* concat_s4(StringData* v1, StringData* v2,
                      StringData* v3, StringData* v4);

void print_string(StringData* s);
void print_int(int64_t i);
void print_boolean(bool val);

void raiseWarning(const StringData* sd);
void raiseNotice(const StringData* sd);
void raiseArrayIndexNotice(int64_t index);
void raiseArrayKeyNotice(const StringData* key);

inline Iter*
frame_iter(const ActRec* fp, int i) {
  return (Iter*)(uintptr_t(fp)
          - uintptr_t(fp->m_func->numLocals() * sizeof(TypedValue))
          - uintptr_t((i+1) * sizeof(Iter)));
}

inline TypedValue*
frame_local(const ActRec* fp, int n) {
  return (TypedValue*)(uintptr_t(fp) -
    uintptr_t((n+1) * sizeof(TypedValue)));
}

inline TypedValue*
frame_local_inner(const ActRec* fp, int n) {
  TypedValue* ret = frame_local(fp, n);
  return ret->m_type == KindOfRef ? ret->m_data.pref->tv() : ret;
}

inline Resumable*
frame_resumable(const ActRec* fp) {
  assert(fp->resumed());
  return (Resumable*)((char*)fp - Resumable::arOff());
}

inline c_AsyncFunctionWaitHandle*
frame_afwh(const ActRec* fp) {
  assert(fp->func()->isAsyncFunction());
  auto resumable = frame_resumable(fp);
  auto arOffset = c_AsyncFunctionWaitHandle::arOff();
  auto waitHandle = (c_AsyncFunctionWaitHandle*)((char*)resumable - arOffset);
  assert(waitHandle->getVMClass() == c_AsyncFunctionWaitHandle::classof());
  return waitHandle;
}

inline Generator*
frame_generator(const ActRec* fp) {
  assert(fp->func()->isNonAsyncGenerator());
  auto resumable = frame_resumable(fp);
  return (Generator*)((char*)resumable - Generator::resumableOff());
}

inline AsyncGenerator*
frame_async_generator(const ActRec* fp) {
  assert(fp->func()->isAsyncGenerator());
  auto resumable = frame_resumable(fp);
  return (AsyncGenerator*)((char*)resumable -
    AsyncGenerator::resumableOff());
}

/*
 * 'Unwinding' versions of the below frame_free_locals_* functions
 * zero locals and the $this pointer.
 *
 * This is necessary during unwinding because another object being
 * destructed by the unwind may decide to do a debug_backtrace and
 * read a destructed value.
 */

template<bool unwinding>
void ALWAYS_INLINE
frame_free_locals_helper_inl(ActRec* fp, int numLocals) {
  assert(numLocals == fp->m_func->numLocals());
  // Check if the frame has a VarEnv or if it has extraArgs
  if (UNLIKELY(fp->func()->attrs() & AttrMayUseVV) &&
      UNLIKELY(fp->m_varEnv != nullptr)) {
    if (fp->hasVarEnv()) {
      // If there is a VarEnv, free the locals and the VarEnv
      // by calling the detach method.
      fp->m_varEnv->exitFP(fp);
      return;
    }
    // Free extra args
    assert(fp->hasExtraArgs());
    ExtraArgs* ea = fp->getExtraArgs();
    int numExtra = fp->numArgs() - fp->m_func->numNonVariadicParams();
    if (unwinding) {
      fp->setNumArgs(fp->m_func->numParams());
      fp->setVarEnv(nullptr);
    }
    ExtraArgs::deallocate(ea, numExtra);
  }
  // Free locals
  for (int i = numLocals - 1; i >= 0; --i) {
    TRACE_MOD(Trace::runtime, 5,
              "RetC: freeing %d'th local of %d\n", i,
              fp->m_func->numLocals());
    TypedValue* loc = frame_local(fp, i);
    DataType t = loc->m_type;
    if (IS_REFCOUNTED_TYPE(t)) {
      uint64_t datum = loc->m_data.num;
      if (unwinding) {
        tvWriteUninit(loc);
      }
      tvDecRefHelper(t, datum);
    }
  }
}

template<bool unwinding>
void ALWAYS_INLINE
frame_free_locals_inl_no_hook(ActRec* fp, int numLocals) {
  frame_free_locals_helper_inl<unwinding>(fp, numLocals);
  if (fp->hasThis()) {
    ObjectData* this_ = fp->getThis();
    if (unwinding) {
      fp->setThis(nullptr);
    }
    decRefObj(this_);
  }
}

void ALWAYS_INLINE
frame_free_locals_inl(ActRec* fp, int numLocals, TypedValue* rv) {
  frame_free_locals_inl_no_hook<false>(fp, numLocals);
  EventHook::FunctionReturn(fp, *rv);
}

void ALWAYS_INLINE
frame_free_inl(ActRec* fp, TypedValue* rv) { // For frames with no locals
  assert(0 == fp->m_func->numLocals());
  assert(fp->m_varEnv == nullptr);
  assert(fp->hasThis());
  decRefObj(fp->getThis());
  EventHook::FunctionReturn(fp, *rv);
}

void ALWAYS_INLINE
frame_free_locals_unwind(ActRec* fp, int numLocals, ObjectData* phpException) {
  frame_free_locals_inl_no_hook<true>(fp, numLocals);
  EventHook::FunctionUnwind(fp, phpException);
}

void ALWAYS_INLINE
frame_free_locals_no_this_inl(ActRec* fp, int numLocals, TypedValue* rv) {
  frame_free_locals_helper_inl<false>(fp, numLocals);
  EventHook::FunctionReturn(fp, *rv);
}

// Helper for iopFCallBuiltin.
void ALWAYS_INLINE
frame_free_args(TypedValue* args, int count) {
  for (int i = 0; i < count; i++) {
    TypedValue* loc = args - i;
    DataType t = loc->m_type;
    if (IS_REFCOUNTED_TYPE(t)) {
      uint64_t datum = loc->m_data.num;
      // We don't have to write KindOfUninit here, because a
      // debug_backtrace wouldn't be able to see these slots (they are
      // stack cells).  But note we're also relying on the destructors
      // not throwing.
      tvDecRefHelper(t, datum);
    }
  }
}

Unit*
compile_file(const char* s, size_t sz, const MD5& md5, const char* fname);
Unit* compile_string(const char* s, size_t sz, const char* fname = nullptr);
Unit* compile_systemlib_string(const char* s, size_t sz, const char* fname);
Unit* build_native_func_unit(const HhbcExtFuncInfo* builtinFuncs,
                                 ssize_t numBuiltinFuncs);
Unit* build_native_class_unit(const HhbcExtClassInfo* builtinClasses,
                                  ssize_t numBuiltinClasses);

// Create a new class instance, and register it in the live object table if
// necessary. The initial ref-count of the instance will be greater than zero.
inline ObjectData*
newInstance(Class* cls) {
  assert(cls);
  auto* inst = ObjectData::newInstance(cls);
  assert(inst->getCount() > 0);
  Stats::inc(cls->getDtor() ? Stats::ObjectData_new_dtor_yes
                            : Stats::ObjectData_new_dtor_no);

  if (UNLIKELY(RuntimeOption::EnableObjDestructCall && cls->getDtor())) {
    g_context->m_liveBCObjs.insert(inst);
  }
  return inst;
}

// Returns a RefData* that is already incref'd.
RefData* lookupStaticFromClosure(ObjectData* closure,
                                 StringData* name,
                                 bool& inited);

/*
 * A few functions are exposed by libhphp_analysis and used in
 * VM-specific parts of the runtime.
 *
 * Currently we handle this by using these global pointers, which must
 * be set up before you use those parts of the runtime.
 */

typedef Unit* (*CompileStringFn)(const char*, int, const MD5&, const char*);
typedef Unit* (*BuildNativeFuncUnitFn)(const HhbcExtFuncInfo*, ssize_t);
typedef Unit* (*BuildNativeClassUnitFn)(const HhbcExtClassInfo*, ssize_t);

extern CompileStringFn g_hphp_compiler_parse;
extern BuildNativeFuncUnitFn g_hphp_build_native_func_unit;
extern BuildNativeClassUnitFn g_hphp_build_native_class_unit;

// always_assert tv is a plausible TypedValue*
void assertTv(const TypedValue* tv);

// returns the number of things it put on sp
int init_closure(ActRec* ar, TypedValue* sp);

int64_t zero_error_level();
void restore_error_level(int64_t oldLevel);

}
#endif
