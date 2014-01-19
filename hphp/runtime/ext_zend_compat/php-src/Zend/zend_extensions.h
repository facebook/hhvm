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

#ifndef ZEND_EXTENSIONS_H
#define ZEND_EXTENSIONS_H

#include "zend_compile.h"
#include "zend_build.h"

/* The first number is the engine version and the rest is the date.
 * This way engine 2/3 API no. is always greater than engine 1 API no..
 */
#define ZEND_EXTENSION_API_NO  220121212

typedef struct _zend_extension_version_info {
  int zend_extension_api_no;
  char *build_id;
} zend_extension_version_info;

#define ZEND_EXTENSION_BUILD_ID "API" ZEND_TOSTR(ZEND_EXTENSION_API_NO) ZEND_BUILD_TS ZEND_BUILD_DEBUG ZEND_BUILD_SYSTEM ZEND_BUILD_EXTRA

typedef struct _zend_extension zend_extension;

/* Typedef's for zend_extension function pointers */
typedef int (*startup_func_t)(zend_extension *extension);
typedef void (*shutdown_func_t)(zend_extension *extension);
typedef void (*activate_func_t)(void);
typedef void (*deactivate_func_t)(void);

typedef void (*message_handler_func_t)(int message, void *arg);

typedef void (*op_array_handler_func_t)(zend_op_array *op_array);

typedef void (*statement_handler_func_t)(zend_op_array *op_array);
typedef void (*fcall_begin_handler_func_t)(zend_op_array *op_array);
typedef void (*fcall_end_handler_func_t)(zend_op_array *op_array);

typedef void (*op_array_ctor_func_t)(zend_op_array *op_array);
typedef void (*op_array_dtor_func_t)(zend_op_array *op_array);

struct _zend_extension {
  char *name;
  char *version;
  char *author;
  char *URL;
  char *copyright;

  startup_func_t startup;
  shutdown_func_t shutdown;
  activate_func_t activate;
  deactivate_func_t deactivate;

  message_handler_func_t message_handler;

  op_array_handler_func_t op_array_handler;

  statement_handler_func_t statement_handler;
  fcall_begin_handler_func_t fcall_begin_handler;
  fcall_end_handler_func_t fcall_end_handler;

  op_array_ctor_func_t op_array_ctor;
  op_array_dtor_func_t op_array_dtor;

  int (*api_no_check)(int api_no);
  int (*build_id_check)(const char* build_id);
  void *reserved3;
  void *reserved4;
  void *reserved5;
  void *reserved6;
  void *reserved7;
  void *reserved8;

  DL_HANDLE handle;
  int resource_number;
};

BEGIN_EXTERN_C()
ZEND_API int zend_get_resource_handle(zend_extension *extension);
ZEND_API void zend_extension_dispatch_message(int message, void *arg);
END_EXTERN_C()

#define ZEND_EXTMSG_NEW_EXTENSION    1


#define ZEND_EXTENSION()  \
  ZEND_EXT_API zend_extension_version_info extension_version_info = { ZEND_EXTENSION_API_NO, ZEND_EXTENSION_BUILD_ID }

#define STANDARD_ZEND_EXTENSION_PROPERTIES       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1
#define COMPAT_ZEND_EXTENSION_PROPERTIES         NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1
#define BUILD_COMPAT_ZEND_EXTENSION_PROPERTIES   NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1


ZEND_API extern zend_llist zend_extensions;

void zend_extension_dtor(zend_extension *extension);
void zend_append_version_info(const zend_extension *extension);
int zend_startup_extensions_mechanism(void);
int zend_startup_extensions(void);
void zend_shutdown_extensions(TSRMLS_D);

BEGIN_EXTERN_C()
ZEND_API int zend_load_extension(const char *path);
ZEND_API int zend_register_extension(zend_extension *new_extension, DL_HANDLE handle);
ZEND_API zend_extension *zend_get_extension(const char *extension_name);
END_EXTERN_C()

#endif /* ZEND_EXTENSIONS_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
