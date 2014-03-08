/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_PREG_H_
#define incl_HPHP_EXT_PREG_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// pcre

Variant HHVM_FUNCTION(preg_filter, const Variant& pattern, const Variant& replacement,
                                   const Variant& subject, int limit = -1,
                                   VRefParam count = uninit_null());
Variant HHVM_FUNCTION(preg_grep, const String& pattern, const Array& input,
                                 int flags = 0);
Variant HHVM_FUNCTION(preg_match, const String& pattern, const String& subject,
                      VRefParam matches = uninit_null(), int flags = 0,
                      int offset = 0);
Variant HHVM_FUNCTION(preg_match_all, const String& pattern,
                        const String& subject,
                        VRefParam matches = uninit_null(),
                        int flags = 0, int offset = 0);
Variant HHVM_FUNCTION(preg_replace, const Variant& pattern, const Variant& replacement,
                                    const Variant& subject, int limit = -1,
                                    VRefParam count = uninit_null());
Variant HHVM_FUNCTION(preg_replace_callback, const Variant& pattern, const Variant& callback,
                                const Variant& subject, int limit = -1,
                                VRefParam count = uninit_null());
Variant HHVM_FUNCTION(preg_split, const String& pattern, const String& subject,
                                  int limit = -1, int flags = 0);
String HHVM_FUNCTION(preg_quote, const String& str,
                                 const Variant& = null_string);
int64_t HHVM_FUNCTION(preg_last_error);

///////////////////////////////////////////////////////////////////////////////
// deprecating these

String HHVM_FUNCTION(ereg_replace, const String& pattern,
                                const String& replacement, const String& str);
String HHVM_FUNCTION(eregi_replace, const String& pattern,
                                const String& replacement, const String& str);
Variant HHVM_FUNCTION(ereg, const String& pattern, const String& str,
                            VRefParam regs = uninit_null());
Variant HHVM_FUNCTION(eregi, const String& pattern, const String& str,
                             VRefParam regs = uninit_null());
Variant HHVM_FUNCTION(split, const String& pattern, const String& str,
                             int limit = -1);
Variant HHVM_FUNCTION(spliti, const String& pattern, const String& str,
                              int limit = -1);
String HHVM_FUNCTION(sql_regcase, const String& str);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_PREG_H_
