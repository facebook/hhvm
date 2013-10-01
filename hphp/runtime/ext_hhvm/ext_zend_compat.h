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
#include "hphp/runtime/ext_zend_compat/hhvm/ZendObjectData.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class TypedValue;

void zPrepArgs(ActRec* ar);

inline TypedValue* zend_wrap_func(
    ActRec* ar,
    void (*builtin_func)(ActRec*, RefData*, RefData*),
    int numParams,
    bool isReturnRef) {

  // Prepare the arguments and return value before they are
  // exposed to the PHP extension
  zPrepArgs(ar);
  Variant return_value(RefData::Make(*init_null_variant.asTypedValue()));
  Variant this_ptr(RefData::Make(*init_null_variant.asTypedValue()));
  if (ar->hasThis()) {
    tvWriteObject(
      ar->getThis(),
      this_ptr.asTypedValue()->m_data.pref->tv()
    );
  }

  // Invoke the PHP extension function/method
  builtin_func(
    ar,
    return_value.asTypedValue()->m_data.pref,
    this_ptr.asTypedValue()->m_data.pref
  );

  // Take care of freeing the args, tearing down the ActRec,
  // and moving the return value to the right place
  frame_free_locals_inl(ar, numParams);
  memcpy(&ar->m_r, return_value.asTypedValue(), sizeof(TypedValue));
  return_value.asTypedValue()->m_type = KindOfNull;
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
