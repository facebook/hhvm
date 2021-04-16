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
APCKind getAPCKind(DataType dt, bool isStatic) {
  switch (dt) {
    case KindOfPersistentVec:
      return isStatic ? APCKind::StaticVec : APCKind::UncountedVec;
    case KindOfPersistentDict:
      return isStatic ? APCKind::StaticDict : APCKind::UncountedDict;
    case KindOfPersistentKeyset:
      return isStatic ? APCKind::StaticKeyset : APCKind::UncountedKeyset;
    default:
      always_assert(false);
  }
}
}

APCTypedValue* APCTypedValue::ForArray(ArrayData* ad) {
  assertx(ad->isVanilla());
  assertx(!ad->isRefCounted());

  auto const dt = ad->toPersistentDataType();
  auto const kind = getAPCKind(dt, ad->isStatic());

  // Check if the "co-allocate array and APCTypedValue" optimization hit.
  // It hit if we a) made a new uncounted array and b) its flag is set.
  if (ad->uncountedCowCheck() || !ad->hasApcTv()) {
    return new APCTypedValue(ad, kind, dt);
  }

  auto const mem = reinterpret_cast<APCTypedValue*>(ad) - 1;
  return new (mem) APCTypedValue(ad, kind, dt);
}

APCTypedValue::APCTypedValue(ArrayData* ad, APCKind kind, DataType dt)
    : m_handle(kind, dt) {
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
    case APCKind::StaticDict:
    case APCKind::StaticKeyset:
    case APCKind::UncountedVec:
    case APCKind::UncountedDict:
    case APCKind::UncountedKeyset: {
      DEBUG_ONLY auto const ad = m_data.arr;
      DEBUG_ONLY auto const dt = ad->toPersistentDataType();
      assertx(m_handle.kind() == getAPCKind(dt, ad->isStatic()));
      break;
    }

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

ArrayData* APCTypedValue::getArrayData() const {
  assertx(checkInvariants());
  return m_data.arr;
}

TypedValue APCTypedValue::toTypedValue() const {
  assertx(m_handle.isTypedValue());
  TypedValue tv;
  tv.m_type = m_handle.type();
  tv.m_data.num = m_data.num;
  return tv;
}

//////////////////////////////////////////////////////////////////////

}
