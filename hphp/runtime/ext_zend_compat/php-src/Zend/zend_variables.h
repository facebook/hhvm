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

#ifndef ZEND_VARIABLES_H
#define ZEND_VARIABLES_H


BEGIN_EXTERN_C()

ZEND_API void _zval_dtor_func(zval *zvalue ZEND_FILE_LINE_DC);

static zend_always_inline void _zval_dtor(zval *zvalue ZEND_FILE_LINE_DC)
{
#ifdef HHVM
  if (!IS_REFCOUNTED_TYPE(zvalue->tv()->m_type)) {
    return;
  }
  bool willRelease = tvDecRefWillRelease(zvalue->tv());
  tvDecRefHelper(zvalue->tv()->m_type, zvalue->tv()->m_data.num);
  if (willRelease) {
    zvalue->tv()->m_type = HPHP::KindOfNull;
    zvalue->tv()->m_data.num = 0;
  }
#else
  if (zvalue->type <= IS_BOOL) {
    return;
  }
  _zval_dtor_func(zvalue ZEND_FILE_LINE_RELAY_CC);
#endif
}

ZEND_API void _zval_copy_ctor_func(zval *zvalue ZEND_FILE_LINE_DC);

static zend_always_inline void _zval_copy_ctor(zval *zvalue ZEND_FILE_LINE_DC)
{
#ifndef HHVM
  if (zvalue->type <= IS_BOOL) {
    return;
  }
#endif
  _zval_copy_ctor_func(zvalue ZEND_FILE_LINE_RELAY_CC);
}

ZEND_API int zval_copy_static_var(zval **p TSRMLS_DC, int num_args, va_list args, zend_hash_key *key);

ZEND_API int zend_print_variable(zval *var);
ZEND_API void _zval_ptr_dtor(zval **zval_ptr ZEND_FILE_LINE_DC);
ZEND_API void _zval_internal_dtor(zval *zvalue ZEND_FILE_LINE_DC);
ZEND_API void _zval_internal_ptr_dtor(zval **zvalue ZEND_FILE_LINE_DC);
ZEND_API void _zval_dtor_wrapper(zval *zvalue);
#define zval_copy_ctor(zvalue) _zval_copy_ctor((zvalue) ZEND_FILE_LINE_CC)
#define zval_dtor(zvalue) _zval_dtor((zvalue) ZEND_FILE_LINE_CC)
#define zval_ptr_dtor(zval_ptr) _zval_ptr_dtor((zval_ptr) ZEND_FILE_LINE_CC)
#define zval_internal_dtor(zvalue) _zval_internal_dtor((zvalue) ZEND_FILE_LINE_CC)
#define zval_internal_ptr_dtor(zvalue) _zval_internal_ptr_dtor((zvalue) ZEND_FILE_LINE_CC)
#define zval_dtor_wrapper _zval_dtor_wrapper

#if ZEND_DEBUG
ZEND_API void _zval_copy_ctor_wrapper(zval *zvalue);
ZEND_API void _zval_ptr_dtor_wrapper(zval **zval_ptr);
ZEND_API void _zval_internal_dtor_wrapper(zval *zvalue);
ZEND_API void _zval_internal_ptr_dtor_wrapper(zval **zvalue);
#define zval_copy_ctor_wrapper _zval_copy_ctor_wrapper
#define zval_ptr_dtor_wrapper _zval_ptr_dtor_wrapper
#define zval_internal_dtor_wrapper _zval_internal_dtor_wrapper
#define zval_internal_ptr_dtor_wrapper _zval_internal_ptr_dtor_wrapper
#else
#define zval_copy_ctor_wrapper _zval_copy_ctor_func
#define zval_ptr_dtor_wrapper _zval_ptr_dtor
#define zval_internal_dtor_wrapper _zval_internal_dtor
#define zval_internal_ptr_dtor_wrapper _zval_internal_ptr_dtor
#endif

ZEND_API void zval_add_ref(zval **p);

END_EXTERN_C()

#define ZVAL_DESTRUCTOR (void (*)(void *)) zval_dtor_wrapper
#define ZVAL_PTR_DTOR (void (*)(void *)) zval_ptr_dtor_wrapper
#define ZVAL_INTERNAL_DTOR (void (*)(void *)) zval_internal_dtor_wrapper
#define ZVAL_INTERNAL_PTR_DTOR (void (*)(void *)) zval_internal_ptr_dtor_wrapper
#define ZVAL_COPY_CTOR (void (*)(void *)) zval_copy_ctor_wrapper

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
