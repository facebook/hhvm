/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/apc-string.h"
#include <cinttypes>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Representation of an object stored in APC.
 * It may also have a serialized form in which case it will be represented by
 * an APCString instance with type KindOfObject.
 * MakeObject and Delete take care of isolating callers from that detail.
 */
struct APCObject {
  //
  // APC object creation
  //
  static APCHandle* MakeShared(String data) {
    APCHandle* handle = APCString::MakeShared(KindOfObject, data.get());
    handle->mustCache();
    return handle;
  }
  static APCHandle* MakeShared(ObjectData* data) {
    APCObject* apcObj = new APCObject(data);
    return apcObj->getHandle();
  }

  // Return an APCObject instance from a serialized version of the object.
  // May return null
  static APCHandle* MakeAPCObject(APCHandle* obj, CVarRef value);

  // Return an instance of a PHP object from the given object handle
  static Variant MakeObject(APCHandle* handle);

  // Delete the APC object holding the object data
  static void Delete(APCHandle* handle);

  static APCObject* fromHandle(APCHandle* handle) {
    assert(offsetof(APCObject, m_handle) == 0);
    return reinterpret_cast<APCObject*>(handle);
  }

  APCHandle* getHandle() {
    return &m_handle;
  }

  //
  // Stats API
  //
  void getSizeStats(APCHandleStats* stats) const;
  int32_t getSpaceUsage() const;

private:
  explicit APCObject(ObjectData* obj);
  ~APCObject();

  APCObject(const APCObject&) = delete;
  APCObject& operator=(const APCObject&) = delete;

  Object createObject() const;

  friend struct APCHandle;

private:
  struct Prop {
    StringData* name;
    APCHandle* val;
  };

  APCHandle m_handle;
  Prop* m_props;
  int m_propCount;
  StringData* const m_cls;  // static string
};

//////////////////////////////////////////////////////////////////////

}

#endif
