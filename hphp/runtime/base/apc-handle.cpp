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
#include "hphp/runtime/base/apc-named-entity.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/ext/apc/ext_apc.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/apc-local-array-defs.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

APCHandle::Pair APCHandle::Create(const_variant_ref source,
                                  bool serialized,
                                  APCHandleLevel level,
                                  bool unserializeObj) {
  auto const cell = source.asTypedValue();
  switch (cell.type()) {
    case KindOfUninit: {
      auto const value = APCTypedValue::tvUninit();
      return {value->getHandle(), sizeof(APCTypedValue)};
    }
    case KindOfNull: {
      auto const value = APCTypedValue::tvNull();
      return {value->getHandle(), sizeof(APCTypedValue)};
    }
    case KindOfBoolean: {
      auto const value = val(cell).num ? APCTypedValue::tvTrue()
                                       : APCTypedValue::tvFalse();
      return {value->getHandle(), sizeof(APCTypedValue)};
    }
    case KindOfInt64: {
      auto const value = new APCTypedValue(val(cell).num);
      return {value->getHandle(), sizeof(APCTypedValue)};
    }
    case KindOfDouble: {
      auto const value = new APCTypedValue(val(cell).dbl);
      return {value->getHandle(), sizeof(APCTypedValue)};
    }
    case KindOfFunc: {
      auto const func = val(cell).pfunc;
      auto const serialize_func =
        RuntimeOption::EvalAPCSerializeFuncs &&
        // Right now cls_meth() can serialize as an array, and attempting to
        // recursively serialize elements in the array will eventually attempt
        // to serialize a method pointer.
        !func->isMethod();
      if (serialize_func) {
        if (func->isPersistent()) {
          auto const value = new APCTypedValue(func);
          return {value->getHandle(), sizeof(APCTypedValue)};
        }
        auto const value = new APCNamedEntity(func);
        return {value->getHandle(), sizeof(APCNamedEntity)};
      }
      // fallthrough to string serialization
    }
    case KindOfClass:
    case KindOfPersistentString:
    case KindOfString: {
      auto const s =
        isFuncType(cell.type())
          ? const_cast<StringData*>(funcToStringHelper(val(cell).pfunc)) :
        isClassType(cell.type())
          ? const_cast<StringData*>(classToStringHelper(val(cell).pclass))
          : val(cell).pstr;
      if (serialized) {
        // It is priming, and there might not be the right class definitions
        // for unserialization.
        return APCString::MakeSerializedObject(apc_reserialize(String{s}));
      }
      if (auto const value = APCTypedValue::HandlePersistent(
            APCTypedValue::StaticStr{}, APCTypedValue::UncountedStr{}, s)) {
        return value;
      }
      auto const st = lookupStaticString(s);
      if (st) {
        auto const value = new APCTypedValue(APCTypedValue::StaticStr{}, st);
        return {value->getHandle(), sizeof(APCTypedValue)};
      }
      if (apcExtension::UseUncounted) {
        auto const st = StringData::MakeUncounted(s->slice());
        auto const value = new APCTypedValue(APCTypedValue::UncountedStr{}, st);
        return {value->getHandle(), st->size() + sizeof(APCTypedValue)};
      }
      return APCString::MakeSharedString(s);
    }

    case KindOfPersistentVec:
    case KindOfVec: {
      auto const ad = val(cell).parr;
      assertx(ad->isVecArrayType());
      return APCArray::MakeSharedVec(ad, level, unserializeObj);
    }

    case KindOfPersistentDict:
    case KindOfDict: {
      auto const ad = val(cell).parr;
      assertx(ad->isDictType());
      return APCArray::MakeSharedDict(ad, level, unserializeObj);
    }

    case KindOfPersistentKeyset:
    case KindOfKeyset: {
      auto const ad = val(cell).parr;
      assertx(ad->isKeysetType());
      return APCArray::MakeSharedKeyset(ad, level, unserializeObj);
    }

    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray: {
      auto const ad = val(cell).parr;
      assertx(ad->isPHPArrayType());
      return APCArray::MakeSharedArray(ad, level, unserializeObj);
    }

    case KindOfObject:
      if (val(cell).pobj->isCollection()) {
        return APCCollection::Make(val(cell).pobj,
                                   level,
                                   unserializeObj);
      }
      return unserializeObj ? APCObject::Construct(val(cell).pobj) :
             APCString::MakeSerializedObject(apc_serialize(source));

    case KindOfResource:
      // TODO Task #2661075: Here and elsewhere in the runtime, we convert
      // Resources to the empty array during various serialization operations,
      // which does not match Zend behavior. We should fix this.
      return APCArray::MakeSharedEmptyArray();
    case KindOfClsMeth: {
      raiseClsMethToVecWarningHelper();
      auto arr = clsMethToVecHelper(val(cell).pclsmeth);
      if (RuntimeOption::EvalHackArrDVArrs) {
        assertx(arr->isVecArrayType());
        return APCArray::MakeSharedVec(arr.detach(), level, unserializeObj);
      } else {
        assertx(arr->isPHPArrayType());
        return APCArray::MakeSharedArray(arr.detach(), level, unserializeObj);
      }
    }

    case KindOfRecord: // TODO (T41019518)
      raise_error(Strings::RECORD_NOT_SUPPORTED);
  }
  not_reached();
}

