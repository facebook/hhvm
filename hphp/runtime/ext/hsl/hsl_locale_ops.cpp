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
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/ext/hsl/hsl_locale_ops.h"
#include "hphp/system/systemlib.h"

namespace HPHP {

int64_t HSLLocale::Ops::normalize_offset(int64_t offset, int64_t length) {
  const auto normalized = offset >= 0 ? offset : offset + length;
  if (normalized < 0 || normalized > length) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat(
        "Offset {} was out-of-bounds for length {}", 
        offset,
        length
      )
    );
  }
  return normalized;
}

} // namespace HPHP
