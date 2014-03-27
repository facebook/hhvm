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

#include "hphp/runtime/ext_zend_compat/hhvm/zend-wrap-func.h"
#include <algorithm>
#include "hphp/runtime/base/proxy-array.h"
#include "hphp/runtime/ext_zend_compat/php-src/TSRM/TSRM.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// TODO zPrepArgs needs to be updated so take care of
// boxing varargs
void zPrepArgs(ActRec* ar) {
  // If you call a function with too few params, zend_parse_parameters will
  // reject it, but we don't want this function boxing random slots from the
  // stack
  int32_t numArgs = std::min(ar->numArgs(), ar->m_func->numParams());
  TypedValue* args = (TypedValue*)ar - 1;
  for (int32_t i = 0; i < numArgs; ++i) {
    TypedValue* arg = args-i;
    zBoxAndProxy(arg);
  }
}

void zBoxAndProxy(TypedValue* arg) {
  if (arg->m_type != KindOfRef) {
    tvBox(arg);
  }
  auto inner = arg->m_data.pref->tv();
  if (inner->m_type == KindOfArray) {
    inner->m_data.parr = ProxyArray::Make(inner->m_data.parr);
  }
}

TypedValue* zend_wrap_func(
    ActRec* ar,
    zend_ext_func builtin_func,
    int numParams,
    bool isReturnRef) {
  TSRMLS_FETCH();

  // Prepare the arguments and return value before they are
  // exposed to the PHP extension
  zPrepArgs(ar);

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
  ZendExceptionStore& exceptionStore = ZendExceptionStore::getInstance();
  exceptionStore.clear();

  // Invoke the PHP extension function/method
  ZendExecutionStack::pushHHVMStack();
  try {
    builtin_func(
      ar->numArgs(),
      return_value->m_data.pref,
      return_value_ptr,
      this_ptr_var.isNull() ? nullptr : this_ptr->m_data.pref,
      1
	  TSRMLS_CC
    );
  } catch (...) {
    ZendExecutionStack::popHHVMStack();
    throw;
  }
  ZendExecutionStack::popHHVMStack();

  // If an exception was caught, rethrow it
  if (!exceptionStore.empty()) {
    exceptionStore.rethrow();
  }

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

