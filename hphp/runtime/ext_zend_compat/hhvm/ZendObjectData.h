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

/* Tons of declarations so we can make call create_object. Including those
 * headers will pull in the world, and this file needs to be included from
 * infotab */
typedef unsigned int zend_object_handle;
typedef unsigned int zend_uint;
struct _zend_object_handlers;
typedef struct _zend_object_handlers zend_object_handlers;
typedef struct _zend_object_value {
  zend_object_handle handle;
  const zend_object_handlers *handlers;
} zend_object_value;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ObjectData;

FORWARD_DECLARE_CLASS_BUILTIN(ZendObjectData);
class c_ZendObjectData : public ObjectData {
  DECLARE_CLASS(ZendObjectData, ZendObjectData, ObjectData)
  public:
    c_ZendObjectData(Class* cls = c_ZendObjectData::s_cls);
    void setHandle(zend_object_handle handle) { m_handle = handle; }
    zend_object_handle getHandle() { return m_handle; }
  private:
    zend_object_handle m_handle;
};

ObjectData* new_ZendObjectData_Instance(Class* cls);

}

#endif // incl_HPHP_ZEND_OBJECT_DATA
