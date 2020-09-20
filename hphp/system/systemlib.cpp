/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/rx.h"

#include <vector>

namespace HPHP { namespace SystemLib {
/////////////////////////////////////////////////////////////////////////////

namespace {

const StaticString s_message("message");
const StaticString s_code("code");
const Slot s_messageIdx{0};
const Slot s_codeIdx{2};

DEBUG_ONLY bool throwable_has_expected_props() {
  auto const erCls = s_ErrorClass;
  auto const exCls = s_ExceptionClass;
  if (erCls->lookupDeclProp(s_message.get()) != s_messageIdx ||
      exCls->lookupDeclProp(s_message.get()) != s_messageIdx ||
      erCls->lookupDeclProp(s_code.get()) != s_codeIdx ||
      exCls->lookupDeclProp(s_code.get()) != s_codeIdx) {
    return false;
  }
  // Check that we have the expected type-hints on these props so we don't need
  // to verify anything.
  return
    erCls->declPropTypeConstraint(s_messageIdx).isString() &&
    exCls->declPropTypeConstraint(s_messageIdx).isString() &&
    erCls->declPropTypeConstraint(s_codeIdx).isInt() &&
    exCls->declPropTypeConstraint(s_codeIdx).isInt();
}

ALWAYS_INLINE
Object createAndConstruct(Class* cls, const Array& args) {
  Object inst{cls};
  tvDecRefGen(g_context->invokeFunc(cls->getCtor(), args, inst.get()));
  return inst;
}

/**
 * Fast path for Errors and Exceptions that do not override the default
 * constructor or message property initializer. Does not reenter VM.
 */
ALWAYS_INLINE
Object createAndConstructThrowable(Class* cls, const Variant& message) {
  assertx(throwable_has_expected_props());
  assertx(cls->getCtor() == s_ErrorClass->getCtor() ||
          cls->getCtor() == s_ExceptionClass->getCtor());

  Object inst{cls};
  if (debug) {
    DEBUG_ONLY auto const code_prop = inst->propRvalAtOffset(s_codeIdx);
    assertx(isIntType(code_prop.type()));
    assertx(code_prop.val().num == 0);
  }
  auto const message_prop = inst->propLvalAtOffset(s_messageIdx);
  assertx(isStringType(message_prop.type()));
  assertx(message_prop.val().pstr == staticEmptyString());
  tvDup(*message.asTypedValue(), message_prop);
  return inst;
}

}

bool s_inited = false;
bool s_anyNonPersistentBuiltins = false;
std::string s_source;
Unit* s_unit = nullptr;
Unit* s_hhas_unit = nullptr;
Func* s_nullFunc = nullptr;
Func* s_singleArgNullFunc = nullptr;
Func* s_nullCtor = nullptr;

/////////////////////////////////////////////////////////////////////////////

#define DEFINE_SYSTEMLIB_CLASS(cls)       \
  Class* s_ ## cls ## Class = nullptr;
SYSTEMLIB_CLASSES(DEFINE_SYSTEMLIB_CLASS)
#undef DEFINE_SYSTEMLIB_CLASS

#define DEFINE_SYSTEMLIB_HH_CLASS(cls)       \
  Class* s_HH_ ## cls ## Class = nullptr;
SYSTEMLIB_HH_CLASSES(DEFINE_SYSTEMLIB_HH_CLASS)
#undef DEFINE_SYSTEMLIB_HH_CLASS

Class* s_ThrowableClass;
Class* s_BaseExceptionClass;
Class* s_ErrorClass;
Class* s_ArithmeticErrorClass;
Class* s_ArgumentCountErrorClass;
Class* s_AssertionErrorClass;
Class* s_DivisionByZeroErrorClass;
Class* s_ParseErrorClass;
Class* s_TypeErrorClass;

Object AllocStdClassObject() {
  return Object{s_stdclassClass};
}

Object AllocPinitSentinel() {
  return Object{s_pinitSentinelClass};
}

Object AllocExceptionObject(const Variant& message) {
  return createAndConstructThrowable(s_ExceptionClass, message);
}

Object AllocErrorObject(const Variant& message) {
  return createAndConstructThrowable(s_ErrorClass, message);
}

Object AllocArithmeticErrorObject(const Variant& message) {
  return createAndConstructThrowable(s_ArithmeticErrorClass, message);
}

Object AllocArgumentCountErrorObject(const Variant& message) {
  return createAndConstructThrowable(s_ArgumentCountErrorClass, message);
}

Object AllocDivisionByZeroErrorObject(const Variant& message) {
  return createAndConstructThrowable(s_DivisionByZeroErrorClass, message);
}

Object AllocParseErrorObject(const Variant& message) {
  return createAndConstructThrowable(s_ParseErrorClass, message);
}

Object AllocTypeErrorObject(const Variant& message) {
  return createAndConstructThrowable(s_TypeErrorClass, message);
}

Object AllocBadMethodCallExceptionObject(const Variant& message) {
  return createAndConstructThrowable(s_BadMethodCallExceptionClass, message);
}

Object AllocInvalidArgumentExceptionObject(const Variant& message) {
  return createAndConstructThrowable(s_InvalidArgumentExceptionClass, message);
}

Object AllocTypeAssertionExceptionObject(const Variant& message) {
  return createAndConstructThrowable(s_TypeAssertionExceptionClass, message);
}

Object AllocRuntimeExceptionObject(const Variant& message) {
  return createAndConstructThrowable(s_RuntimeExceptionClass, message);
}

Object AllocOutOfBoundsExceptionObject(const Variant& message) {
  return createAndConstructThrowable(s_OutOfBoundsExceptionClass, message);
}

Object AllocInvalidOperationExceptionObject(const Variant& message) {
  return createAndConstructThrowable(s_InvalidOperationExceptionClass, message);
}

Object AllocDOMExceptionObject(const Variant& message) {
  return createAndConstructThrowable(s_DOMExceptionClass, message);
}

Object AllocDivisionByZeroExceptionObject() {
  return createAndConstructThrowable(s_DivisionByZeroExceptionClass,
                                     Strings::DIVISION_BY_ZERO);
}

Object AllocSoapFaultObject(const Variant& code,
                                 const Variant& message,
                                 const Variant& actor /* = uninit_variant */,
                                 const Variant& detail /* = uninit_variant */,
                                 const Variant& name /* = uninit_variant */,
                                 const Variant& header /* = uninit_variant */) {
  return createAndConstruct(
    s_SoapFaultClass,
    make_varray(code, message, actor, detail, name, header)
  );
}

Object AllocLazyKVZipIterableObject(const Variant& mp) {
  return createAndConstruct(s_LazyKVZipIterableClass,
                            make_varray(mp));
}

Object AllocLazyIterableViewObject(const Variant& iterable) {
  return createAndConstruct(s_LazyIterableViewClass,
                            make_varray(iterable));
}

Object AllocLazyKeyedIterableViewObject(const Variant& iterable) {
  return createAndConstruct(s_LazyKeyedIterableViewClass,
                            make_varray(iterable));
}

void throwExceptionObject(const Variant& message) {
  throw_object(AllocExceptionObject(message));
}

void throwErrorObject(const Variant& message) {
  throw_object(AllocErrorObject(message));
}

void throwArithmeticErrorObject(const Variant& message) {
  throw_object(AllocArithmeticErrorObject(message));
}

void throwArgumentCountErrorObject(const Variant& message) {
  throw_object(AllocArgumentCountErrorObject(message));
}

void throwDivisionByZeroErrorObject(const Variant& message) {
  throw_object(AllocDivisionByZeroErrorObject(message));
}

void throwParseErrorObject(const Variant& message) {
  throw_object(AllocParseErrorObject(message));
}

void throwTypeErrorObject(const Variant& message) {
  throw_object(AllocTypeErrorObject(message));
}

void throwBadMethodCallExceptionObject(const Variant& message) {
  throw_object(AllocBadMethodCallExceptionObject(message));
}

void throwInvalidArgumentExceptionObject(const Variant& message) {
  throw_object(AllocInvalidArgumentExceptionObject(message));
}

void throwTypeAssertionExceptionObject(const Variant& message) {
  throw_object(AllocTypeAssertionExceptionObject(message));
}

void throwRuntimeExceptionObject(const Variant& message) {
  throw_object(AllocRuntimeExceptionObject(message));
}

void throwOutOfBoundsExceptionObject(const Variant& message) {
  throw_object(AllocOutOfBoundsExceptionObject(message));
}

void throwInvalidOperationExceptionObject(const Variant& message) {
  throw_object(AllocInvalidOperationExceptionObject(message));
}

void throwDOMExceptionObject(const Variant& message) {
  throw_object(AllocDOMExceptionObject(message));
}

void throwDivisionByZeroExceptionObject() {
  throw_object(AllocDivisionByZeroExceptionObject());
}

void throwSoapFaultObject(const Variant& code,
                          const Variant& message,
                          const Variant& actor /* = uninit_variant */,
                          const Variant& detail /* = uninit_variant */,
                          const Variant& name /* = uninit_variant */,
                          const Variant& header /* = uninit_variant */) {
  throw_object(Object{AllocSoapFaultObject(code, message,
                                    actor, detail,
                                    name, header)});
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

namespace {

Func* setupNullClsMethod(Func* f, Class* cls, StringData* name) {
  assertx(f && f->isPhpLeafFn());
  auto clone = f->clone(cls, name);
  clone->setNewFuncId();
  clone->setAttrs(static_cast<Attr>(
                    AttrPublic | AttrNoInjection | AttrDynamicallyCallable) |
                    rxMakeAttr(RxLevel::Rx, false));
  return clone;
}

Func* setup86ctorMethod(Class* cls) {
  if (!s_nullFunc) {
    s_nullFunc = Func::lookup(makeStaticString("__SystemLib\\__86null"));
  }
  return setupNullClsMethod(s_nullFunc, cls, s_86ctor.get());
}

Func* setup86ReifiedInitMethod(Class* cls) {
  if (!s_singleArgNullFunc) {
    s_singleArgNullFunc =
      Func::lookup(makeStaticString("__SystemLib\\__86single_arg_null"));
  }
  return setupNullClsMethod(s_singleArgNullFunc, cls, s_86reifiedinit.get());
}

} // namespace

void setupNullCtor(Class* cls) {
  assertx(!s_nullCtor);
  s_nullCtor = setup86ctorMethod(cls);
  s_nullCtor->setHasForeignThis(true);
}

Func* getNull86reifiedinit(Class* cls) {
  auto f = setup86ReifiedInitMethod(cls);
  f->setBaseCls(cls);
  f->setGenerated(true);
  return f;
}

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::SystemLib
