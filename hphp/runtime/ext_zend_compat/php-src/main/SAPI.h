/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author:  Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef SAPI_H
#define SAPI_H

#if defined(__GNUC__) && __GNUC__ >= 4
# define SAPI_API __attribute__ ((visibility("default")))
#else
# define SAPI_API
#endif

typedef struct _sapi_module_struct sapi_module_struct;

BEGIN_EXTERN_C()
extern SAPI_API sapi_module_struct sapi_module;  /* true global */
END_EXTERN_C()

struct _sapi_module_struct {
  char *name;
  void (*treat_data)(int arg, char *str, zval *destArray TSRMLS_DC);
};

/*
 * This is the preferred and maintained API for
 * operating on HTTP headers.
 */

/*
 * Always specify a sapi_header_line this way:
 *
 *     sapi_header_line ctr = {0};
 */

typedef struct {
  char *line; /* If you allocated this, you need to free it yourself */
  uint line_len;
  long response_code; /* long due to zend_parse_parameters compatibility */
} sapi_header_line;

typedef enum {          /* Parameter:       */
  SAPI_HEADER_REPLACE,    /* sapi_header_line*  */
  SAPI_HEADER_ADD,      /* sapi_header_line*  */
  SAPI_HEADER_DELETE,     /* sapi_header_line*  */
  SAPI_HEADER_DELETE_ALL,   /* void         */
  SAPI_HEADER_SET_STATUS    /* int          */
} sapi_header_op_enum;

BEGIN_EXTERN_C()
SAPI_API int sapi_header_op(sapi_header_op_enum op, void *arg TSRMLS_DC);
END_EXTERN_C()

/* header_handler() constants */
#define SAPI_HEADER_ADD      (1<<0)

#define SAPI_HEADER_SENT_SUCCESSFULLY  1
#define SAPI_HEADER_DO_SEND        2
#define SAPI_HEADER_SEND_FAILED      3

#define SAPI_DEFAULT_MIMETYPE    "text/html"
#define SAPI_DEFAULT_CHARSET    ""
#define SAPI_PHP_VERSION_HEADER    "X-Powered-By: PHP/" PHP_VERSION

#define SAPI_POST_READER_FUNC(post_reader) void post_reader(TSRMLS_D)
#define SAPI_POST_HANDLER_FUNC(post_handler) void post_handler(char *content_type_dup, void *arg TSRMLS_DC)

#define SAPI_TREAT_DATA_FUNC(treat_data) void treat_data(int arg, char *str, zval* destArray TSRMLS_DC)
#define SAPI_INPUT_FILTER_FUNC(input_filter) unsigned int input_filter(int arg, char *var, char **val, unsigned int val_len, unsigned int *new_val_len TSRMLS_DC)

BEGIN_EXTERN_C()
SAPI_API SAPI_POST_READER_FUNC(sapi_read_standard_form_data);
SAPI_API SAPI_POST_READER_FUNC(php_default_post_reader);
SAPI_API SAPI_TREAT_DATA_FUNC(php_default_treat_data);
SAPI_API SAPI_INPUT_FILTER_FUNC(php_default_input_filter);
END_EXTERN_C()

#endif /* SAPI_H */
