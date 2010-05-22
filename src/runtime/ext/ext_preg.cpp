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

#include <runtime/base/preg.h>
#include <runtime/ext/ext_mb.h>
#include <runtime/ext/ext_string.h>
#include <runtime/base/util/request_local.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(pcre);
///////////////////////////////////////////////////////////////////////////////

Variant f_preg_grep(CStrRef pattern, CArrRef input, int flags /* = 0 */) {
  return preg_grep(pattern, input, flags);
}

///////////////////////////////////////////////////////////////////////////////

Variant f_preg_match(CStrRef pattern, CStrRef subject,
                     Variant matches /* = null */, int flags /* = 0 */,
                     int offset /* = 0 */) {
  return preg_match_impl(pattern, subject, matches, flags, offset, false);
}

Variant f_preg_match_all(CStrRef pattern, CStrRef subject, Variant matches,
                         int flags /* = 0 */, int offset /* = 0 */) {
  return preg_match_impl(pattern, subject, matches, flags, offset, true);
}

///////////////////////////////////////////////////////////////////////////////


Variant f_preg_replace(CVarRef pattern, CVarRef replacement, CVarRef subject,
                       int limit /* = -1 */, Variant count /* = null */) {
  return preg_replace_impl(pattern, replacement, subject, limit, count, false);
}

Variant f_preg_replace_callback(CVarRef pattern, CVarRef callback,
                                CVarRef subject, int limit /* = -1 */,
                                Variant count /* = null */) {
  return preg_replace_impl(pattern, callback, subject, limit, count, true);
}

///////////////////////////////////////////////////////////////////////////////

Variant f_preg_split(CVarRef pattern, CVarRef subject, int limit /* = -1 */,
                     int flags /* = 0 */) {
  return preg_split(pattern, subject, limit, flags);
}

///////////////////////////////////////////////////////////////////////////////

String f_preg_quote(CStrRef str, CStrRef delimiter /* = null_string */) {
  return preg_quote(str, delimiter);
}

int f_preg_last_error() {
  return preg_last_error();
}

///////////////////////////////////////////////////////////////////////////////
// ereg

String f_ereg_replace(CStrRef pattern, CStrRef replacement, CStrRef str) {
  return f_mb_ereg_replace(pattern, replacement, str);
}

String f_eregi_replace(CStrRef pattern, CStrRef replacement, CStrRef str) {
  return f_mb_eregi_replace(pattern, replacement, str);
}

Variant f_ereg(CStrRef pattern, CStrRef str, Variant regs /* = null */) {
  return f_mb_ereg(pattern, str, ref(regs));
}

Variant f_eregi(CStrRef pattern, CStrRef str, Variant regs /* = null */) {
  return f_mb_eregi(pattern, str, ref(regs));
}

///////////////////////////////////////////////////////////////////////////////
// regexec

Variant f_split(CStrRef pattern, CStrRef str, int limit /* = -1 */) {
  return php_split(pattern, str, limit, true);
}

Variant f_spliti(CStrRef pattern, CStrRef str, int limit /* = -1 */) {
  return php_split(pattern, str, limit, false);
}

String f_sql_regcase(CStrRef str) {
  unsigned char c;
  register int i, j;

  char *tmp = (char*)malloc(str.size() * 4 + 1);
  for (i = j = 0; i < str.size(); i++) {
    c = (unsigned char)str.charAt(i);
    if (isalpha(c)) {
      tmp[j++] = '[';
      tmp[j++] = toupper(c);
      tmp[j++] = tolower(c);
      tmp[j++] = ']';
    } else {
      tmp[j++] = c;
    }
  }
  tmp[j] = 0;

  return String(tmp, j, AttachString);
}

///////////////////////////////////////////////////////////////////////////////
}
