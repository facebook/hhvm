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

#ifndef incl_HPHP_ZEND_OBJECT

#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_types.h"
#include "hphp/runtime/base/object-data.h"

namespace HPHP {

class ZendObject {
  public:
    static void registerNativeData();

    void setHandle(zend_object_handle handle) {
      m_handle = handle;
    }

    zend_object_handle getHandle() const {
      return m_handle;
    }

  protected:
    static void nativeDataCtor(ObjectData* obj);
    void initZendObject(Class* cls);

    int64_t m_handle;
};

}
#endif
