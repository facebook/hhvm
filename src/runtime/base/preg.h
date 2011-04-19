/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __PREG_H__
#define __PREG_H_

#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant preg_grep(CStrRef pattern, CArrRef input, int flags = 0);

Variant preg_match(CStrRef pattern, CStrRef subject, Variant &matches,
                   int flags = 0, int offset = 0);

Variant preg_match(CStrRef pattern, CStrRef subject, int flags = 0,
                   int offset = 0);

Variant preg_match_all(CStrRef pattern, CStrRef subject, Variant &matches,
                       int flags = 0, int offset = 0);

Variant preg_match_all(CStrRef pattern, CStrRef subject,
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
String preg_quote(CStrRef str, CStrRef delimiter = null_string);
Variant php_split(CStrRef spliton, CStrRef str, int count, bool icase);

int preg_last_error();

void preg_get_pcre_cache() ATTRIBUTE_COLD;
///////////////////////////////////////////////////////////////////////////////
}

#endif // __PREG_H__
