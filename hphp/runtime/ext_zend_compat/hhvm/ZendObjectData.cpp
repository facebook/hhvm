/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext_zend_compat/hhvm/ZendObjectData.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_objects.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

c_ZendObjectData::c_ZendObjectData(Class* cls) : ObjectData(cls) {}

ObjectData* new_ZendObjectData_Instance(Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_ZendObjectData) - sizeof(ObjectData);
  size_t size = ObjectData::sizeForNProps(nProps) + builtinPropSize;
  auto inst = new (MM().objMalloc(size)) c_ZendObjectData(cls);

  zend_class_entry* ce = zend_hphp_class_to_class_entry(cls);
  auto create_func = ce->create_object;
  while (!create_func) {
    Class* parent = cls->parent();
    if (!parent) {
      break;
    }
    zend_class_entry* parent_ce = zend_hphp_class_to_class_entry(parent);
    create_func = parent_ce->create_object;
  }

  zend_object_value ov;
  if (create_func) {
    ov = create_func(ce);
  } else {
    zend_object *object;
    ov = zend_objects_new(&object, ce);
  }
  inst->setHandle(ov.handle);

  return inst;
}

}
