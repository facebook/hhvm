/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext_zend_compat/hhvm/zend-object.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend-object-store.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend-class-entry.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_objects.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {

static StaticString s_ZendCompat("ZendCompat");

void ZendObject::registerNativeData() {
  static bool registered = false;
  if (!registered) {
    registered = true;
    Native::registerNativeDataInfo(
        s_ZendCompat.get(),
        sizeof(ZendObject),
        nativeDataCtor,
        nativeDataCopy,
        nativeDataDtor,
        nullptr /* sweep */,
        nullptr /* sleep */,
        nullptr /* wakeup */,
        nullptr /* scan */);
  }
}

void ZendObject::nativeDataCtor(ObjectData* obj) {
  Native::nativeDataInfoInit<ZendObject>(obj);
  Native::data<ZendObject>(obj)->initZendObject(obj->getVMClass());
}

/* Copy the native data. This is called on clone, so we treat it like how Zend
 * would treat a clone, following zend_objects_store_clone_obj(). A new
 * underlying object is created, with a new handle.
 */
void ZendObject::nativeDataCopy(ObjectData* dest, ObjectData* src) {
  ZendObject * zop_dest = Native::data<ZendObject>(dest);
  ZendObject * zop_src = Native::data<ZendObject>(src);
  TSRMLS_FETCH();

  // Call the object's clone handler, like what the PHP VM's
  // ZEND_CLONE handler does
  zend_object_clone_obj_t clone_call = zop_src->getHandlers()->clone_obj;
  if (clone_call == nullptr) {
    raise_error("Trying to clone uncloneable object of class %s",
        src->getVMClass()->name()->data());
  }
  TypedValue tv;
  tvWriteObject(src, &tv);
  auto ref = req::ptr<RefData>::attach(RefData::Make(tv));
  zend_object_value ov = clone_call(ref.get() TSRMLS_CC);

  zop_dest->setHandle(ov.handle);
  zop_dest->setHandlers(ov.handlers);
}

void ZendObject::nativeDataDtor(ObjectData* obj) {
  ZendObjectStore::getInstance().freeObject(
    Native::data<ZendObject>(obj)->getHandle());
  Native::nativeDataInfoDestroy<ZendObject>(obj);
}

void ZendObject::initZendObject(Class * cls) {
  TSRMLS_FETCH();
  zend_class_entry* ce = zend_hphp_class_to_class_entry(cls);
  auto create_func = ce->create_object;
  Class * current_class = cls;
  while (!create_func) {
    Class* parent = current_class->parent();
    if (!parent) {
      break;
    }
    zend_class_entry* parent_ce = zend_hphp_class_to_class_entry(parent);
    create_func = parent_ce->create_object;
    current_class = parent;
  }

  zend_object_value ov;
  if (create_func) {
    ov = create_func(ce TSRMLS_CC);
  } else {
    zend_object *object;
    ov = zend_objects_new(&object, ce TSRMLS_CC);
  }
  setHandle(ov.handle);
  setHandlers(ov.handlers);
}

/* Call the free_storage handler, invalidate the bucket and reuse its handle.
 * This is equivalent to Zend's zend_objects_store_del_ref_by_handle_ex()
 * in the case where the object is deleted.
 */
void ZendObject::destroyZendObject() {
  ZendObjectStore::getInstance().freeObject(m_handle);
}

}
