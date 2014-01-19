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

#ifndef incl_HPHP_EXT_ZEND_COMPAT_H_
#define incl_HPHP_EXT_ZEND_COMPAT_H_

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/ext_zend_compat/hhvm/ZendExecutionStack.h"
#include "hphp/runtime/ext_zend_compat/hhvm/ZendObjectData.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zval-helpers.h"

// zend.h is way to big to include here

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class RefData;

void zPrepArgs(ActRec* ar);

typedef void (*zend_ext_func)(
  // The number of args passed to this function.
  // Usually accessed with ARG_COUNT() or ZEND_NUM_ARGS.
  int ht,
  // The return value. Written to with RETURN_* macros.
  RefData* return_value,
  // The same as the previous arg but a **. This is provided so that the callee
  // can set the caller's zval* to point to a different zval. This is needed if
  // a builtin function wants to return by reference.
  RefData** return_value_ptr,
  // The $this in the method. Accessed with getThis().
  RefData* this_ptr,
  // An optimization that extensions can use (but I haven't seen one use it
  // yet). If we can detect that the return value isn't used, we could pass in 0
  // here and the ext wouldn't set it.
  int return_value_used
);

void zBoxAndProxy(TypedValue* arg);
void zBoxAndProxy(const TypedValue* arg);

inline TypedValue* zend_wrap_func(
    ActRec* ar,
    zend_ext_func builtin_func,
    int numParams,
    bool isReturnRef) {

  // Prepare the arguments and return value before they are
  // exposed to the PHP extension
  zPrepArgs(ar);

  // Using Variant so exceptions will decref them
  Variant return_value_var(
    RefData::Make(*init_null_variant.asTypedValue()),
    Variant::noInc
  );
  TypedValue* return_value = return_value_var.asTypedValue();

  Variant this_ptr_var(
    RefData::Make(*init_null_variant.asTypedValue()),
    Variant::noInc
  );
  TypedValue* this_ptr = this_ptr_var.asTypedValue();

  if (ar->hasThis()) {
    tvWriteObject(
      ar->getThis(),
      this_ptr->m_data.pref->tv()
    );
  }

  auto *return_value_ptr = &return_value->m_data.pref;

  // Invoke the PHP extension function/method
  ZendExecutionStack::pushHHVMStack();
  builtin_func(
    ar->numArgs(),
    return_value->m_data.pref,
    return_value_ptr,
    this_ptr_var.isNull() ? nullptr : this_ptr->m_data.pref,
    1
  );
  ZendExecutionStack::popHHVMStack();

  // Take care of freeing the args, tearing down the ActRec,
  // and moving the return value to the right place
  frame_free_locals_inl(ar, numParams);
  memcpy(&ar->m_r, return_value, sizeof(TypedValue));
  return_value->m_type = KindOfNull;
  if (isReturnRef) {
    if (!ar->m_r.m_data.pref->isReferenced()) {
      tvUnbox(&ar->m_r);
    }
  } else {
    tvUnbox(&ar->m_r);
  }

  return &ar->m_r;
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_ZEND_COMPAT_H_
