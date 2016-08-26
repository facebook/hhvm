/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
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

bool APCTypedValue::checkInvariants() const {
  assert(m_handle.checkInvariants());
  switch (m_handle.kind()) {
    case APCKind::Uninit:
    case APCKind::Null: assert(m_data.num == 0); break;
    case APCKind::Bool:
    case APCKind::Int:
    case APCKind::Double: break;
    case APCKind::StaticString: assert(m_data.str->isStatic()); break;
    case APCKind::UncountedString: assert(m_data.str->isUncounted()); break;
    case APCKind::StaticArray:
      assert(m_data.arr->isPHPArray());
      assert(m_data.arr->isStatic());
      break;
    case APCKind::StaticVec:
      assert(m_data.vec->isVecArray());
      assert(m_data.vec->isStatic());
      break;
    case APCKind::StaticDict:
      assert(m_data.dict->isDict());
      assert(m_data.dict->isStatic());
      break;
    case APCKind::StaticKeyset:
      assert(m_data.keyset->isKeyset());
      assert(m_data.keyset->isStatic());
      break;
    case APCKind::UncountedArray:
      assert(m_data.arr->isPHPArray());
      assert(m_data.arr->isUncounted());
      break;
    case APCKind::UncountedVec:
      assert(m_data.vec->isVecArray());
      assert(m_data.vec->isUncounted());
      break;
    case APCKind::UncountedDict:
      assert(m_data.dict->isDict());
      assert(m_data.dict->isUncounted());
      break;
    case APCKind::UncountedKeyset:
      assert(m_data.keyset->isKeyset());
      assert(m_data.keyset->isUncounted());
      break;
    case APCKind::SharedString:
    case APCKind::SharedArray:
    case APCKind::SharedPackedArray:
    case APCKind::SharedObject:
    case APCKind::SharedCollection:
    case APCKind::SharedVec:
    case APCKind::SharedDict:
    case APCKind::SharedKeyset:
    case APCKind::SerializedArray:
    case APCKind::SerializedObject:
    case APCKind::SerializedVec:
    case APCKind::SerializedDict:
    case APCKind::SerializedKeyset:
      assert(false);
      break;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

void APCTypedValue::deleteUncounted() {
  assert(m_handle.isUncounted());
  auto kind = m_handle.kind();
  assert(kind == APCKind::UncountedString ||
         kind == APCKind::UncountedArray ||
         kind == APCKind::UncountedVec ||
         kind == APCKind::UncountedDict ||
         kind == APCKind::UncountedKeyset);
  if (kind == APCKind::UncountedString) {
    m_data.str->destructUncounted();
  } else if (kind == APCKind::UncountedArray) {
    assert(m_data.arr->isPHPArray());
    if (m_data.arr->hasPackedLayout()) {
      auto arr = m_data.arr;
      this->~APCTypedValue();
      PackedArray::ReleaseUncounted(arr, sizeof(APCTypedValue));
      return;  // Uncounted PackedArray frees the joint allocation.
    } else {
      auto arr = m_data.arr;
      this->~APCTypedValue();
      MixedArray::ReleaseUncounted(arr, sizeof(APCTypedValue));
      return;  // Uncounted MixedArray frees the joint allocation.
    }
  } else if (kind == APCKind::UncountedVec) {
    auto vec = m_data.vec;
    assert(vec->isVecArray());
    this->~APCTypedValue();
    PackedArray::ReleaseUncounted(vec, sizeof(APCTypedValue));
    return;
  } else if (kind == APCKind::UncountedDict) {
    auto dict = m_data.dict;
    assert(dict->isDict());
    this->~APCTypedValue();
    MixedArray::ReleaseUncounted(dict, sizeof(APCTypedValue));
    return;
  } else if (kind == APCKind::UncountedKeyset) {
    auto keyset = m_data.keyset;
    assert(keyset->isKeyset());
    this->~APCTypedValue();
    SetArray::ReleaseUncounted(keyset, sizeof(APCTypedValue));
    return;
  }

  delete this;
}

//////////////////////////////////////////////////////////////////////

}
