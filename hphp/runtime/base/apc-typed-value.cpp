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

#include "hphp/runtime/base/tv-uncounted.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

namespace HPHP {

namespace {
APCKind getAPCKind(const ArrayData* ad) {
  switch (ad->toPersistentDataType()) {
    case KindOfPersistentVec:
      return ad->isStatic() ? APCKind::StaticVec : APCKind::UncountedVec;
    case KindOfPersistentDict:
      return ad->isStatic() ? APCKind::StaticDict : APCKind::UncountedDict;
    case KindOfPersistentKeyset:
      return ad->isStatic() ? APCKind::StaticKeyset : APCKind::UncountedKeyset;
    default:
      always_assert(false);
  }
}
}

APCTypedValue::APCTypedValue(ArrayData* ad)
    : m_handle(getAPCKind(ad), ad->toPersistentDataType()) {
  assertx(!ad->isRefCounted());
  m_data.arr = ad;
  assertx(checkInvariants());
}

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
    case APCKind::PersistentClass: assertx(m_data.cls->isPersistent()); break;
    case APCKind::PersistentClsMeth:
      assertx(use_lowptr);
      assertx(m_data.pclsmeth->getCls()->isPersistent()); break;
    case APCKind::StaticString: assertx(m_data.str->isStatic()); break;
    case APCKind::UncountedString: assertx(m_data.str->isUncounted()); break;
    case APCKind::LazyClass: assertx(m_data.str->isStatic()); break;
    case APCKind::StaticVec:
      assertx(m_data.arr->isVecType());
      assertx(m_data.arr->isStatic());
      break;
    case APCKind::StaticDict:
      assertx(m_data.arr->isDictType());
      assertx(m_data.arr->isStatic());
      break;
    case APCKind::StaticKeyset:
      assertx(m_data.arr->isKeysetType());
      assertx(m_data.arr->isStatic());
      break;
    case APCKind::UncountedVec:
      assertx(m_data.arr->isVecType());
      assertx(m_data.arr->isUncounted());
      break;
    case APCKind::UncountedDict:
      assertx(m_data.arr->isDictType());
      assertx(m_data.arr->isUncounted());
      break;
    case APCKind::UncountedKeyset:
      assertx(m_data.arr->isKeysetType());
      assertx(m_data.arr->isUncounted());
      break;
    case APCKind::FuncEntity:
    case APCKind::ClassEntity:
    case APCKind::ClsMeth:
    case APCKind::RFunc:
    case APCKind::RClsMeth:
    case APCKind::SharedString:
    case APCKind::SharedObject:
    case APCKind::SharedCollection:
    case APCKind::SharedVec:
    case APCKind::SharedLegacyVec:
    case APCKind::SharedDict:
    case APCKind::SharedLegacyDict:
    case APCKind::SharedKeyset:
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
          kind == APCKind::UncountedVec ||
          kind == APCKind::UncountedDict ||
          kind == APCKind::UncountedKeyset);

  static_assert(std::is_trivially_destructible<APCTypedValue>::value,
                "APCTypedValue must be trivially destructible - "
                "*Array::ReleaseUncounted() frees the memory without "
                "destroying it");

  if (kind == APCKind::UncountedString) {
    DecRefUncountedString(m_data.str);
  } else {
    auto const arr = m_data.arr;
    DecRefUncountedArray(arr);
    if (arr == static_cast<void*>(this + 1)) {
      return;  // *::ReleaseUncounted freed the joint allocation.
    }
  }

  delete this;
}

//////////////////////////////////////////////////////////////////////

}