Variant APCHandle::toLocalHelper() const {
  assertx(!isTypedValue());
  switch (m_kind) {
    case APCKind::Uninit:
    case APCKind::Null:
    case APCKind::Bool:
    case APCKind::Int:
    case APCKind::Double:
    case APCKind::PersistentFunc:
    case APCKind::StaticString:
    case APCKind::UncountedString:
    case APCKind::StaticArray:
    case APCKind::UncountedArray:
    case APCKind::StaticVec:
    case APCKind::UncountedVec:
    case APCKind::StaticDict:
    case APCKind::UncountedDict:
    case APCKind::StaticKeyset:
    case APCKind::UncountedKeyset:
      not_reached();

    case APCKind::FuncEntity:
      return APCNamedEntity::fromHandle(this)->getEntityOrNull();

    case APCKind::SharedString:
      return Variant::attach(
        StringData::MakeProxy(APCString::fromHandle(this))
      );
    case APCKind::SerializedArray: {
      auto const serArr = APCString::fromHandle(this)->getStringData();
      auto const v = apc_unserialize(serArr->data(), serArr->size());
      assertx(v.isPHPArray());
      return v;
    }
    case APCKind::SerializedVec: {
      auto const serVec = APCString::fromHandle(this)->getStringData();
      auto const v = apc_unserialize(serVec->data(), serVec->size());
      assertx(v.isVecArray());
      return v;
    }
    case APCKind::SerializedDict: {
      auto const serDict = APCString::fromHandle(this)->getStringData();
      auto const v = apc_unserialize(serDict->data(), serDict->size());
      assertx(v.isDict());
      return v;
    }
    case APCKind::SerializedKeyset: {
      auto const serKeyset = APCString::fromHandle(this)->getStringData();
      auto const v = apc_unserialize(serKeyset->data(), serKeyset->size());
      assertx(v.isKeyset());
      return v;
    }
    case APCKind::SharedArray:
    case APCKind::SharedPackedArray:
      return Variant::attach(
        APCLocalArray::Make(APCArray::fromHandle(this))->asArrayData()
      );
    case APCKind::SharedVArray:
      assertx(!RuntimeOption::EvalHackArrDVArrs);
      return Variant::attach(
        APCArray::fromHandle(this)->toLocalVArray()
      );
    case APCKind::SharedDArray:
      assertx(!RuntimeOption::EvalHackArrDVArrs);
      return Variant::attach(
        APCArray::fromHandle(this)->toLocalDArray()
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
  assertx(checkInvariants());
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
    case APCKind::PersistentFunc:
      delete APCTypedValue::fromHandle(this);
      return;

    case APCKind::FuncEntity:
      delete APCNamedEntity::fromHandle(this);
      return;

    case APCKind::SharedString:
    case APCKind::SerializedArray:
    case APCKind::SerializedVec:
    case APCKind::SerializedDict:
    case APCKind::SerializedKeyset:
    case APCKind::SerializedObject:
      APCString::Delete(APCString::fromHandle(this));
      return;

    case APCKind::SharedVArray:
    case APCKind::SharedDArray:
      assertx(!RuntimeOption::EvalHackArrDVArrs);
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
      assertx(false);
      return;
  }
  not_reached();
}

bool APCHandle::checkInvariants() const {
  switch (m_kind) {
    case APCKind::Uninit:
      assertx(m_type == KindOfUninit);
      return true;
    case APCKind::Null:
      assertx(m_type == KindOfNull);
      return true;
    case APCKind::Bool:
      assertx(m_type == KindOfBoolean);
      return true;
    case APCKind::Int:
      assertx(m_type == KindOfInt64);
      return true;
    case APCKind::Double:
      assertx(m_type == KindOfDouble);
      return true;
    case APCKind::PersistentFunc:
      assertx(m_type == KindOfFunc);
      return true;
    case APCKind::StaticString:
    case APCKind::UncountedString:
      assertx(m_type == KindOfPersistentString);
      return true;
    case APCKind::StaticArray:
    case APCKind::UncountedArray:
      assertx(m_type == KindOfPersistentArray);
      return true;
    case APCKind::StaticVec:
    case APCKind::UncountedVec:
      assertx(m_type == KindOfPersistentVec);
      return true;
    case APCKind::StaticDict:
    case APCKind::UncountedDict:
      assertx(m_type == KindOfPersistentDict);
      return true;
    case APCKind::StaticKeyset:
    case APCKind::UncountedKeyset:
      assertx(m_type == KindOfPersistentKeyset);
      return true;
    case APCKind::SharedVArray:
    case APCKind::SharedDArray:
      assertx(!RuntimeOption::EvalHackArrDVArrs);
    case APCKind::FuncEntity:
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
      assertx(m_type == kInvalidDataType);
      return true;
  }
  not_reached();
  return false;
}

//////////////////////////////////////////////////////////////////////

}
