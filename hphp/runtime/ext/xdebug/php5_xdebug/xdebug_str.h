/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <derick@xdebug.org>                         |
   +----------------------------------------------------------------------+
 */
// TODO(#4489053) This is almost a direct copy-paste, but it could be abstracted
//                and cleaned up relatively easily

#ifndef incl_XDEBUG_STR_H_
#define incl_XDEBUG_STR_H_

#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_mm.h"

#define XDEBUG_STR_PREALLOC 1024
#define xdebug_str_ptr_init(str) \
  str = (xdebug_str*) xdmalloc(sizeof(xdebug_str)); \
  str->l = 0; str->a = 0; str->d = nullptr;
#define xdebug_str_ptr_dtor(str) xdfree(str->d); xdfree(str)
#define xdebug_str_dtor(str)     xdfree(str.d)

typedef struct xdebug_str {
  int   l;
  int   a;
  char *d;
} xdebug_str;

void xdebug_str_add(xdebug_str *xs, char *str, int f);
void xdebug_str_addl(xdebug_str *xs, char *str, int le, int f);
void xdebug_str_chop(xdebug_str *xs, int c);
void xdebug_str_free(xdebug_str *s);

char* xdebug_sprintf(const char* fmt, ...);
char* xdstrdup(const char*);

#endif
