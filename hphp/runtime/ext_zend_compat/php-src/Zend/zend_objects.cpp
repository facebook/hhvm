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
#include "zend_interfaces.h"
#include "zend_exceptions.h"

ZEND_API void zend_object_std_init(zend_object *object, zend_class_entry *ce TSRMLS_DC)
{
  object->ce = ce;
  object->properties = NULL;
  object->properties_table = NULL;
  object->guards = NULL;
}

ZEND_API void zend_object_std_dtor(zend_object *object TSRMLS_DC)
{
  not_implemented();
}

ZEND_API void zend_objects_destroy_object(zend_object *object, zend_object_handle handle TSRMLS_DC)
{
  not_implemented();
}

ZEND_API void zend_objects_free_object_storage(zend_object *object TSRMLS_DC)
{
  zend_object_std_dtor(object TSRMLS_CC);
  efree(object);
}

ZEND_API zend_object_value zend_objects_new(zend_object **object, zend_class_entry *class_type TSRMLS_DC)
{
  zend_object_value retval;

  *object = (zend_object*) emalloc(sizeof(zend_object));
  (*object)->ce = class_type;
  (*object)->properties = NULL;
  (*object)->properties_table = NULL;
  (*object)->guards = NULL;
  retval.handle = zend_objects_store_put(*object, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) zend_objects_free_object_storage, NULL TSRMLS_CC);
  retval.handlers = &std_object_handlers;
  return retval;
}

ZEND_API zend_object *zend_objects_get_address(const zval *zobject TSRMLS_DC)
{
  return (zend_object *)zend_object_store_get_object(zobject TSRMLS_CC);
}

ZEND_API void zend_objects_clone_members(zend_object *new_object, zend_object_value new_obj_val, zend_object *old_object, zend_object_handle handle TSRMLS_DC)
{
  not_implemented();
}

ZEND_API zend_object_value zend_objects_clone_obj(zval *zobject TSRMLS_DC)
{
  zend_object_value new_obj_val;
  zend_object *old_object;
  zend_object *new_object;
  zend_object_handle handle = Z_OBJ_HANDLE_P(zobject);

  /* assume that create isn't overwritten, so when clone depends on the
   * overwritten one then it must itself be overwritten */
  old_object = zend_objects_get_address(zobject TSRMLS_CC);
  new_obj_val = zend_objects_new(&new_object, old_object->ce TSRMLS_CC);

  zend_objects_clone_members(new_object, new_obj_val, old_object, handle TSRMLS_CC);

  return new_obj_val;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
