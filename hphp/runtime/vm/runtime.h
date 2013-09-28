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
#ifndef incl_HPHP_VM_RUNTIME_H_
#define incl_HPHP_VM_RUNTIME_H_

#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/funcdict.h"
#include "hphp/runtime/base/builtin-functions.h"

namespace HPHP {

struct HhbcExtFuncInfo;
struct HhbcExtClassInfo;

ObjectData* newVectorHelper(int nElms);
ObjectData* newMapHelper(int nElms);
ObjectData* newStableMapHelper(int nElms);
ObjectData* newSetHelper(int nElms);
ObjectData* newPairHelper();

StringData* concat_is(int64_t v1, StringData* v2);
StringData* concat_si(StringData* v1, int64_t v2);
StringData* concat_ss(StringData* v1, StringData* v2);

int64_t eq_null_str(StringData* v1);
int64_t eq_bool_str(int64_t v1, StringData* v2);
int64_t eq_int_str(int64_t v1, StringData* v2);
int64_t eq_str_str(StringData* v1, StringData* v2);

int64_t same_str_str(StringData* v1, StringData* v2);

int64_t str0_to_bool(StringData* sd);
int64_t str_to_bool(StringData* sd);
int64_t arr0_to_bool(ArrayData* ad);
int64_t arr_to_bool(ArrayData* ad);

void print_string(StringData* s);
void print_int(int64_t i);
void print_boolean(bool val);

void raiseWarning(const StringData* sd);
int64_t modHelper(int64_t left, int64_t right);

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

inline c_Continuation*
frame_continuation(const ActRec* fp) {
  size_t arOffset = c_Continuation::getArOffset(fp->m_func);
  ObjectData* obj = (ObjectData*)((char*)fp - arOffset);
  assert(obj->getVMClass() == c_Continuation::classof());
  return static_cast<c_Continuation*>(obj);
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
  assert(!fp->hasInvName());
  // Check if the frame has a VarEnv or if it has extraArgs
  if (UNLIKELY(fp->m_varEnv != nullptr)) {
    if (fp->hasVarEnv()) {
      // If there is a VarEnv, free the locals and the VarEnv
      // by calling the detach method.
      fp->m_varEnv->detach(fp);
      return;
    }
    // Free extra args
    assert(fp->hasExtraArgs());
    ExtraArgs::deallocate(fp);
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
  if (fp->hasThis()) {
    ObjectData* this_ = fp->getThis();
    if (unwinding) {
      fp->setThis(nullptr);
    }
    decRefObj(this_);
  }
  frame_free_locals_helper_inl<unwinding>(fp, numLocals);
}

void ALWAYS_INLINE
frame_free_locals_inl(ActRec* fp, int numLocals) {
  frame_free_locals_inl_no_hook<false>(fp, numLocals);
  EventHook::FunctionExit(fp);
}

void ALWAYS_INLINE
frame_free_locals_unwind(ActRec* fp, int numLocals) {
  frame_free_locals_inl_no_hook<true>(fp, numLocals);
  EventHook::FunctionExit(fp);
}

void ALWAYS_INLINE
frame_free_locals_no_this_inl(ActRec* fp, int numLocals) {
  frame_free_locals_helper_inl<false>(fp, numLocals);
  EventHook::FunctionExit(fp);
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
Unit* build_native_func_unit(const HhbcExtFuncInfo* builtinFuncs,
                                 ssize_t numBuiltinFuncs);
Unit* build_native_class_unit(const HhbcExtClassInfo* builtinClasses,
                                  ssize_t numBuiltinClasses);

inline ObjectData*
newInstance(Class* cls) {
  assert(cls);
  auto* inst = ObjectData::newInstance(cls);
  if (UNLIKELY(RuntimeOption::EnableObjDestructCall)) {
    g_vmContext->m_liveBCObjs.insert(inst);
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

void collection_setm_wk1_v0(ObjectData* obj, TypedValue* value);
void collection_setm_ik1_v0(ObjectData* obj, int64_t key, TypedValue* value);
void collection_setm_sk1_v0(ObjectData* obj, StringData* key,
                            TypedValue* value);

// always_assert tv is a plausible TypedValue*
void assertTv(const TypedValue* tv);

// returns the number of things it put on sp
int init_closure(ActRec* ar, TypedValue* sp);

void defClsHelper(PreClass*);

}
#endif
