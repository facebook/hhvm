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

#include "hphp/runtime/ext_zend_compat/hhvm/zend-wrap-func.h"
#include <algorithm>
#include "hphp/runtime/base/proxy-array.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend.h"
#include "hphp/runtime/ext_zend_compat/php-src/TSRM/TSRM.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_exceptions.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_globals.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void zBoxAndProxy(TypedValue* arg) {
  if (arg->m_type != KindOfRef) {
    tvBox(arg);
  }
  auto inner = arg->m_data.pref->tv();
  if (inner->m_type == KindOfArray && !inner->m_data.parr->isProxyArray()) {
    ArrayData * inner_arr = inner->m_data.parr;
    if (inner_arr->isStatic() || inner_arr->hasMultipleRefs()) {
      ArrayData * tmp = inner_arr->copy();
      if (inner_arr != tmp) {
        inner_arr->decRefAndRelease();
        inner_arr = tmp;
      }
    }
    inner->m_data.parr = ProxyArray::Make(inner_arr);
  }
}

static void zend_wrap_func_cleanup() {
    ZendExecutionStack::popHHVMStack();
    if (EG(exception)) {
      zval_ptr_dtor(&EG(exception));
      EG(exception) = NULL;
    }
}

TypedValue* zend_wrap_func(ActRec* ar) {
  // Sync the translator state.
  // We need to do this before a native function calls into a C library
  // compiled with -fomit-frame-pointer with the intention of having it call
  // back. Normal HHVM extensions have the luxury of only when such a thing
  // will be attempted, but we have no way to know in advance.
  VMRegAnchor _;

  TSRMLS_FETCH();
  zend_ext_func native_func = reinterpret_cast<zend_ext_func>(ar->func()->nativeFuncPtr());

  // Using Variant so exceptions will decref them
  Variant return_value_var(Variant::NullInit{});
  auto const return_value = return_value_var.asTypedValue();
  tvBox(return_value);

  Variant this_ptr_var(Variant::NullInit{});
  auto const this_ptr = this_ptr_var.asTypedValue();
  tvBox(this_ptr);

  if (ar->hasThis()) {
    tvWriteObject(
      ar->getThis(),
      this_ptr->m_data.pref->tv()
    );
  }

  auto *return_value_ptr = &return_value->m_data.pref;

  // Clear any stored exception
  zend_clear_exception(TSRMLS_C);

  // Invoke the PHP extension function/method
  ZendExecutionStack::pushHHVMStack((void*)ar);
  try {
    native_func(
      ar->numArgs(),
      return_value->m_data.pref,
      return_value_ptr,
      this_ptr_var.isNull() ? nullptr : this_ptr->m_data.pref,
      1
      TSRMLS_CC
    );
  } catch (...) {
    zend_wrap_func_cleanup();
    throw;
  }
  zend_wrap_func_cleanup();

  // If an exception was caught, rethrow it
  ZendExceptionStore& exceptionStore = ZendExceptionStore::getInstance();
  if (!exceptionStore.empty()) {
    exceptionStore.rethrow();
  }

  // Take care of freeing the args, tearing down the ActRec, and moving the
  // return value to the right place.  Note that frame_free_locals expects to
  // be able to free return_value in the event of an exception, so we have to
  // take it out of our Variant /before/ calling that.
  TypedValue return_value_copy = *return_value;
  return_value->m_type = KindOfNull; // clear the Variant's copy
  frame_free_locals_inl(ar, ar->func()->numLocals(), &return_value_copy);
  memcpy(&ar->m_r, &return_value_copy, sizeof(TypedValue));
  if (ar->func()->isReturnRef()) {
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
