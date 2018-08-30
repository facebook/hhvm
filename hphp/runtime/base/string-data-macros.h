/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_STRING_DATA_MACROS_H_
#define incl_HPHP_STRING_DATA_MACROS_H_

#include "hphp/util/low-ptr-def.h"

#ifdef USE_LOWPTR
#define NO_M_DATA 1
#endif

#ifdef NO_M_DATA
// Offsets of fields in StringData, when NO_M_DATA
#define SD_LEN    8
#define SD_DATA   16
#define SD_HASH   12
#endif

#endif
