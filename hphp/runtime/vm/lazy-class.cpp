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

#include "hphp/runtime/vm/lazy-class.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/strings.h"

#include <folly/Random.h>

namespace HPHP {
LazyClassData::LazyClassData(const StringData* name)
  : className(name) {
  assertx(name && name->isStatic());
}

const StringData* lazyClassToStringHelper(const LazyClassData& lclass) {
  if (folly::Random::oneIn(RO::EvalRaiseClassConversionNoticeSampleRate)) {
    raise_class_to_string_conversion_notice();
  }
  return lclass.name();
}

LazyClassData LazyClassData::create(const StringData* cname) {
  return LazyClassData(cname);
}
}
