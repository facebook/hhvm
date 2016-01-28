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
#include "hphp/runtime/base/apc-handle.h"

#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/apc-string.h"
#include "hphp/runtime/base/apc-array.h"
#include "hphp/runtime/base/apc-object.h"
#include "hphp/runtime/base/apc-collection.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/ext/apc/ext_apc.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/apc-local-array-defs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

APCHandle::Pair APCHandle::Create(const Variant& source,
                                  bool serialized,
                                  APCHandleLevel level,
                                  bool unserializeObj) {

  auto type = source.getType(); // this gets rid of the ref, if it was one
  switch (type) {
    case KindOfUninit: {
      auto value = APCTypedValue::tvUninit();
      return {value->getHandle(), sizeof(APCTypedValue)};
    }
    case KindOfNull: {
      auto value = APCTypedValue::tvNull();
      return {value->getHandle(), sizeof(APCTypedValue)};
    }
    case KindOfBoolean: {
      auto value = source.getBoolean() ? APCTypedValue::tvTrue()
                                       : APCTypedValue::tvFalse();
      return {value->getHandle(), sizeof(APCTypedValue)};
    }
    case KindOfInt64: {
      auto value = new APCTypedValue(source.getInt64());
      return {value->getHandle(), sizeof(APCTypedValue)};
    }
    case KindOfDouble: {
      auto value = new APCTypedValue(source.getDouble());
      return {value->getHandle(), sizeof(APCTypedValue)};
    }
    case KindOfPersistentString:
    case KindOfString: {
      StringData* s = source.getStringData();
      if (serialized) {
        // It is priming, and there might not be the right class definitions
        // for unserialization.
        return APCString::MakeSerializedObject(apc_reserialize(String{s}));
      }
      if (s->isStatic()) {
        auto value = new APCTypedValue(APCTypedValue::StaticStr{}, s);
        return APCHandle::Pair{value->getHandle(), sizeof(APCTypedValue)};
      }
      auto const st = lookupStaticString(s);
      if (st) {
        auto value = new APCTypedValue(APCTypedValue::StaticStr{}, st);
        return {value->getHandle(), sizeof(APCTypedValue)};
      }
      if (level == APCHandleLevel::Outer && apcExtension::UseUncounted) {
        auto st = StringData::MakeUncounted(s->slice());
        auto value = new APCTypedValue(APCTypedValue::UncountedStr{}, st);
        return {value->getHandle(), st->size() + sizeof(APCTypedValue)};
      }
      return APCString::MakeSharedString(s);
    }

    case KindOfPersistentArray:
    case KindOfArray: {
      auto ad = source.getArrayData();
      if (ad->isStatic()) {
        auto value = new APCTypedValue(APCTypedValue::StaticArr{}, ad);
        return {value->getHandle(), sizeof(APCTypedValue)};
      }

      return APCArray::MakeSharedArray(source.getArrayData(), level,
                                       unserializeObj);
    }

    case KindOfObject:
      if (source.getObjectData()->isCollection()) {
        return APCCollection::Make(source.getObjectData(),
                                   level,
                                   unserializeObj);
      }
      return unserializeObj ? APCObject::Construct(source.getObjectData()) :
             APCString::MakeSerializedObject(apc_serialize(source));

    case KindOfResource:
      // TODO Task #2661075: Here and elsewhere in the runtime, we convert
      // Resources to the empty array during various serialization operations,
      // which does not match Zend behavior. We should fix this.
      return APCArray::MakeSharedEmptyArray();

    case KindOfRef:
    case KindOfClass:
      return {nullptr, 0};
  }
  not_reached();
}

Variant APCHandle::toLocal() const {
  switch (m_kind) {
    case APCKind::Uninit:
    case APCKind::Null:
      return init_null(); // shortcut.. no point to forward
    case APCKind::Bool:
      return APCTypedValue::fromHandle(this)->getBoolean();
    case APCKind::Int:
      return APCTypedValue::fromHandle(this)->getInt64();
    case APCKind::Double:
      return APCTypedValue::fromHandle(this)->getDouble();
    case APCKind::StaticString:
    case APCKind::UncountedString:
      return Variant{APCTypedValue::fromHandle(this)->getStringData(),
                     Variant::PersistentStrInit{}};
    case APCKind::SharedString:
      return Variant::attach(
        StringData::MakeProxy(APCString::fromHandle(this))
      );
    case APCKind::StaticArray:
    case APCKind::UncountedArray:
      return Variant{APCTypedValue::fromHandle(this)->getArrayData(),
                     Variant::PersistentArrInit{}};
    case APCKind::SerializedArray: {
      auto const serArr = APCString::fromHandle(this)->getStringData();
      return apc_unserialize(serArr->data(), serArr->size());
    }
    case APCKind::SharedArray:
    case APCKind::SharedPackedArray:
      return Variant::attach(
        APCLocalArray::Make(APCArray::fromHandle(this))->asArrayData()
      );
    case APCKind::SerializedObject: {
      auto const serObj = APCString::fromHandle(this)->getStringData();
      return apc_unserialize(serObj->data(), serObj->size());
    }
    case APCKind::SharedCollection:
      return APCCollection::fromHandle(this)->createObject();
    case APCKind::SharedObject:
      return APCObject::MakeLocalObject(this);
  }
  not_reached();
}

void APCHandle::deleteShared() {
  assert(checkInvariants());
  switch (m_kind) {
    case APCKind::Uninit:
    case APCKind::Null:
    case APCKind::Bool:
      return;
    case APCKind::Int:
    case APCKind::Double:
    case APCKind::StaticString:
    case APCKind::StaticArray:
      delete APCTypedValue::fromHandle(this);
      return;

    case APCKind::SharedString:
    case APCKind::SerializedArray:
    case APCKind::SerializedObject:
      APCString::Delete(APCString::fromHandle(this));
      return;

    case APCKind::SharedPackedArray:
    case APCKind::SharedArray:
      APCArray::Delete(this);
      return;

    case APCKind::SharedObject:
      APCObject::Delete(this);
      return;

    case APCKind::SharedCollection:
      APCCollection::Delete(this);
      return;

    case APCKind::UncountedArray:
    case APCKind::UncountedString:
      assert(false);
      return;
  }
  not_reached();
}

bool APCHandle::checkInvariants() const {
  switch (m_kind) {
    case APCKind::Uninit:
      assert(m_type == KindOfUninit);
      return true;
    case APCKind::Null:
      assert(m_type == KindOfNull);
      return true;
    case APCKind::Bool:
      assert(m_type == KindOfBoolean);
      return true;
    case APCKind::Int:
      assert(m_type == KindOfInt64);
      return true;
    case APCKind::Double:
      assert(m_type == KindOfDouble);
      return true;
    case APCKind::StaticString:
    case APCKind::UncountedString:
      assert(m_type == KindOfPersistentString);
      return true;
    case APCKind::StaticArray:
    case APCKind::UncountedArray:
      assert(m_type == KindOfPersistentArray);
      return true;
    case APCKind::SharedString:
    case APCKind::SharedArray:
    case APCKind::SharedPackedArray:
    case APCKind::SharedObject:
    case APCKind::SharedCollection:
    case APCKind::SerializedArray:
    case APCKind::SerializedObject:
      assert(m_type == kInvalidDataType);
      return true;
  }
  not_reached();
  return false;
}

//////////////////////////////////////////////////////////////////////

}
