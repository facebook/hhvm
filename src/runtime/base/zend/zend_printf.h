/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_ZEND_PRINTF_H__
#define __HPHP_ZEND_PRINTF_H__

#include <runtime/base/types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * How PHP formats a string. Called by ext/ext_string.cpp.
 */
char *string_printf(const char *format, int len, CArrRef args, int *outlen);

int vspprintf(char **pbuf, size_t max_len, const char *format, ...);
int vspprintf_ap(char **pbuf, size_t max_len, const char *format, va_list ap);
int spprintf(char **pbuf, size_t max_len, const char *format, ...);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ZEND_PRINTF_H__
