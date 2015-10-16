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
#include "hphp/runtime/base/apc-handle.h"

#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/apc-string.h"
#include "hphp/runtime/base/apc-array.h"
#include "hphp/runtime/base/apc-object.h"
#include "hphp/runtime/base/apc-collection.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

APCHandle::Pair APCHandle::Create(const Variant& source,
                                  bool serialized,
                                  bool inner,
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
    case KindOfStaticString: {
      StringData* s = source.getStringData();
      if (serialized) {
        // It is priming, and there might not be the right class definitions
        // for unserialization.
        return APCObject::MakeSerializedObj(apc_reserialize(String{s}));
      }
      auto value = new APCTypedValue(APCTypedValue::StaticStr{}, s);
      return {value->getHandle(), sizeof(APCTypedValue)};
    }
    case KindOfString: {
      StringData* s = source.getStringData();
      if (serialized) {
        // It is priming, and there might not be the right class definitions
        // for unserialization.
        return APCObject::MakeSerializedObj(apc_reserialize(String{s}));
      }

      auto const st = lookupStaticString(s);
      if (st) {
        auto value = new APCTypedValue(APCTypedValue::StaticStr{}, st);
        return {value->getHandle(), sizeof(APCTypedValue)};
      }

      assert(!s->isStatic()); // would've been handled above
      if (!inner && apcExtension::UseUncounted) {
        auto st = StringData::MakeUncounted(s->slice());
        auto value = new APCTypedValue(APCTypedValue::UncountedStr{}, st);
        return {value->getHandle(), st->size() + sizeof(APCTypedValue)};
      }
      return APCString::MakeSharedString(type, s);
    }

    case KindOfArray:
      return APCArray::MakeSharedArray(source.getArrayData(), inner,
                                       unserializeObj);

    case KindOfObject:
      if (source.getObjectData()->isCollection()) {
        return APCCollection::Make(source.getObjectData(),
                                   inner,
                                   unserializeObj);
      }
      return unserializeObj ? APCObject::Construct(source.getObjectData()) :
             APCObject::MakeSerializedObj(apc_serialize(source));

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
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:
      return init_null(); // shortcut.. no point to forward
    case KindOfBoolean:
      return APCTypedValue::fromHandle(this)->getBoolean();
    case KindOfInt64:
      return APCTypedValue::fromHandle(this)->getInt64();
    case KindOfDouble:
      return APCTypedValue::fromHandle(this)->getDouble();
    case KindOfStaticString:
      return Variant{APCTypedValue::fromHandle(this)->getStringData()};
    case KindOfString:
      return APCString::MakeString(this);
    case KindOfArray:
      return APCArray::MakeArray(this);
    case KindOfObject:
      return APCObject::MakeObject(this);
    case KindOfResource:
    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

void APCHandle::deleteShared() {
  assert(!isUncounted());
  switch (m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
      return;
    case KindOfInt64:
    case KindOfDouble:
    case KindOfStaticString:
      delete APCTypedValue::fromHandle(this);
      return;

    case KindOfString:
      APCString::Delete(APCString::fromHandle(this));
      return;

    case KindOfArray:
      APCArray::Delete(this);
      return;

    case KindOfObject:
      if (isAPCCollection()) {
        APCCollection::Delete(this);
        return;
      }
      APCObject::Delete(this);
      return;

    case KindOfResource:
    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}
