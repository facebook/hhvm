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

#ifndef incl_HPHP_ZEND_OBJECT_H_
#define incl_HPHP_ZEND_OBJECT_H_

#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_types.h"
#include "hphp/runtime/base/object-data.h"

namespace HPHP {

struct ZendObject {
  static void registerNativeData();

  void setHandle(unsigned int handle) {
    m_handle = handle;
  }

  zend_object_handle getHandle() const {
    return m_handle;
  }

  void setHandlers(const zend_object_handlers * handlers) {
    m_handlers = handlers;
  }

  const zend_object_handlers * getHandlers() const {
    return m_handlers;
  }

protected:
  static void nativeDataCtor(ObjectData* obj);
  static void nativeDataCopy(ObjectData* dest, ObjectData* src);
  static void nativeDataDtor(ObjectData* obj);

  void initZendObject(Class* cls);
  void destroyZendObject();

  zend_object_handle m_handle;
  // Note: zend_object_handlers will be an opaque (incomplete) type for
  // callers external to EZC
  const zend_object_handlers * m_handlers;
};

}
#endif
