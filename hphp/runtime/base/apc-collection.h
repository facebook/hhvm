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
#ifndef incl_HPHP_APC_COLLECTION_H_
#define incl_HPHP_APC_COLLECTION_H_

#include "hphp/runtime/base/apc-handle.h"

namespace HPHP {

struct ObjectData;
struct Object;

//////////////////////////////////////////////////////////////////////

struct APCCollection {
  static APCHandle::Pair Make(const ObjectData*,
                              APCHandleLevel level,
                              bool unserializeObj);
  static void Delete(APCHandle*);

  static const APCCollection* fromHandle(const APCHandle* handle) {
    assert(handle->checkInvariants());
    assert(handle->kind() == APCKind::SharedCollection);
    static_assert(offsetof(APCCollection, m_handle) == 0, "");
    return reinterpret_cast<const APCCollection*>(handle);
  }

  Object createObject() const;

private:
  APCCollection();
  ~APCCollection();
  static APCHandle::Pair WrapArray(APCHandle::Pair, CollectionType);

private:
  APCHandle m_handle;
  APCHandle* m_arrayHandle;
  CollectionType m_colType;
};

//////////////////////////////////////////////////////////////////////

}

#endif
