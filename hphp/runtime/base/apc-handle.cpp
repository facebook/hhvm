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
      if (apcExtension::UseUncounted) {
        auto st = StringData::MakeUncounted(s->slice());
        auto value = new APCTypedValue(APCTypedValue::UncountedStr{}, st);
        return {value->getHandle(), st->size() + sizeof(APCTypedValue)};
      }
      return APCString::MakeSharedString(s);
    }

    case KindOfPersistentVec:
    case KindOfVec: {
      auto ad = source.getArrayData();
      assert(ad->isVecArray());
      if (ad->isStatic()) {
        auto value = new APCTypedValue(APCTypedValue::StaticVec{}, ad);
        return {value->getHandle(), sizeof(APCTypedValue)};
      }

      return APCArray::MakeSharedVec(ad, level, unserializeObj);
    }

    case KindOfPersistentDict:
    case KindOfDict: {
      auto ad = source.getArrayData();
      assert(ad->isDict());
      if (ad->isStatic()) {
        auto value = new APCTypedValue(APCTypedValue::StaticDict{}, ad);
        return {value->getHandle(), sizeof(APCTypedValue)};
      }

      return APCArray::MakeSharedDict(ad, level, unserializeObj);
    }

    case KindOfPersistentKeyset:
    case KindOfKeyset: {
      auto ad = source.getArrayData();
      assert(ad->isKeyset());
      if (ad->isStatic()) {
        auto value = new APCTypedValue(APCTypedValue::StaticKeyset{}, ad);
        return {value->getHandle(), sizeof(APCTypedValue)};
      }

      return APCArray::MakeSharedKeyset(ad, level, unserializeObj);
    }

    case KindOfPersistentArray:
    case KindOfArray: {
      auto ad = source.getArrayData();
      assert(ad->isPHPArray());
      if (ad->isStatic()) {
        auto value = new APCTypedValue(APCTypedValue::StaticArr{}, ad);
        return {value->getHandle(), sizeof(APCTypedValue)};
      }

      return APCArray::MakeSharedArray(ad, level, unserializeObj);
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
                     KindOfPersistentArray,
                     Variant::PersistentArrInit{}};
    case APCKind::StaticVec:
    case APCKind::UncountedVec:
      return Variant{APCTypedValue::fromHandle(this)->getVecData(),
                     KindOfPersistentVec,
                     Variant::PersistentArrInit{}};
    case APCKind::StaticDict:
    case APCKind::UncountedDict:
      return Variant{APCTypedValue::fromHandle(this)->getDictData(),
                     KindOfPersistentDict,
                     Variant::PersistentArrInit{}};
    case APCKind::StaticKeyset:
    case APCKind::UncountedKeyset:
      return Variant{APCTypedValue::fromHandle(this)->getKeysetData(),
                     KindOfPersistentKeyset,
                     Variant::PersistentArrInit{}};
    case APCKind::SerializedArray: {
      auto const serArr = APCString::fromHandle(this)->getStringData();
      auto const v = apc_unserialize(serArr->data(), serArr->size());
      assert(v.isPHPArray());
      return v;
    }
    case APCKind::SerializedVec: {
      auto const serVec = APCString::fromHandle(this)->getStringData();
      auto const v = apc_unserialize(serVec->data(), serVec->size());
      assert(v.isVecArray());
      return v;
    }
    case APCKind::SerializedDict: {
      auto const serDict = APCString::fromHandle(this)->getStringData();
      auto const v = apc_unserialize(serDict->data(), serDict->size());
      assert(v.isDict());
      return v;
    }
    case APCKind::SerializedKeyset: {
      auto const serKeyset = APCString::fromHandle(this)->getStringData();
      auto const v = apc_unserialize(serKeyset->data(), serKeyset->size());
      assert(v.isKeyset());
      return v;
    }
    case APCKind::SharedArray:
    case APCKind::SharedPackedArray:
      return Variant::attach(
        APCLocalArray::Make(APCArray::fromHandle(this))->asArrayData()
      );
    case APCKind::SharedVec:
      return Variant::attach(
        APCArray::fromHandle(this)->toLocalVec()
      );
    case APCKind::SharedDict:
      return Variant::attach(
        APCArray::fromHandle(this)->toLocalDict()
      );
    case APCKind::SharedKeyset:
      return Variant::attach(
        APCArray::fromHandle(this)->toLocalKeyset()
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
    case APCKind::StaticVec:
    case APCKind::StaticDict:
    case APCKind::StaticKeyset:
      delete APCTypedValue::fromHandle(this);
      return;

    case APCKind::SharedString:
    case APCKind::SerializedArray:
    case APCKind::SerializedVec:
    case APCKind::SerializedDict:
    case APCKind::SerializedKeyset:
    case APCKind::SerializedObject:
      APCString::Delete(APCString::fromHandle(this));
      return;

    case APCKind::SharedPackedArray:
    case APCKind::SharedArray:
    case APCKind::SharedVec:
    case APCKind::SharedDict:
    case APCKind::SharedKeyset:
      APCArray::Delete(this);
      return;

    case APCKind::SharedObject:
      APCObject::Delete(this);
      return;

    case APCKind::SharedCollection:
      APCCollection::Delete(this);
      return;

    case APCKind::UncountedArray:
    case APCKind::UncountedVec:
    case APCKind::UncountedDict:
    case APCKind::UncountedKeyset:
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
    case APCKind::StaticVec:
    case APCKind::UncountedVec:
      assert(m_type == KindOfPersistentVec);
      return true;
    case APCKind::StaticDict:
    case APCKind::UncountedDict:
      assert(m_type == KindOfPersistentDict);
      return true;
    case APCKind::StaticKeyset:
    case APCKind::UncountedKeyset:
      assert(m_type == KindOfPersistentKeyset);
      return true;
    case APCKind::SharedString:
    case APCKind::SharedArray:
    case APCKind::SharedPackedArray:
    case APCKind::SharedVec:
    case APCKind::SharedDict:
    case APCKind::SharedKeyset:
    case APCKind::SharedObject:
    case APCKind::SharedCollection:
    case APCKind::SerializedArray:
    case APCKind::SerializedVec:
    case APCKind::SerializedDict:
    case APCKind::SerializedKeyset:
    case APCKind::SerializedObject:
      assert(m_type == kInvalidDataType);
      return true;
  }
  not_reached();
  return false;
}

//////////////////////////////////////////////////////////////////////

}
