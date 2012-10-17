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

#include <runtime/vm/event_hook.h>
#include <runtime/vm/func.h>
#include <runtime/vm/funcdict.h>
#include <runtime/base/tv_macros.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/vm/translator/translator-inline.h>

namespace HPHP {
namespace VM {

int64 new_iter_array(HPHP::VM::Iter* dest, HphpArray* arr);
int64 new_iter_object(HPHP::VM::Iter* dest, ObjectData* obj, Class* ctx);
int64 iter_next_array(HPHP::VM::Iter* dest);
void iter_value_cell_array(HPHP::VM::Iter* iter, TypedValue* out);
void iter_value_cell_iterator(HPHP::VM::Iter* iter, TypedValue* out);
void iter_value_cell_local_array(HPHP::VM::Iter* iter, TypedValue* out);
void iter_value_cell_local_iterator(HPHP::VM::Iter* iter, TypedValue* out);
void iter_key_cell_array(HPHP::VM::Iter* iter, TypedValue* out);
void iter_key_cell_iterator(HPHP::VM::Iter* iter, TypedValue* out);
void iter_key_cell_local_array(HPHP::VM::Iter* iter, TypedValue* out);
void iter_key_cell_local_iterator(HPHP::VM::Iter* iter, TypedValue* out);

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
void tv_release_ref(RefData* datum);

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

inline void ALWAYS_INLINE
frame_free_locals_helper_inl(ActRec* fp, int numLocals) {
  ASSERT(numLocals == fp->m_func->numLocals());
  ASSERT(!fp->hasInvName());
  // Check if the frame has a VarEnv or if it has extraArgs
  if (UNLIKELY(fp->m_varEnv != NULL)) {
    if (fp->hasVarEnv()) {
      // If there is a VarEnv, free the locals and the VarEnv
      // by calling the detach method.
      fp->m_varEnv->detach(fp);
      return;
    }
    // Free extra args
    ASSERT(fp->hasExtraArgs());
    ExtraArgs::deallocate(fp);
  }
  // Free locals
  for (int i = 0; i < numLocals; i++) {
    TRACE_MOD(Trace::runtime, 5,
              "RetC: freeing %d'th local of %d\n", i,
              fp->m_func->numLocals());
    TypedValue* loc = frame_local(fp, i);
    DataType t = loc->m_type;
    if (IS_REFCOUNTED_TYPE(t)) {
      uint64_t datum = loc->m_data.num;
      // When destroying an array or object we can reenter the VM
      // to call a __destruct method. Null out the local before
      // calling the destructor so that stacktrace logic doesn't
      // choke.
      TV_WRITE_UNINIT(loc);
      tvDecRefHelper(t, datum);
    }
  }
}

inline void ALWAYS_INLINE
frame_free_locals_inl(ActRec* fp, int numLocals) {
  frame_free_locals_helper_inl(fp, numLocals);
  // Destroying the locals may re-enter the VM and take a backtrace which
  // accesses the 'this' field from this frame, so we need to destroy 'this'
  // after destroying the locals.
  if (fp->hasThis()) {
    ObjectData* this_ = fp->getThis();
    fp->setThis(NULL);
    if (this_->decRefCount() == 0) {
      this_->release();
    }
  }
  EventHook::FunctionExit(fp);
}

inline void ALWAYS_INLINE
frame_free_locals_no_this_inl(ActRec* fp, int numLocals) {
  frame_free_locals_helper_inl(fp, numLocals);
  EventHook::FunctionExit(fp);
}

void frame_free_locals(ActRec* fp, int numLocals);
void frame_free_locals_no_this(ActRec* fp, int numLocals);
Unit* compile_file(const char* s, size_t sz, const MD5& md5, const char* fname);
Unit* compile_string(const char* s, size_t sz);
Unit* build_native_func_unit(const HhbcExtFuncInfo* builtinFuncs,
                             ssize_t numBuiltinFuncs);
Unit* build_native_class_unit(const HhbcExtClassInfo* builtinClasses,
                              ssize_t numBuiltinClasses);

HphpArray* pack_args_into_array(ActRec* ar, int nargs);

template <bool handle_throw>
void call_intercept_handler(TypedValue* retval,
                            CArrRef intArgs,
                            ActRec* ar,
                            Variant* ihandler) {
  if (handle_throw) { ASSERT(ar); }
  ObjectData* intThis = NULL;
  Class* intCls = NULL;
  StringData* intInvName = NULL;
  const Func* handler = vm_decode_function(ihandler->asCArrRef()[0],
                                           g_vmContext->getFP(),
                                           false, intThis, intCls, intInvName);
  if (handle_throw) {
    // It's possible for the intercept handler could throw an exception.
    // If run_intercept_handler() was called from the translator and the
    // handler throws, we need to do some cleanup here before allowing
    // the exception to propagate.
    try {
      g_vmContext->invokeFunc(retval, handler, intArgs,
                              intThis, intCls, NULL, intInvName);
    } catch (...) {
      Stack& stack = g_vmContext->getStack();
      ASSERT((TypedValue*)ar - stack.top() == ar->numArgs());
      while (uintptr_t(stack.top()) < uintptr_t(ar)) {
        stack.popTV();
      }
      stack.popAR();
      throw;
    }
  } else {
    g_vmContext->invokeFunc(retval, handler, intArgs,
                            intThis, intCls, NULL, intInvName);
  }
}

/**
 * run_intercept_handler is used for functions invoked via FCall, whereas
 * run_intercept_handler_for_invokefunc is used for functions invoked by
 * the runtime via invokeFunc(). The run_intercept_handler* functions will
 * return true if and only if the original function should be executed.
 *
 * run_intercept_handler is called when the ActRec for the original function
 * is in the "pre-live" state. run_intercept_handler_for_invokefunc is called
 * before the ActRec for the original function has been materialized.
 */

template <bool handle_throw>
bool run_intercept_handler(ActRec* ar, Variant* ihandler) {
  using namespace HPHP::VM::Transl;
  ASSERT(ihandler);
  TypedValue retval;
  tvWriteNull(&retval);
  Variant doneFlag = true;
  Array intArgs =
    CREATE_VECTOR5(ar->m_func->fullNameRef(),
                   (ar->hasThis() ? Variant(Object(ar->getThis())) : null),
                   Array(pack_args_into_array(ar, ar->numArgs())),
                   ihandler->asCArrRef()[1],
                   ref(doneFlag));
  call_intercept_handler<handle_throw>(&retval, intArgs, ar, ihandler);
  if (doneFlag.toBoolean()) {
    // $done is true, meaning don't enter the intercepted function. Clean up
    // the pre-live ActRec and the args, move the intercept handler's return
    // value to the right place, and get out.
    Stack& stack = g_vmContext->getStack();
    ASSERT((TypedValue*)ar - stack.top() == ar->numArgs());
    while (uintptr_t(stack.top()) < uintptr_t(ar)) {
      stack.popTV();
    }
    stack.popAR();
    stack.allocTV();
    memcpy(stack.top(), &retval, sizeof(TypedValue));
    return false;
  }
  // Discard the handler's return value
  tvRefcountedDecRef(&retval);
  return true;
}

bool run_intercept_handler_for_invokefunc(TypedValue* retval,
                                          const Func* f,
                                          CArrRef params,
                                          ObjectData* this_,
                                          StringData* invName,
                                          Variant* ihandler);

static inline Instance*
newInstance(Class* cls) {
  ASSERT(cls);
  Instance *inst = Instance::newInstance(cls);
  if (UNLIKELY(RuntimeOption::EnableObjDestructCall)) {
    g_vmContext->m_liveBCObjs.insert(inst);
  }
  return inst;
}

HphpArray* get_static_locals(const ActRec* ar);

} }
#endif
