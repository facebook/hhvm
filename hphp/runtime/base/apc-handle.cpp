/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

APCHandle* APCHandle::Create(const Variant& source,
                             size_t& size,
                             bool serialized,
                             bool inner /* = false */,
                             bool unserializeObj /* = false */) {
  auto type = source.getType(); // this gets rid of the ref, if it was one
  switch (type) {
    case KindOfUninit:
    case KindOfNull: {
      auto value = new APCTypedValue(type);
      size = sizeof(APCTypedValue);
      return value->getHandle();
    }
    case KindOfBoolean: {
      auto value = new APCTypedValue(type,
          static_cast<int64_t>(source.getBoolean()));
      size = sizeof(APCTypedValue);
      return value->getHandle();
    }
    case KindOfInt64: {
      auto value = new APCTypedValue(type, source.getInt64());
      size = sizeof(APCTypedValue);
      return value->getHandle();
    }
    case KindOfDouble: {
      auto value = new APCTypedValue(type, source.getDouble());
      size = sizeof(APCTypedValue);
      return value->getHandle();
    }
    case KindOfStaticString: {
      if (serialized) goto StringCase;

      auto value = new APCTypedValue(type, source.getStringData());
      size = sizeof(APCTypedValue);
      return value->getHandle();
    }
StringCase:
    case KindOfString: {
      StringData* s = source.getStringData();
      if (serialized) {
        // It is priming, and there might not be the right class definitions
        // for unserialization.
        return APCObject::MakeShared(apc_reserialize(s), size);
      }

      auto const st = lookupStaticString(s);
      if (st) {
        APCTypedValue* value = new APCTypedValue(KindOfStaticString, st);
        size = sizeof(APCTypedValue);
        return value->getHandle();
      }

      assert(!s->isStatic()); // would've been handled above
      if (!inner && apcExtension::UseUncounted) {
        StringData* st = StringData::MakeUncounted(s->slice());
        APCTypedValue* value = new APCTypedValue(st);
        size = sizeof(APCTypedValue) + st->size();
        return value->getHandle();
      }
      return APCString::MakeShared(type, s, size);
    }

    case KindOfArray:
      return APCArray::MakeShared(source.getArrayData(),
                                  size,
                                  inner,
                                  unserializeObj);

    case KindOfObject:
      return unserializeObj ?
          APCObject::Construct(source.getObjectData(), size) :
          APCObject::MakeShared(apc_serialize(source), size);

    case KindOfResource:
      // TODO Task #2661075: Here and elsewhere in the runtime, we convert
      // Resources to the empty array during various serialization operations,
      // which does not match Zend behavior. We should fix this.
      size = sizeof(APCArray);
      return APCArray::MakeShared();

    case KindOfRef:
    case KindOfClass:
      return nullptr;
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
      return APCTypedValue::fromHandle(this)->getStringData();
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
    case KindOfInt64:
    case KindOfDouble:
    case KindOfStaticString:
      delete APCTypedValue::fromHandle(this);
      return;

    case KindOfString:
      delete APCString::fromHandle(this);
      return;

    case KindOfArray:
      APCArray::Delete(this);
      return;

    case KindOfObject:
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
