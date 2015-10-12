/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/apc-typed-value.h"

#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/struct-array.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

namespace HPHP {

APCTypedValue* APCTypedValue::tvUninit() {
  static APCTypedValue* value = new APCTypedValue(KindOfUninit);
  return value;
}

APCTypedValue* APCTypedValue::tvNull() {
  static APCTypedValue* value = new APCTypedValue(KindOfNull);
  return value;
}

APCTypedValue* APCTypedValue::tvTrue() {
  static auto value = new APCTypedValue(APCTypedValue::Bool{}, true);
  return value;
}

APCTypedValue* APCTypedValue::tvFalse() {
  static auto value = new APCTypedValue(APCTypedValue::Bool{}, false);
  return value;
}

//////////////////////////////////////////////////////////////////////

void APCTypedValue::deleteUncounted() {
  assert(m_handle.isUncounted());
  DataType type = m_handle.type();
  assert(type == KindOfString || type == KindOfArray);
  if (type == KindOfString) {
    m_data.str->destructUncounted();
  } else if (type == KindOfArray) {
    if (m_data.arr->isPacked()) {
      MixedArray::ReleaseUncountedPacked(m_data.arr);
    } else if (m_data.arr->isStruct()) {
      StructArray::ReleaseUncounted(m_data.arr);
    } else {
      MixedArray::ReleaseUncounted(m_data.arr);
    }
  }
  delete this;
}

//////////////////////////////////////////////////////////////////////

}

