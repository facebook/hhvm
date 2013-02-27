/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <system/lib/systemlib.h>
#include <runtime/base/hphp_system.h>
#include <runtime/base/complex_types.h>
#include <runtime/vm/unit.h>
#include <runtime/vm/class.h>
#include <runtime/vm/instance.h>
#include <system/gen/php/classes/exception.h>
#include <system/gen/php/classes/stdclass.h>
#include <system/gen/php/classes/soapfault.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#ifdef HHVM
#define ALLOC_OBJECT_STUB_RETURN(name)                                    \
  HPHP::VM::Instance::newInstance(SystemLib::s_##name##Class)
#else
#define ALLOC_OBJECT_STUB_RETURN(name)                                    \
  NEWOBJ(c_##name)()
#endif
#define ALLOC_OBJECT_STUB(name)                                           \
  HPHP::VM::Class* SystemLib::s_##name##Class = nullptr;                     \
  ObjectData* SystemLib::Alloc##name##Object() {                          \
    return ALLOC_OBJECT_STUB_RETURN(name);                                \
  }

bool SystemLib::s_inited = false;
HPHP::Eval::PhpFile* SystemLib::s_phpFile = nullptr;
HPHP::VM::Unit* SystemLib::s_unit = nullptr;
HPHP::VM::Unit* SystemLib::s_nativeFuncUnit = nullptr;
HPHP::VM::Unit* SystemLib::s_nativeClassUnit = nullptr;
HPHP::VM::Class* SystemLib::s_stdclassClass = nullptr;
HPHP::VM::Class* SystemLib::s_ExceptionClass = nullptr;
HPHP::VM::Class* SystemLib::s_BadMethodCallExceptionClass = nullptr;
HPHP::VM::Class* SystemLib::s_InvalidArgumentExceptionClass = nullptr;
HPHP::VM::Class* SystemLib::s_RuntimeExceptionClass = nullptr;
HPHP::VM::Class* SystemLib::s_OutOfBoundsExceptionClass = nullptr;
HPHP::VM::Class* SystemLib::s_InvalidOperationExceptionClass = nullptr;
HPHP::VM::Class* SystemLib::s_pinitSentinelClass = nullptr;
HPHP::VM::Class* SystemLib::s_resourceClass = nullptr;
HPHP::VM::Class* SystemLib::s_DOMExceptionClass = nullptr;
HPHP::VM::Class* SystemLib::s_SplFileObjectClass = nullptr;
HPHP::VM::Class* SystemLib::s_SoapFaultClass = nullptr;
HPHP::VM::Class* SystemLib::s_ContinuationClass = nullptr;

ObjectData* SystemLib::AllocStdClassObject() {
  if (hhvm) {
    return HPHP::VM::Instance::newInstance(SystemLib::s_stdclassClass);
  } else {
    return NEWOBJ(c_stdClass)();
  }
}

ObjectData* SystemLib::AllocPinitSentinel() {
  const_assert(hhvm);
  return HPHP::VM::Instance::newInstance(SystemLib::s_pinitSentinelClass);
}

#define CREATE_AND_CONSTRUCT(clsname, params)                               \
  HPHP::VM::Instance* inst =                                                \
    HPHP::VM::Instance::newInstance(SystemLib::s_##clsname##Class);         \
  TypedValue ret;                                                           \
  {                                                                         \
    /* Increment refcount across call to ctor, so the object doesn't */     \
    /* get destroyed when ctor's frame is torn down */                      \
    CountableHelper cnt(inst);                                              \
    inst->invokeUserMethod(&ret,                                            \
                           SystemLib::s_##clsname##Class->getCtor(),        \
                           params);                                         \
  }                                                                         \
  tvRefcountedDecRef(&ret);                                                 \
  return inst;

ObjectData* SystemLib::AllocExceptionObject(CVarRef message) {
  if (hhvm) {
    CREATE_AND_CONSTRUCT(Exception, CREATE_VECTOR1(message));
  } else {
    return (NEWOBJ(c_Exception)())->create(message);
  }
}

ObjectData* SystemLib::AllocBadMethodCallExceptionObject(CVarRef message) {
  if (hhvm) {
    CREATE_AND_CONSTRUCT(BadMethodCallException, CREATE_VECTOR1(message));
  } else {
    return (NEWOBJ(c_BadMethodCallException)())->create(message);
  }
}

ObjectData* SystemLib::AllocInvalidArgumentExceptionObject(CVarRef message) {
  if (hhvm) {
    CREATE_AND_CONSTRUCT(InvalidArgumentException, CREATE_VECTOR1(message));
  } else {
    return (NEWOBJ(c_InvalidArgumentException)())->create(message);
  }
}

ObjectData* SystemLib::AllocRuntimeExceptionObject(CVarRef message) {
  if (hhvm) {
    CREATE_AND_CONSTRUCT(RuntimeException, CREATE_VECTOR1(message));
  } else {
    return (NEWOBJ(c_RuntimeException)())->create(message);
  }
}

ObjectData* SystemLib::AllocOutOfBoundsExceptionObject(CVarRef message) {
  if (hhvm) {
    CREATE_AND_CONSTRUCT(OutOfBoundsException, CREATE_VECTOR1(message));
  } else {
    return (NEWOBJ(c_OutOfBoundsException)())->create(message);
  }
}

ObjectData* SystemLib::AllocInvalidOperationExceptionObject(CVarRef message) {
  if (hhvm) {
    CREATE_AND_CONSTRUCT(InvalidOperationException, CREATE_VECTOR1(message));
  } else {
    return (NEWOBJ(c_InvalidOperationException)())->create(message);
  }
}

ObjectData* SystemLib::AllocDOMExceptionObject(CVarRef message, CVarRef code) {
  if (hhvm) {
    CREATE_AND_CONSTRUCT(DOMException, CREATE_VECTOR2(message, code));
  } else {
    return (NEWOBJ(c_DOMException)())->create(message, code);
  }
}

ObjectData* SystemLib::AllocSplFileObjectObject(CVarRef filename,
                                                CVarRef open_mode,
                                                CVarRef use_include_path,
                                                CVarRef context) {
  if (hhvm) {
    CREATE_AND_CONSTRUCT(SplFileObject,
      CREATE_VECTOR4(filename, open_mode, use_include_path, context));
  } else {
    return (NEWOBJ(c_SplFileObject)())->create(
      filename, open_mode, use_include_path, context);
  }
}

ObjectData*
SystemLib::AllocSoapFaultObject(CVarRef code,
                                CVarRef message,
                                CVarRef actor /* = null_variant */,
                                CVarRef detail /* = null_variant */,
                                CVarRef name /* = null_variant */,
                                CVarRef header /* = null_variant */) {
  if (hhvm) {
    CREATE_AND_CONSTRUCT(SoapFault, CREATE_VECTOR6(code, message, actor,
                                                   detail, name, header));
  } else {
    return (NEWOBJ(c_SoapFault)())->create(code, message, actor, detail, name,
                                           header);
  }
}

#undef CREATE_AND_CONSTRUCT

VM::Func*
SystemLib::GetNullFunction() {
  VM::Func* f = s_nativeFuncUnit->firstHoistable();
  assert(!strcmp(f->name()->data(), "86null"));
  return f;
}

ALLOC_OBJECT_STUB(Directory);
ALLOC_OBJECT_STUB(RecursiveDirectoryIterator);
ALLOC_OBJECT_STUB(SplFileInfo);
ALLOC_OBJECT_STUB(PDOException);

///////////////////////////////////////////////////////////////////////////////
}

