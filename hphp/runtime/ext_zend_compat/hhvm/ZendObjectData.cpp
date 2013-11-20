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

c_ZendObjectData::c_ZendObjectData(Class* cls)
  : ObjectData(cls, ObjectData::IsCppBuiltin) {
  // TODO: #3235411 ZendObjectData fields collide with fields of Exception
  // Work around the problem by using ObjectData::o_subclassData for handle
  static_assert(sizeof(*this) == sizeof(ObjectData),
                "must be true, or else ZOD fields overlap PHP fields");
}

ObjectData* new_ZendObjectData_Instance(Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinObjSize = sizeof(c_ZendObjectData) - sizeof(ObjectData);
  size_t size = ObjectData::sizeForNProps(nProps) + builtinObjSize;
  auto obj = new (MM().objMallocLogged(size)) c_ZendObjectData(cls);

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
  obj->setHandle(ov.handle);

  if (UNLIKELY(cls->callsCustomInstanceInit())) {
    /*
      This must happen after the constructor finishes,
      because it can leak references to obj AND it can
      throw exceptions. If we have this in the ObjectData
      constructor, and it throws, obj will be partially
      destroyed (ie ~ObjectData will be called, resetting
      the vtable pointer) leaving dangling references
      to the object (eg in backtraces).
    */
    obj->callCustomInstanceInit();
  }

  return obj;
}

void delete_ZendObjectData(ObjectData* objData, const Class* cls) {
  auto const zobj = static_cast<c_ZendObjectData*>(objData);
  /*
   * TODO: ~c_ZendObjectData should probably run extension-provided
   * deallocation functions.
   */
  zobj->~c_ZendObjectData();

  auto const nProps = cls->numDeclProperties();
  auto prop = reinterpret_cast<TypedValue*>(zobj + 1);
  auto const stop = prop + nProps;
  for (; prop != stop; ++prop) {
    tvRefcountedDecRef(prop);
  }

  auto const builtinSz = sizeof(c_ZendObjectData) - sizeof(ObjectData);
  auto const size = ObjectData::sizeForNProps(nProps) + builtinSz;
  if (LIKELY(size <= kMaxSmartSize)) {
    return MM().smartFreeSizeLogged(zobj, size);
  }
  return MM().smartFreeSizeBigLogged(zobj, size);
}

}
