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

#ifndef incl_HPHP_EXT_ZEND_COMPAT_H_
#define incl_HPHP_EXT_ZEND_COMPAT_H_

#ifdef ENABLE_ZEND_COMPAT

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend-exception-store.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend-execution-stack.h"
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
  int return_value_used,
  // TSRMLS_DC
  void *** tsrm_ls
);

void zBoxAndProxy(TypedValue* arg);
void zBoxAndProxy(const TypedValue* arg);
TypedValue* zend_wrap_func(ActRec* ar);

///////////////////////////////////////////////////////////////////////////////
}
#else

namespace HPHP {

struct TypedValue;
inline void zBoxAndProxy(TypedValue* arg) {}
inline void zBoxAndProxy(const TypedValue* arg) {}

}

#include "hphp/runtime/vm/native.h"
#define zend_wrap_func Native::unimplementedWrapper

#endif // ENABLE_ZEND_COMPAT

#endif // incl_HPHP_EXT_ZEND_COMPAT_H_
