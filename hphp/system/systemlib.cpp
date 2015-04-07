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

#include "hphp/system/systemlib.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/base/execution-context.h"

#include <vector>

namespace HPHP { namespace SystemLib {
/////////////////////////////////////////////////////////////////////////////

bool s_inited = false;
bool s_anyNonPersistentBuiltins = false;
std::string s_source;
Unit* s_unit = nullptr;
Unit* s_hhas_unit = nullptr;
Unit* s_nativeFuncUnit = nullptr;
Unit* s_nativeClassUnit = nullptr;
Func* s_nullFunc = nullptr;

/////////////////////////////////////////////////////////////////////////////

#define DEFINE_SYSTEMLIB_CLASS(cls)       \
  Class* s_ ## cls ## Class = nullptr;
SYSTEMLIB_CLASSES(DEFINE_SYSTEMLIB_CLASS)
#undef DEFINE_SYSTEMLIB_CLASS

ObjectData* AllocStdClassObject() {
  return ObjectData::newInstance(s_stdclassClass);
}

ObjectData* AllocPinitSentinel() {
  return ObjectData::newInstance(s_pinitSentinelClass);
}

#define CREATE_AND_CONSTRUCT(clsname, params)                               \
  ObjectData* inst =                                                        \
    ObjectData::newInstance(s_##clsname##Class);                            \
  TypedValue ret;                                                           \
  {                                                                         \
    /* Increment refcount across call to ctor, so the object doesn't */     \
    /* get destroyed when ctor's frame is torn down */                      \
    CountableHelper cnt(inst);                                              \
    g_context->invokeFunc(&ret,                                             \
                          s_##clsname##Class->getCtor(),                    \
                          params,                                           \
                          inst);                                            \
  }                                                                         \
  tvRefcountedDecRef(&ret);                                                 \
  return inst;

ObjectData* AllocExceptionObject(const Variant& message) {
  CREATE_AND_CONSTRUCT(Exception, make_packed_array(message));
}

ObjectData* AllocBadMethodCallExceptionObject(const Variant& message) {
  CREATE_AND_CONSTRUCT(BadMethodCallException, make_packed_array(message));
}

ObjectData* AllocInvalidArgumentExceptionObject(const Variant& message) {
  CREATE_AND_CONSTRUCT(InvalidArgumentException, make_packed_array(message));
}

ObjectData* AllocRuntimeExceptionObject(const Variant& message) {
  CREATE_AND_CONSTRUCT(RuntimeException, make_packed_array(message));
}

ObjectData* AllocOutOfBoundsExceptionObject(const Variant& message) {
  CREATE_AND_CONSTRUCT(OutOfBoundsException, make_packed_array(message));
}

ObjectData* AllocInvalidOperationExceptionObject(const Variant& message) {
  CREATE_AND_CONSTRUCT(InvalidOperationException, make_packed_array(message));
}

ObjectData* AllocDOMExceptionObject(const Variant& message,
                                    const Variant& code) {
  CREATE_AND_CONSTRUCT(DOMException, make_packed_array(message, code));
}

ObjectData* AllocSoapFaultObject(const Variant& code,
                                 const Variant& message,
                                 const Variant& actor /* = null_variant */,
                                 const Variant& detail /* = null_variant */,
                                 const Variant& name /* = null_variant */,
                                 const Variant& header /* = null_variant */) {
  CREATE_AND_CONSTRUCT(SoapFault, make_packed_array(code, message, actor,
                                                    detail, name, header));
}

ObjectData* AllocLazyKVZipIterableObject(const Variant& mp) {
  CREATE_AND_CONSTRUCT(LazyKVZipIterable, make_packed_array(mp));
}

ObjectData* AllocLazyIterableViewObject(const Variant& iterable) {
  CREATE_AND_CONSTRUCT(LazyIterableView, make_packed_array(iterable));
}

ObjectData* AllocLazyKeyedIterableViewObject(const Variant& iterable) {
  CREATE_AND_CONSTRUCT(LazyKeyedIterableView, make_packed_array(iterable));
}

#undef CREATE_AND_CONSTRUCT

#define ALLOC_OBJECT_STUB(name)                                           \
  ObjectData* Alloc##name##Object() {                                     \
    return ObjectData::newInstance(s_##name##Class);                      \
  }

ALLOC_OBJECT_STUB(Directory);
ALLOC_OBJECT_STUB(PDOException);

#undef ALLOC_OBJECT_STUB

/////////////////////////////////////////////////////////////////////////////

static std::vector<Unit*> s_persistent_units;

/* To be called during process startup ONLY, before threads are spun up.
 * Typically this will be called by HPHP::Extension::moduleInit to load an
 * extension-specific systemlib file, or to load the main systemlib.
 */
void addPersistentUnit(Unit* unit) {
  s_persistent_units.push_back(unit);
}

/* Typically called between requests in non-RepoAuthoritative mode
 * when function renaming is enabled.
 */
void mergePersistentUnits() {
  for (auto unit : s_persistent_units) {
    unit->merge();
  }
}

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::SystemLib
