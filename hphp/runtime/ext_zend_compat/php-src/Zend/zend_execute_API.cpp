/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2013 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#include <stdio.h>
#include <signal.h>

#include "zend.h"
#include "zend_compile.h"
#include "zend_execute.h"
// builtin-functions has to happen before zend_API since that defines getThis()
#include "hphp/runtime/base/builtin-functions.h"
#include "zend_API.h"
#include "zend_constants.h"
#include "zend_extensions.h"
#include "zend_exceptions.h"
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
namespace HPHP {
  void zBoxAndProxy(TypedValue* arg);
}

/* true globals */
ZEND_API const zend_fcall_info empty_fcall_info = { 0, NULL, NULL, NULL, NULL, 0, NULL, NULL, 0 };
ZEND_API const zend_fcall_info_cache empty_fcall_info_cache = { 0, NULL, NULL, NULL, NULL };

ZEND_API int zend_lookup_class(const char *name, int name_length, zend_class_entry ***ce TSRMLS_DC) {
  HPHP::StringData *class_name = HPHP::makeStaticString(name, name_length);
  (**ce)->hphp_class = HPHP::Unit::loadClass(class_name);
  return (**ce)->hphp_class == nullptr ? FAILURE : SUCCESS;
}

ZEND_API const char *get_active_function_name(TSRMLS_D) {
  HPHP::JIT::VMRegAnchor _;
  return HPHP::liveFunc()->name()->data();
}

ZEND_API const char *get_active_class_name(const char **space TSRMLS_DC) {
  HPHP::Class *cls = HPHP::liveClass();
  if (!cls) {
    if (space) {
      *space = "";
    }
    return "";
  }
  if (space) {
    *space = "::";
  }
  return cls->name()->data();
}

ZEND_API const char *zend_get_executed_filename(TSRMLS_D) {
  return g_vmContext->getContainingFileName().data();
}

ZEND_API uint zend_get_executed_lineno(TSRMLS_D) {
  return g_vmContext->getLine();
}

ZEND_API int call_user_function(HashTable *function_table, zval **object_pp, zval *function_name, zval *retval_ptr, zend_uint param_count, zval *params[] TSRMLS_DC) {
  zval ***params_array;
  zend_uint i;
  int ex_retval;
  zval *local_retval_ptr = NULL;

  if (param_count) {
    params_array = (zval ***) emalloc(sizeof(zval **)*param_count);
    for (i=0; i<param_count; i++) {
      params_array[i] = &params[i];
    }
  } else {
    params_array = NULL;
  }
  ex_retval = call_user_function_ex(function_table, object_pp, function_name, &local_retval_ptr, param_count, params_array, 1, NULL TSRMLS_CC);
  if (local_retval_ptr) {
    COPY_PZVAL_TO_ZVAL(*retval_ptr, local_retval_ptr);
  } else {
    INIT_ZVAL(*retval_ptr);
  }
  if (params_array) {
    efree(params_array);
  }
  return ex_retval;
}

ZEND_API int call_user_function_ex(HashTable *function_table, zval **object_pp, zval *function_name, zval **retval_ptr_ptr, zend_uint param_count, zval **params[], int no_separation, HashTable *symbol_table TSRMLS_DC) {
  zend_fcall_info fci;

  fci.size = sizeof(fci);
  fci.function_table = function_table;
  fci.object_ptr = object_pp ? *object_pp : NULL;
  fci.function_name = function_name;
  fci.retval_ptr_ptr = retval_ptr_ptr;
  fci.param_count = param_count;
  fci.params = params;
  fci.no_separation = (zend_bool) no_separation;
  fci.symbol_table = symbol_table;

  return zend_call_function(&fci, NULL TSRMLS_CC);
}

int zend_call_function(zend_fcall_info *fci, zend_fcall_info_cache *fci_cache TSRMLS_DC) /* {{{ */
{
  assert(fci->object_ptr == nullptr);

  // mostly from vm_call_user_func
  HPHP::ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  HPHP::JIT::CallerFrame cf;
  HPHP::StringData* invName = nullptr;
  const HPHP::Func* f = HPHP::vm_decode_function(
    HPHP::tvAsCVarRef(fci->function_name->tv()), cf(), false, obj, cls, invName
  );
  if (f == nullptr) {
    return FAILURE;
  }

  HPHP::PackedArrayInit ad_params(fci->param_count);
  for (int i = 0; i < fci->param_count; i++) {
    HPHP::Variant v;
    v.asTypedValue()->m_type = HPHP::KindOfRef;
    v.asTypedValue()->m_data.pref = *fci->params[i];
    v.asTypedValue()->m_data.pref->incRefCount();
    if (f->byRef(i)) {
      ad_params.appendWithRef(v);
    } else {
      ad_params.append(v);
    }
  }
  HPHP::TypedValue retval;
  g_vmContext->invokeFunc(
    &retval, f, ad_params.toArray(), obj, cls,
    nullptr, invName, HPHP::ExecutionContext::InvokeCuf
  );
  if (retval.m_type == HPHP::KindOfUninit) {
    return FAILURE;
  }

  HPHP::zBoxAndProxy(&retval);
  *fci->retval_ptr_ptr = retval.m_data.pref;
  return SUCCESS;
}
