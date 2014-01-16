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

#ifndef incl_HPHP_ZEND_OBJECT_DATA
#define incl_HPHP_ZEND_OBJECT_DATA

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend_hphp_class_to_class_entry.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ObjectData;

FORWARD_DECLARE_CLASS(ZendObjectData);
class c_ZendObjectData : public ObjectData {
  DECLARE_CLASS_NO_SWEEP(ZendObjectData)
  public:
    c_ZendObjectData(Class* cls = classof());
    void setHandle(zend_object_handle handle) {
      always_assert(uint16_t(handle) == handle);
      o_subclassData.u16 = uint16_t(handle);
    }
    zend_object_handle getHandle() { return o_subclassData.u16; }
};

ObjectData* new_ZendObjectData_Instance(Class* cls);

}

#endif // incl_HPHP_ZEND_OBJECT_DATA
