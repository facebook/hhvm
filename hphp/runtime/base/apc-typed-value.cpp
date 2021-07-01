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

#include "hphp/runtime/base/apc-bespoke.h"
#include "hphp/runtime/base/tv-uncounted.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

namespace HPHP {

APCTypedValue* APCTypedValue::ForArray(ArrayData* ad) {
  assertx(!ad->isRefCounted());
  auto const dt = ad->toPersistentDataType();
  auto const result = initAPCBespoke(ad);
  ad = result.ad;

  // If we made an APCBespoke, it'll always be part of a joint allocation.
  if (result.tv) {
    auto const kind = ad->isStatic() ? APCKind::StaticBespoke
                                     : APCKind::UncountedBespoke;
    return new (result.tv) APCTypedValue(ad, kind, dt);
  }

  // We didn't make an APCBespoke. Just use a regular persistent array.
  auto const kind = ad->isStatic() ? APCKind::StaticArray
                                   : APCKind::UncountedArray;

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
  m_data.arr.store(ad, std::memory_order_release);
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
      assertx(m_data.pclsmeth->getCls()->isPersistent()); break;
    case APCKind::StaticString: assertx(m_data.str->isStatic()); break;
    case APCKind::UncountedString: assertx(m_data.str->isUncounted()); break;
    case APCKind::LazyClass: assertx(m_data.str->isStatic()); break;

    case APCKind::StaticArray:
    case APCKind::StaticBespoke: {
      DEBUG_ONLY auto const ad = m_data.arr.load(std::memory_order_acquire);
      assertx(ad->isStatic());
      assertx(ad->toPersistentDataType() == m_handle.type());
      break;
    }

    case APCKind::UncountedArray:
    case APCKind::UncountedBespoke: {
      DEBUG_ONLY auto const ad = m_data.arr.load(std::memory_order_acquire);
      assertx(ad->isUncounted());
      assertx(ad->toPersistentDataType() == m_handle.type());
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
  static_assert(std::is_trivially_destructible<APCTypedValue>::value,
                "APCTypedValue must be trivially destructible - "
                "*Array::ReleaseUncounted() frees the memory without "
                "destroying it");

  switch (m_handle.kind()) {
    case APCKind::UncountedArray: {
      auto const ad = m_data.arr.load(std::memory_order_acquire);
      DecRefUncountedArray(ad);
      if (ad != static_cast<void*>(this + 1)) {
        delete this;
      }
      return;
    }

    case APCKind::UncountedBespoke:
      freeAPCBespoke(this);
      return;

    case APCKind::UncountedString:
      DecRefUncountedString(m_data.str);
      delete this;
      return;

    default:
      always_assert(false);
  }
}

ArrayData* APCTypedValue::getArrayData() const {
  assertx(checkInvariants());
  return m_data.arr.load(std::memory_order_acquire);
}

void APCTypedValue::setArrayData(ArrayData* ad) {
  m_data.arr.store(ad, std::memory_order_release);
  assertx(checkInvariants());
}

TypedValue APCTypedValue::toTypedValue() const {
  assertx(m_handle.isTypedValue());
  TypedValue tv;
  tv.m_type = m_handle.type();
  if (m_handle.kind() == APCKind::UncountedBespoke) {
    tv.m_data.parr = readAPCBespoke(this);
  } else {
    tv.m_data.num = m_data.num;
  }
  return tv;
}

//////////////////////////////////////////////////////////////////////

}
