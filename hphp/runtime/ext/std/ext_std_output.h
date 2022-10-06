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

#include "hphp/runtime/ext/std/ext_std.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_PHP_OUTPUT_HANDLER_CONT;
extern const int64_t k_PHP_OUTPUT_HANDLER_WRITE;
extern const int64_t k_PHP_OUTPUT_HANDLER_START;
extern const int64_t k_PHP_OUTPUT_HANDLER_CLEAN;
extern const int64_t k_PHP_OUTPUT_HANDLER_FLUSH;
extern const int64_t k_PHP_OUTPUT_HANDLER_END;
extern const int64_t k_PHP_OUTPUT_HANDLER_FINAL;
extern const int64_t k_PHP_OUTPUT_HANDLER_CLEANABLE;
extern const int64_t k_PHP_OUTPUT_HANDLER_FLUSHABLE;
extern const int64_t k_PHP_OUTPUT_HANDLER_REMOVABLE;
extern const int64_t k_PHP_OUTPUT_HANDLER_STDFLAGS;

bool HHVM_FUNCTION(ob_start, const Variant& output_callback = uninit_null(),
                             int64_t chunk_size = 0,
                             int64_t flags = k_PHP_OUTPUT_HANDLER_STDFLAGS);
bool HHVM_FUNCTION(ob_end_clean);
Variant HHVM_FUNCTION(ob_get_contents);
int64_t HHVM_FUNCTION(ob_get_level);

///////////////////////////////////////////////////////////////////////////////
}
