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

namespace {

ALWAYS_INLINE
Object createAndConstruct(Class* cls, const Variant& args) {
  Object inst{cls};
  TypedValue ret;
  g_context->invokeFunc(&ret,
                        cls->getCtor(),
                        args,
                        inst.get());
  tvRefcountedDecRef(&ret);
  return inst;
}

}

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

Object AllocStdClassObject() {
  return Object{s_stdclassClass};
}

Object AllocPinitSentinel() {
  return Object{s_pinitSentinelClass};
}

Object AllocExceptionObject(const Variant& message) {
  return createAndConstruct(s_ExceptionClass, make_packed_array(message));
}

Object AllocBadMethodCallExceptionObject(const Variant& message) {
  return createAndConstruct(s_BadMethodCallExceptionClass,
                            make_packed_array(message));
}

Object AllocInvalidArgumentExceptionObject(const Variant& message) {
  return createAndConstruct(s_InvalidArgumentExceptionClass,
                            make_packed_array(message));
}

Object AllocRuntimeExceptionObject(const Variant& message) {
  return createAndConstruct(s_RuntimeExceptionClass,
                            make_packed_array(message));
}

Object AllocOutOfBoundsExceptionObject(const Variant& message) {
  return createAndConstruct(s_OutOfBoundsExceptionClass,
                            make_packed_array(message));
}

Object AllocInvalidOperationExceptionObject(const Variant& message) {
  return createAndConstruct(s_InvalidOperationExceptionClass,
                            make_packed_array(message));
}

Object AllocDOMExceptionObject(const Variant& message,
                                    const Variant& code) {
  return createAndConstruct(s_DOMExceptionClass,
                            make_packed_array(message, code));
}

Object AllocSoapFaultObject(const Variant& code,
                                 const Variant& message,
                                 const Variant& actor /* = null_variant */,
                                 const Variant& detail /* = null_variant */,
                                 const Variant& name /* = null_variant */,
                                 const Variant& header /* = null_variant */) {
  return createAndConstruct(
    s_SoapFaultClass,
    make_packed_array(code,
                      message,
                      actor,
                      detail,
                      name,
                      header
                     )
  );
}

Object AllocLazyKVZipIterableObject(const Variant& mp) {
  return createAndConstruct(s_LazyKVZipIterableClass,
                            make_packed_array(mp));
}

Object AllocLazyIterableViewObject(const Variant& iterable) {
  return createAndConstruct(s_LazyIterableViewClass,
                            make_packed_array(iterable));
}

Object AllocLazyKeyedIterableViewObject(const Variant& iterable) {
  return createAndConstruct(s_LazyKeyedIterableViewClass,
                            make_packed_array(iterable));
}

void throwExceptionObject(const Variant& message) {
  throw AllocExceptionObject(message);
}

void throwBadMethodCallExceptionObject(const Variant& message) {
  throw AllocBadMethodCallExceptionObject(message);
}

void throwInvalidArgumentExceptionObject(const Variant& message) {
  throw AllocInvalidArgumentExceptionObject(message);
}

void throwRuntimeExceptionObject(const Variant& message) {
  throw AllocRuntimeExceptionObject(message);
}

void throwOutOfBoundsExceptionObject(const Variant& message) {
  throw AllocOutOfBoundsExceptionObject(message);
}

void throwInvalidOperationExceptionObject(const Variant& message) {
  throw AllocInvalidOperationExceptionObject(message);
}

void throwDOMExceptionObject(const Variant& message,
                             const Variant& code) {
  throw AllocDOMExceptionObject(message, code);
}

void throwSoapFaultObject(const Variant& code,
                          const Variant& message,
                          const Variant& actor /* = null_variant */,
                          const Variant& detail /* = null_variant */,
                          const Variant& name /* = null_variant */,
                          const Variant& header /* = null_variant */) {
  throw Object{AllocSoapFaultObject(code, message,
                                    actor, detail,
                                    name, header)};
}

#define ALLOC_OBJECT_STUB(name)                                         \
  Object Alloc##name##Object() {                                        \
    return Object{s_##name##Class};                                     \
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
