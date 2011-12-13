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
#ifndef incl_VM_RUNTIME_H_
#define incl_VM_RUNTIME_H_

#include <runtime/vm/dyn_tracer.h>
#include <runtime/vm/func.h>
#include <runtime/vm/funcdict.h>
#include <runtime/base/tv_macros.h>

namespace HPHP {
namespace VM {

int64 new_iter(HPHP::VM::Iter* dest, HphpArray* arr);
int64 iter_next_array(HPHP::VM::Iter* dest);
void iter_value_cell(HPHP::VM::Iter* iter, TypedValue* out);
void iter_key_cell(HPHP::VM::Iter* iter, TypedValue* out);

StringData* concat_is(int64 v1, StringData* v2);
StringData* concat_si(StringData* v1, int64 v2);
StringData* concat_ss(StringData* v1, StringData* v2);
StringData* concat(DataType t1, uint64 v1, DataType t2, uint64 v2);

int64 tv_to_bool(TypedValue* tv);

int64 eq_null_str(StringData* v1);
int64 eq_bool_str(int64 v1, StringData* v2);
int64 eq_int_str(int64 v1, StringData* v2);
int64 eq_str_str(StringData* v1, StringData* v2);

int64 same_str_str(StringData* v1, StringData* v2);

int64 str0_to_bool(StringData* sd);
int64 str_to_bool(StringData* sd);
int64 arr0_to_bool(ArrayData* ad);
int64 arr_to_bool(ArrayData* ad);

void print_string(StringData* s);
void print_int(int64 i);
void print_boolean(int64 val);

void tv_release_str(StringData* datum);
void tv_release_arr(ArrayData* datum);
void tv_release_obj(ObjectData* datum);
void tv_release_var(Variant* datum);

void tv_release_generic(TypedValue* tv);

#define FP2LOC(f, l) ((uintptr_t)f - (uintptr_t)((l+1) * sizeof(TypedValue)))
#define FP2ITER(f, i)                                                       \
  (Iter*)(uintptr_t(f)                                                      \
          - uintptr_t((f)->m_func->m_numLocals * sizeof(TypedValue))        \
          - uintptr_t((i+1) * sizeof(Iter)))

inline TypedValue*
frame_local(const ActRec* fp, int n) {
  return (TypedValue*)FP2LOC(fp, n);
}

template <bool canThrow>
inline void ALWAYS_INLINE
frame_free_locals_inl_impl(ActRec* fp) {
  ASSERT(!fp->hasInvName());
  {
    if (fp->hasThis()) {
      ObjectData* this_ = fp->getThis();
      if (this_->decRefCount() == 0) {
        this_->releaseImpl<canThrow>();
      }
    }
  }
  if (UNLIKELY(fp->m_varEnv != NULL)) {
    // If there is a VarEnv, free the locals and args by
    // calling the detach method.
    fp->m_varEnv->detach(fp);
  } else {
    // Otherise, free locals and args here.
    int numLocals = fp->m_func->m_numLocals;
    for (int i = 0; i < numLocals; i++) {
      TRACE_MOD(Trace::runtime, 5,
                "RetC: freeing %d'th local of %d\n", i,
                fp->m_func->m_numLocals);
      TypedValue* loc = (TypedValue*)FP2LOC(fp, i);
      if (IS_REFCOUNTED_TYPE(loc->m_type)) {
        tvDecRefImpl<canThrow>(loc);
        // Clobber, in case the VM recurses before this frame is fully freed
        // due to e.g. a __destruct() call on an object.
        TV_WRITE_UNINIT(loc);
      }
    }
  }

  DynTracer::FunctionExit(fp);
}

#define frame_free_locals_inl(fp)    frame_free_locals_inl_impl<true>(fp)

void frame_free_locals(ActRec* fp);
Unit* compile_string(const char* s, size_t sz, const char* fname = NULL);

HphpArray* pack_args_into_array(ActRec* ar, int nargs);

// Returns null if the function isn't intercepted
FuncDict::InterceptData* intercept_data(ActRec* ar);
// Returns true if and only if you should keep going with the original function
bool run_intercept_handler(ActRec* ar, FuncDict::InterceptData* data);

} }
#endif
