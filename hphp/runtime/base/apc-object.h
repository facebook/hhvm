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

#ifndef incl_HPHP_APC_OBJECT_H_
#define incl_HPHP_APC_OBJECT_H_

#include <cinttypes>

#include "hphp/util/either.h"
#include "hphp/runtime/base/apc-string.h"
#include "hphp/runtime/base/type-string.h"

namespace HPHP {

struct ObjectData;

//////////////////////////////////////////////////////////////////////

/*
 * Representation of an object stored in APC.
 *
 * It may also have a serialized form in which case it will be represented by
 * an APCString instance with type KindOfObject.
 *
 * MakeObject and Delete take care of isolating callers from that detail.
 */
struct APCObject {
  /*
   * Create an APCObject from an ObjectData*; returns its APCHandle.
   */
  static APCHandle::Pair Construct(ObjectData* data);

  // Return an APCObject instance from a serialized version of the
  // object.  May return null.
  static APCHandle::Pair MakeAPCObject(APCHandle* obj, const Variant& value);

  // Return an instance of a PHP object from the given object handle
  static Variant MakeObject(const APCHandle* handle);

  /*
   * Make a serialized version of an object.
   */
  static APCHandle::Pair MakeSerializedObj(String data) {
    auto const pair = APCString::MakeSharedString(KindOfObject, data.get());
    pair.handle->setSerializedObj();
    return pair;
  }

  // Delete the APC object holding the object data
  static void Delete(APCHandle* handle);

  static APCObject* fromHandle(APCHandle* handle) {
    assert(offsetof(APCObject, m_handle) == 0);
    return reinterpret_cast<APCObject*>(handle);
  }

  static const APCObject* fromHandle(const APCHandle* handle) {
    assert(offsetof(APCObject, m_handle) == 0);
    return reinterpret_cast<const APCObject*>(handle);
  }

  APCHandle* getHandle() { return &m_handle; }

private:
  friend struct APCHandle;
  friend size_t getMemSize(const APCObject*);

  struct Prop {
    StringData* name;
    APCHandle* val;
    Either<const Class*,const StringData*> ctx;
  };

private:
  explicit APCObject(ObjectData*, uint32_t propCount);
  ~APCObject();
  APCObject(const APCObject&) = delete;
  APCObject& operator=(const APCObject&) = delete;

private:
  Object createObject() const;

  Prop* props() { return reinterpret_cast<Prop*>(this + 1); }
  const Prop* props() const {
    return const_cast<APCObject*>(this)->props();
  }

private:
  APCHandle m_handle;
  Either<const Class*,const StringData*> m_cls;
  uint32_t m_propCount;
};

//////////////////////////////////////////////////////////////////////

}

#endif
