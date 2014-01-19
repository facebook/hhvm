/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_PREG_H_
#define incl_HPHP_PREG_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/complex-types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant preg_grep(const String& pattern, CArrRef input, int flags = 0);

Variant preg_match(const String& pattern, const String& subject,
                   Variant &matches,
                   int flags = 0, int offset = 0);

Variant preg_match(const String& pattern, const String& subject, int flags = 0,
                   int offset = 0);

Variant preg_match_all(const String& pattern, const String& subject,
                       Variant &matches,
                       int flags = 0, int offset = 0);

Variant preg_match_all(const String& pattern, const String& subject,
                       int flags = 0, int offset = 0);

Variant preg_replace_impl(CVarRef pattern, CVarRef replacement,
                          CVarRef subject, int limit, Variant &count,
                          bool is_callable);
int preg_replace(Variant &result, CVarRef pattern, CVarRef replacement,
                 CVarRef subject, int limit = -1);
int preg_replace_callback(Variant &result, CVarRef pattern, CVarRef callback,
                          CVarRef subject, int limit = -1);

Variant preg_split(CVarRef pattern, CVarRef subject, int limit = -1,
                   int flags = 0);
String preg_quote(const String& str, const String& delimiter = null_string);
Variant php_split(const String& spliton, const String& str, int count,
                  bool icase);

int preg_last_error();

size_t preg_pcre_cache_size();

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PREG_H__
