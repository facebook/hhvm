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

#ifndef __HPHP_ZEND_STRTOD_H__
#define __HPHP_ZEND_STRTOD_H__

#include <runtime/base/types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void zend_freedtoa(char *s);
char * zend_dtoa(double _d, int mode, int ndigits, int *decpt, int *sign,
                 char **rve);
double zend_strtod(const char *s00, char **se);
double zend_hex_strtod(const char *str, char **endptr);
double zend_oct_strtod(const char *str, char **endptr);
int zend_startup_strtod(void);
int zend_shutdown_strtod(void);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ZEND_STRTOD_H__
