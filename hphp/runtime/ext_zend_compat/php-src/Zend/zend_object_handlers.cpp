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
#include "zend_API.h"
#include "zend_objects.h"
#include "zend_objects_API.h"
#include "zend_object_handlers.h"
#include "zend_interfaces.h"
#include "zend_compile.h"

zval *zend_std_read_property(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC) {
  return Z_OBJVAL_P(member)->o_get(tvCastToString(member->tv())).asRef()->m_data.pref;
}

ZEND_API void zend_std_write_property(zval *object, zval *member, zval *value, const zend_literal *key TSRMLS_DC) {
  Z_OBJVAL_P(member)->o_set(tvCastToString(member->tv()), tvAsVariant(value->tv()));
}

zval *zend_std_read_dimension(zval *object, zval *offset, int type TSRMLS_DC) {
  zend_class_entry *ce = Z_OBJCE_P(object);
  zend_error_noreturn(E_ERROR, "Cannot use object of type %s as array", ce->name);
}

static void zend_std_write_dimension(zval *object, zval *offset, zval *value TSRMLS_DC) {
  zend_class_entry *ce = Z_OBJCE_P(object);
  zend_error_noreturn(E_ERROR, "Cannot use object of type %s as array", ce->name);
}

static zval **zend_std_get_property_ptr_ptr(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC) {
  not_implemented();
  return nullptr;
}

static int zend_std_has_property(zval *object, zval *member, int has_set_exists, const zend_literal *key TSRMLS_DC) {
  not_implemented();
  return FAILURE;
}

static void zend_std_unset_property(zval *object, zval *member, const zend_literal *key TSRMLS_DC) {
  not_implemented();
}
static int zend_std_has_dimension(zval *object, zval *offset, int check_empty TSRMLS_DC) {
  not_implemented();
  return FAILURE;
}
static void zend_std_unset_dimension(zval *object, zval *offset TSRMLS_DC) {
  not_implemented();
}
ZEND_API HashTable *zend_std_get_properties(zval *object TSRMLS_DC) {
  not_implemented();
  return nullptr;
}
static union _zend_function *zend_std_get_method(zval **object_ptr, char *method_name, int method_len, const zend_literal *key TSRMLS_DC) {
  not_implemented();
  return nullptr;
}
ZEND_API union _zend_function *zend_std_get_constructor(zval *object TSRMLS_DC) {
  not_implemented();
  return nullptr;
}
zend_class_entry *zend_std_object_get_class(const zval *object TSRMLS_DC) {
  not_implemented();
  return nullptr;
}
int zend_std_object_get_class_name(const zval *object, const char **class_name, zend_uint *class_name_len, int parent TSRMLS_DC) {
  not_implemented();
  return FAILURE;
}
static int zend_std_compare_objects(zval *o1, zval *o2 TSRMLS_DC) {
  not_implemented();
  return FAILURE;
}
int zend_std_get_closure(zval *obj, zend_class_entry **ce_ptr, zend_function **fptr_ptr, zval **zobj_ptr TSRMLS_DC) {
  not_implemented();
  return FAILURE;
}
ZEND_API HashTable *zend_std_get_gc(zval *object, zval ***table, int *n TSRMLS_DC) {
  not_implemented();
  return nullptr;
}
ZEND_API int zend_std_cast_object_tostring(zval *readobj, zval *writeobj, int type TSRMLS_DC) {
  return FAILURE;
}

ZEND_API zend_object_handlers std_object_handlers = {
  zend_objects_store_add_ref,        /* add_ref */
  zend_objects_store_del_ref,        /* del_ref */
  zend_objects_clone_obj,          /* clone_obj */

  zend_std_read_property,          /* read_property */
  zend_std_write_property,        /* write_property */
  zend_std_read_dimension,        /* read_dimension */
  zend_std_write_dimension,        /* write_dimension */
  zend_std_get_property_ptr_ptr,      /* get_property_ptr_ptr */
  NULL,                  /* get */
  NULL,                  /* set */
  zend_std_has_property,          /* has_property */
  zend_std_unset_property,        /* unset_property */
  zend_std_has_dimension,          /* has_dimension */
  zend_std_unset_dimension,        /* unset_dimension */
  zend_std_get_properties,        /* get_properties */
  zend_std_get_method,          /* get_method */
  NULL,                  /* call_method */
  zend_std_get_constructor,        /* get_constructor */
  zend_std_object_get_class,        /* get_class_entry */
  zend_std_object_get_class_name,      /* get_class_name */
  zend_std_compare_objects,        /* compare_objects */
  zend_std_cast_object_tostring,      /* cast_object */
  NULL,                  /* count_elements */
  NULL,                  /* get_debug_info */
  zend_std_get_closure,          /* get_closure */
  zend_std_get_gc,            /* get_gc */
};
