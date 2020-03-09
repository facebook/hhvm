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
  assertx(m_handle.checkInvariants());
  switch (m_handle.kind()) {
    case APCKind::Uninit:
    case APCKind::Null: assertx(m_data.num == 0); break;
    case APCKind::Bool:
    case APCKind::Int:
    case APCKind::Double: break;
    case APCKind::PersistentFunc: assertx(m_data.func->isPersistent()); break;
    case APCKind::StaticString: assertx(m_data.str->isStatic()); break;
    case APCKind::UncountedString: assertx(m_data.str->isUncounted()); break;
    case APCKind::StaticArray:
      assertx(m_data.arr->isPHPArrayKind());
      assertx(m_data.arr->isStatic());
      break;
    case APCKind::StaticVec:
      assertx(m_data.vec->isVecArrayKind());
      assertx(m_data.vec->isStatic());
      break;
    case APCKind::StaticDict:
      assertx(m_data.dict->isDictKind());
      assertx(m_data.dict->isStatic());
      break;
    case APCKind::StaticKeyset:
      assertx(m_data.keyset->isKeysetKind());
      assertx(m_data.keyset->isStatic());
      break;
    case APCKind::UncountedArray:
      assertx(m_data.arr->isPHPArrayKind());
      assertx(m_data.arr->isUncounted());
      break;
    case APCKind::UncountedVec:
      assertx(m_data.vec->isVecArrayKind());
      assertx(m_data.vec->isUncounted());
      break;
    case APCKind::UncountedDict:
      assertx(m_data.dict->isDictKind());
      assertx(m_data.dict->isUncounted());
      break;
    case APCKind::UncountedKeyset:
      assertx(m_data.keyset->isKeysetKind());
      assertx(m_data.keyset->isUncounted());
      break;
    case APCKind::FuncEntity:
    case APCKind::SharedString:
    case APCKind::SharedArray:
    case APCKind::SharedPackedArray:
    case APCKind::SharedObject:
    case APCKind::SharedCollection:
    case APCKind::SharedVec:
    case APCKind::SharedDict:
    case APCKind::SharedKeyset:
    case APCKind::SharedVArray:
    case APCKind::SharedDArray:
    case APCKind::SerializedArray:
    case APCKind::SerializedObject:
    case APCKind::SerializedVec:
    case APCKind::SerializedDict:
    case APCKind::SerializedKeyset:
      assertx(false);
      break;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

void APCTypedValue::deleteUncounted() {
  assertx(m_handle.isUncounted());
  auto kind = m_handle.kind();
  assertx(kind == APCKind::UncountedString ||
         kind == APCKind::UncountedArray ||
         kind == APCKind::UncountedVec ||
         kind == APCKind::UncountedDict ||
         kind == APCKind::UncountedKeyset);

  static_assert(std::is_trivially_destructible<APCTypedValue>::value,
                "APCTypedValue must be trivially destructible - "
                "*Array::ReleaseUncounted() frees the memory without "
                "destroying it");

  if (kind == APCKind::UncountedString) {
    StringData::ReleaseUncounted(m_data.str);
  } else {
    auto const arr = [&] {
      if (kind == APCKind::UncountedArray) {
        auto const parr = m_data.arr;
        assertx(parr->isPHPArrayKind());
        if (parr->hasVanillaPackedLayout()) {
          PackedArray::ReleaseUncounted(parr);
        } else {
          assertx(parr->hasVanillaMixedLayout());
          MixedArray::ReleaseUncounted(parr);
        }
        return parr;
      }
      if (kind == APCKind::UncountedVec) {
        auto const vec = m_data.vec;
        assertx(vec->isVecArrayKind());
        PackedArray::ReleaseUncounted(vec);
        return vec;
      }
      if (kind == APCKind::UncountedDict) {
        auto const dict = m_data.dict;
        assertx(dict->isDictKind());
        MixedArray::ReleaseUncounted(dict);
        return dict;
      }
      assertx(kind == APCKind::UncountedKeyset);
      auto const keyset = m_data.keyset;
      assertx(keyset->isKeysetKind());
      SetArray::ReleaseUncounted(keyset);
      return keyset;
    }();
    if (arr == static_cast<void*>(this + 1)) {
      return;  // *::ReleaseUncounted freed the joint allocation.
    }
  }

  delete this;
}

//////////////////////////////////////////////////////////////////////

}
