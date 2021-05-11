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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// pcre

Variant HHVM_FUNCTION(preg_filter, const Variant& pattern,
                                   const Variant& replacement,
                                   const Variant& subject,
                                   int limit,
                                   int64_t& count);
Variant HHVM_FUNCTION(preg_grep, const String& pattern, const Variant& input,
                                 int flags = 0);
Variant HHVM_FUNCTION(preg_replace, const Variant& pattern, const Variant& replacement,
                                    const Variant& subject, int limit = -1);
Variant HHVM_FUNCTION(preg_replace_callback, const Variant& pattern,
                                             const Variant& callback,
                                             const Variant& subject,
                                             int limit,
                                             int64_t& count);
Variant HHVM_FUNCTION(
    preg_replace_callback_array, const Variant& patterns_and_callbacks,
                                 const Variant& subject, int limit,
                                 int64_t& count);
Variant HHVM_FUNCTION(preg_split, const String& pattern, const String& subject,
                                  const Variant& limit, int flags = 0);
String HHVM_FUNCTION(preg_quote, const String& str,
                                 const Variant& = null_string);

///////////////////////////////////////////////////////////////////////////////
// deprecating these

String HHVM_FUNCTION(ereg_replace, const String& pattern,
                                const String& replacement, const String& str);
String HHVM_FUNCTION(eregi_replace, const String& pattern,
                                const String& replacement, const String& str);
Variant HHVM_FUNCTION(split, const String& pattern, const String& str,
                             int limit = -1);
Variant HHVM_FUNCTION(spliti, const String& pattern, const String& str,
                              int limit = -1);
String HHVM_FUNCTION(sql_regcase, const String& str);

///////////////////////////////////////////////////////////////////////////////
}
