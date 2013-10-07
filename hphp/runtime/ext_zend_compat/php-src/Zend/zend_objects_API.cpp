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

#include "zend.h"
#include "zend_globals.h"
#include "zend_variables.h"
// has to be before zend_API since that defines getThis()
#include "hphp/runtime/ext_zend_compat/hhvm/ZendRequestLocal.h"
#include "zend_API.h"
#include "zend_objects_API.h"
#include "hphp/runtime/ext_zend_compat/hhvm/ZendObjectData.h"

#define ZEND_DEBUG_OBJECTS 0

ZEND_API void zend_objects_store_add_ref(zval *object TSRMLS_DC) {
  Z_OBJVAL_P(object)->incRefCount();
}

ZEND_API void zend_objects_store_del_ref(zval *zobject TSRMLS_DC) {
  Z_OBJVAL_P(zobject)->decRefCount();
}

ZEND_API void *zend_object_store_get_object(const zval *zobject TSRMLS_DC) {
  const auto& zod = static_cast<HPHP::c_ZendObjectData*>(Z_OBJVAL_P(zobject));
  return zend_object_store_get_object_by_handle(zod->getHandle());
}

ZEND_REQUEST_LOCAL_LIST(void*, s_object_store);
ZEND_API void *zend_object_store_get_object_by_handle(zend_object_handle handle TSRMLS_DC) {
  auto& store = s_object_store.get()->get();
  return store[handle];
}

// TODO(#2898342) free the objects
ZEND_API zend_object_handle zend_objects_store_put(void *object, zend_objects_store_dtor_t dtor, zend_objects_free_object_storage_t free_storage, zend_objects_store_clone_t clone TSRMLS_DC) {
  auto& store = s_object_store.get()->get();
  zend_object_handle index = store.size();
  store.push_back(object);
  return index;
}

ZEND_API zend_object_handlers *zend_get_std_object_handlers(void) {
    return &std_object_handlers;
}

/* Called when the ctor was terminated by an exception */
ZEND_API void zend_object_store_ctor_failed(zval *zobject TSRMLS_DC) {
}
