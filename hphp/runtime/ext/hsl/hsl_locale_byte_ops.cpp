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

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/hsl/hsl_locale_byte_ops.h"
#include "hphp/util/bstring.h"
#include "hphp/zend/zend-string.h"

namespace HPHP {

HSLLocaleByteOps::HSLLocaleByteOps(
) : HSLLocaleLibcOps(*Locale::getCLocale()) {
}

HSLLocaleByteOps::~HSLLocaleByteOps() {
}

int64_t HSLLocaleByteOps::strcoll(const String& a, const String& b) const {
  assertx(!a.isNull() && !b.isNull());
  return string_strcmp(a.data(), a.size(), b.data(), b.size());
}

int64_t HSLLocaleByteOps::strcasecmp(const String& a, const String& b) const {
  assertx(!a.isNull() && !b.isNull());
  return bstrcasecmp(a.data(), a.size(), b.data(), b.size());
}

} // namespace HPHP
