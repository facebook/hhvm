/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef __EXT_PHP_MCC_IMPL_H__
#define __EXT_PHP_MCC_IMPL_H__

#include "types.h"

namespace HPHP {

extern void phpmcc_log(MccResourcePtr &phpmcc, const mcc_errtype_t type,
                       const int code, const char* source, const int lineno,
                       const char* format, ...);
extern int mcc_log_if_error(MccResourcePtr &phpmcc);
extern void phpmcc_apevent_dispatcher(MccResourcePtr &phpmcc);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_PHP_MCC_IMPL_H__
