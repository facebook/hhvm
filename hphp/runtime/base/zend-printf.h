/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_ZEND_PRINTF_H_
#define incl_HPHP_ZEND_PRINTF_H_

#include "hphp/runtime/base/types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * How PHP formats a string. Called by ext/ext_string.cpp.
 */
char *string_printf(const char *format, int len, CArrRef args, int *outlen);

// XXX: vspprintf and spprintf have slightly different semantics and flags than
// C99 printf (because PHP) so we can't annotate them with ATTRIBUTE_PRINTF

int vspprintf(char **pbuf, size_t max_len, const char *format, ...);
int vspprintf_ap(char **pbuf, size_t max_len, const char *format, va_list ap);
int spprintf(char **pbuf, size_t max_len, const char *format, ...);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ZEND_PRINTF_H_
