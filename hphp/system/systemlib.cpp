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

#include "hphp/system/systemlib.h"
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/class.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define ALLOC_OBJECT_STUB_RETURN(name)                                    \
  ObjectData::newInstance(SystemLib::s_##name##Class)

#define ALLOC_OBJECT_STUB(name)                                           \
  ObjectData* SystemLib::Alloc##name##Object() {                          \
    return ALLOC_OBJECT_STUB_RETURN(name);                                \
  }

bool SystemLib::s_inited = false;
string SystemLib::s_source = "";
HPHP::Unit* SystemLib::s_unit = nullptr;
HPHP::Unit* SystemLib::s_hhas_unit = nullptr;
HPHP::Unit* SystemLib::s_nativeFuncUnit = nullptr;
HPHP::Unit* SystemLib::s_nativeClassUnit = nullptr;
HPHP::Func* SystemLib::s_nullFunc = nullptr;
HPHP::Func* SystemLib::s_continuationSendFunc = nullptr;
HPHP::Func* SystemLib::s_continuationRaiseFunc = nullptr;

#define DEFINE_SYSTEMLIB_CLASS(cls)       \
  HPHP::Class* SystemLib::s_ ## cls ## Class = nullptr;
SYSTEMLIB_CLASSES(DEFINE_SYSTEMLIB_CLASS)
#undef DEFINE_SYSTEMLIB_CLASS

ObjectData* SystemLib::AllocStdClassObject() {
  return ObjectData::newInstance(SystemLib::s_stdclassClass);
}

ObjectData* SystemLib::AllocPinitSentinel() {
  return ObjectData::newInstance(SystemLib::s_pinitSentinelClass);
}

#define CREATE_AND_CONSTRUCT(clsname, params)                               \
  ObjectData* inst =                                                        \
    ObjectData::newInstance(SystemLib::s_##clsname##Class);                 \
  TypedValue ret;                                                           \
  {                                                                         \
    /* Increment refcount across call to ctor, so the object doesn't */     \
    /* get destroyed when ctor's frame is torn down */                      \
    CountableHelper cnt(inst);                                              \
    g_vmContext->invokeFunc(&ret,                                           \
                            SystemLib::s_##clsname##Class->getCtor(),       \
                            params,                                         \
                            inst);                                          \
  }                                                                         \
  tvRefcountedDecRef(&ret);                                                 \
  return inst;

ObjectData* SystemLib::AllocExceptionObject(CVarRef message) {
  CREATE_AND_CONSTRUCT(Exception, make_packed_array(message));
}

ObjectData* SystemLib::AllocBadMethodCallExceptionObject(CVarRef message) {
  CREATE_AND_CONSTRUCT(BadMethodCallException, make_packed_array(message));
}

ObjectData* SystemLib::AllocInvalidArgumentExceptionObject(CVarRef message) {
  CREATE_AND_CONSTRUCT(InvalidArgumentException, make_packed_array(message));
}

ObjectData* SystemLib::AllocRuntimeExceptionObject(CVarRef message) {
  CREATE_AND_CONSTRUCT(RuntimeException, make_packed_array(message));
}

ObjectData* SystemLib::AllocOutOfBoundsExceptionObject(CVarRef message) {
  CREATE_AND_CONSTRUCT(OutOfBoundsException, make_packed_array(message));
}

ObjectData* SystemLib::AllocInvalidOperationExceptionObject(CVarRef message) {
  CREATE_AND_CONSTRUCT(InvalidOperationException, make_packed_array(message));
}

ObjectData* SystemLib::AllocDOMDocumentObject(const String& version,
                                              const String& encoding) {
  CREATE_AND_CONSTRUCT(DOMDocument, make_packed_array(version, encoding));
}

ObjectData* SystemLib::AllocDOMExceptionObject(CVarRef message, CVarRef code) {
  CREATE_AND_CONSTRUCT(DOMException, make_packed_array(message, code));
}

ObjectData* SystemLib::AllocReflectionExceptionObject(CVarRef message) {
  CREATE_AND_CONSTRUCT(ReflectionException, make_packed_array(message));
}

ObjectData*
SystemLib::AllocSoapFaultObject(CVarRef code,
                                CVarRef message,
                                CVarRef actor /* = null_variant */,
                                CVarRef detail /* = null_variant */,
                                CVarRef name /* = null_variant */,
                                CVarRef header /* = null_variant */) {
  CREATE_AND_CONSTRUCT(SoapFault, make_packed_array(code, message, actor,
                                                 detail, name, header));
}

ObjectData* SystemLib::AllocLazyKVZipIterableObject(CVarRef mp) {
  CREATE_AND_CONSTRUCT(LazyKVZipIterable, make_packed_array(mp));
}

ObjectData* SystemLib::AllocLazyIterableViewObject(CVarRef iterable) {
  CREATE_AND_CONSTRUCT(LazyIterableView, make_packed_array(iterable));
}

ObjectData* SystemLib::AllocLazyKeyedIterableViewObject(CVarRef iterable) {
  CREATE_AND_CONSTRUCT(LazyKeyedIterableView, make_packed_array(iterable));
}

#undef CREATE_AND_CONSTRUCT

ALLOC_OBJECT_STUB(Directory);
ALLOC_OBJECT_STUB(PDOException);

///////////////////////////////////////////////////////////////////////////////
}

