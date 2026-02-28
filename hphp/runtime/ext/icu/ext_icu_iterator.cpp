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
#include "hphp/runtime/ext/icu/ext_icu_iterator.h"

namespace HPHP::Intl {
//////////////////////////////////////////////////////////////////////////////

#define II_GET(dest, src, def) \
  auto dest = IntlIterator::Get(src); \
  if (!dest) { \
    return def; \
  }

#if U_ICU_VERSION_MAJOR_NUM * 10 + U_ICU_VERSION_MINOR_NUM >= 42
UOBJECT_DEFINE_RTTI_IMPLEMENTATION(BugStringCharEnumeration)
#endif

//////////////////////////////////////////////////////////////////////////////
// class IntlIterator

static Variant HHVM_METHOD(IntlIterator, current) {
  II_GET(data, this_, false);
  return data->current();
}

static Variant HHVM_METHOD(IntlIterator, key) {
  II_GET(data, this_, false);
  return data->key();
}

static Variant HHVM_METHOD(IntlIterator, next) {
  II_GET(data, this_, false);
  return data->next();
}

static Variant HHVM_METHOD(IntlIterator, rewind) {
  II_GET(data, this_, false);
  data->rewind();
  return data->current();
}

static bool HHVM_METHOD(IntlIterator, valid) {
  II_GET(data, this_, false);
  return data->valid();
}

//////////////////////////////////////////////////////////////////////////////

void IntlExtension::registerNativeIterator() {
  HHVM_ME(IntlIterator, current);
  HHVM_ME(IntlIterator, key);
  HHVM_ME(IntlIterator, next);
  HHVM_ME(IntlIterator, rewind);
  HHVM_ME(IntlIterator, valid);

  Native::registerNativeDataInfo<IntlIterator>();
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl
