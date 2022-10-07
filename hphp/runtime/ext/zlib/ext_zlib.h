/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/std/ext_std_file.h"

namespace HPHP {

extern const int64_t k_FORCE_GZIP;
extern const int64_t k_FORCE_DEFLATE;

///////////////////////////////////////////////////////////////////////////////
// zlib functions

Variant HHVM_FUNCTION(gzcompress, const String& data,
                                  int64_t level = -1);
Variant HHVM_FUNCTION(gzuncompress, const String& data,
                                    int64_t limit = 0);
Variant HHVM_FUNCTION(gzinflate, const String& data, int64_t limit = 0);
Variant HHVM_FUNCTION(gzencode, const String& data, int64_t level = -1,
                                int64_t encoding_mode = k_FORCE_GZIP);

///////////////////////////////////////////////////////////////////////////////

}
