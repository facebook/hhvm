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
#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/types.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-emitter.h"

#include <memory>
#include <vector>

namespace HPHP::SystemLib {
/////////////////////////////////////////////////////////////////////////////

namespace {

const StaticString s_message("message");
const StaticString s_code("code");
const Slot s_messageIdx{0};
const Slot s_codeIdx{2};

DEBUG_ONLY bool throwable_has_expected_props() {
  auto const erCls = getErrorClass();
  auto const exCls = getExceptionClass();
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
  assertx(cls->getCtor() == getErrorClass()->getCtor() ||
          cls->getCtor() == getExceptionClass()->getCtor());

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

bool s_anyNonPersistentBuiltins = false;
std::string s_source;
Unit* s_unit = nullptr;
Func* s_nullFunc = nullptr;
Func* s_singleArgNullFunc = nullptr;
Func* s_nullCtor = nullptr;

/////////////////////////////////////////////////////////////////////////////

#define DEFINE_SYSTEMLIB_CLASS(cls, prefix, namespace) \
  Class* s_ ## prefix ## cls ## Class = nullptr;       \
  const StaticString s_##prefix##cls(#namespace #cls); \
  Class* get ## prefix ## cls ## Class() {             \
    return classLoad(s_ ## prefix ## cls.get(), s_ ## prefix ## cls ## Class); \
  }
SYSTEMLIB_CLASSES(DEFINE_SYSTEMLIB_CLASS)
#undef DEFINE_SYSTEMLIB_CLASS

Object AllocStdClassObject() {
  return Object{getstdClassClass()};
}

Object AllocPinitSentinel() {
  return Object{getpinitSentinelClass()};
}

Object AllocExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getExceptionClass(), message);
}

Object AllocErrorObject(const Variant& message) {
  return createAndConstructThrowable(getErrorClass(), message);
}

Object AllocArithmeticErrorObject(const Variant& message) {
  return createAndConstructThrowable(getArithmeticErrorClass(), message);
}

Object AllocArgumentCountErrorObject(const Variant& message) {
  return createAndConstructThrowable(getArgumentCountErrorClass(), message);
}

Object AllocDivisionByZeroErrorObject(const Variant& message) {
  return createAndConstructThrowable(getDivisionByZeroErrorClass(), message);
}

Object AllocParseErrorObject(const Variant& message) {
  return createAndConstructThrowable(getParseErrorClass(), message);
}

Object AllocTypeErrorObject(const Variant& message) {
  return createAndConstructThrowable(getTypeErrorClass(), message);
}

Object AllocBadMethodCallExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getBadMethodCallExceptionClass(), message);
}

Object AllocInvalidArgumentExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getInvalidArgumentExceptionClass(), message);
}

Object AllocTypeAssertionExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getTypeAssertionExceptionClass(), message);
}

Object AllocRuntimeExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getRuntimeExceptionClass(), message);
}

Object AllocOutOfBoundsExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getOutOfBoundsExceptionClass(), message);
}

Object AllocInvalidOperationExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getInvalidOperationExceptionClass(), message);
}

Object AllocUnexpectedValueExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getUnexpectedValueExceptionClass(), message);
}

Object AllocDOMExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getDOMExceptionClass(), message);
}

Object AllocDivisionByZeroExceptionObject() {
  return createAndConstructThrowable(getDivisionByZeroExceptionClass(),
                                     Strings::DIVISION_BY_ZERO);
}

Object AllocInvalidForeachArgumentExceptionObject() {
  return createAndConstructThrowable(getInvalidForeachArgumentExceptionClass(),
                                     Strings::INVALID_ARGUMENT_FOREACH);
}

Object AllocUndefinedPropertyExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getUndefinedPropertyExceptionClass(),
                                     message);
}

Object AllocSoapFaultObject(const Variant& code,
                                 const Variant& message,
                                 const Variant& actor /* = uninit_variant */,
                                 const Variant& detail /* = uninit_variant */,
                                 const Variant& name /* = uninit_variant */,
                                 const Variant& header /* = uninit_variant */) {
  return createAndConstruct(
    getSoapFaultClass(),
    make_vec_array(code, message, actor, detail, name, header)
  );
}

Object AllocLazyKVZipIterableObject(const Variant& mp) {
  return createAndConstruct(getLazyKVZipIterableClass(),
                            make_vec_array(mp));
}

Object AllocLazyIterableViewObject(const Variant& iterable) {
  return createAndConstruct(getLazyIterableViewClass(),
                            make_vec_array(iterable));
}

Object AllocLazyKeyedIterableViewObject(const Variant& iterable) {
  return createAndConstruct(getLazyKeyedIterableViewClass(),
                            make_vec_array(iterable));
}

Object AllocUndefinedVariableExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getUndefinedVariableExceptionClass(), message);
}

Object AllocTypecastExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getTypecastExceptionClass(), message);
}

Object AllocReadonlyViolationExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getReadonlyViolationExceptionClass(), message);
}

Object AllocCoeffectViolationExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getCoeffectViolationExceptionClass(), message);
}

Object AllocModuleBoundaryViolationExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getModuleBoundaryViolationExceptionClass(), message);
}

Object AllocDeploymentBoundaryViolationExceptionObject(const Variant& message) {
  return createAndConstructThrowable(getDeploymentBoundaryViolationExceptionClass(), message);
}

[[noreturn]] void throwExceptionObject(const Variant& message) {
  throw_object(AllocExceptionObject(message));
}

[[noreturn]] void throwErrorObject(const Variant& message) {
  throw_object(AllocErrorObject(message));
}

[[noreturn]] void throwArithmeticErrorObject(const Variant& message) {
  throw_object(AllocArithmeticErrorObject(message));
}

[[noreturn]] void throwArgumentCountErrorObject(const Variant& message) {
  throw_object(AllocArgumentCountErrorObject(message));
}

[[noreturn]] void throwDivisionByZeroErrorObject(const Variant& message) {
  throw_object(AllocDivisionByZeroErrorObject(message));
}

[[noreturn]] void throwParseErrorObject(const Variant& message) {
  throw_object(AllocParseErrorObject(message));
}

[[noreturn]] void throwTypeErrorObject(const Variant& message) {
  throw_object(AllocTypeErrorObject(message));
}

[[noreturn]] void throwBadMethodCallExceptionObject(const Variant& message) {
  throw_object(AllocBadMethodCallExceptionObject(message));
}

[[noreturn]] void throwInvalidArgumentExceptionObject(const Variant& message) {
  throw_object(AllocInvalidArgumentExceptionObject(message));
}

[[noreturn]] void throwTypeAssertionExceptionObject(const Variant& message) {
  throw_object(AllocTypeAssertionExceptionObject(message));
}

[[noreturn]] void throwRuntimeExceptionObject(const Variant& message) {
  throw_object(AllocRuntimeExceptionObject(message));
}

[[noreturn]] void throwOutOfBoundsExceptionObject(const Variant& message) {
  throw_object(AllocOutOfBoundsExceptionObject(message));
}

[[noreturn]] void throwInvalidOperationExceptionObject(const Variant& message) {
  throw_object(AllocInvalidOperationExceptionObject(message));
}

[[noreturn]] void throwUnexpectedValueExceptionObject(const Variant& message) {
  throw_object(AllocUnexpectedValueExceptionObject(message));
}

[[noreturn]] void throwDOMExceptionObject(const Variant& message) {
  throw_object(AllocDOMExceptionObject(message));
}

[[noreturn]] void throwDivisionByZeroExceptionObject() {
  throw_object(AllocDivisionByZeroExceptionObject());
}

[[noreturn]] void throwSoapFaultObject(const Variant& code,
                                       const Variant& message,
                                       const Variant& actor /* = uninit_variant */,
                                       const Variant& detail /* = uninit_variant */,
                                       const Variant& name /* = uninit_variant */,
                                       const Variant& header /* = uninit_variant */) {
  throw_object(Object{AllocSoapFaultObject(code, message,
                                    actor, detail,
                                    name, header)});
}

[[noreturn]] void throwInvalidForeachArgumentExceptionObject() {
  throw_object(AllocInvalidForeachArgumentExceptionObject());
}

[[noreturn]] void throwUndefinedPropertyExceptionObject(const Variant& message) {
  throw_object(AllocUndefinedPropertyExceptionObject(message));
}

[[noreturn]] void throwUndefinedVariableExceptionObject(const Variant& message) {
  throw_object(AllocUndefinedVariableExceptionObject(message));
}

[[noreturn]] void throwTypecastExceptionObject(const Variant& message) {
  throw_object(AllocTypecastExceptionObject(message));
}

[[noreturn]] void throwReadonlyViolationExceptionObject(const Variant& message) {
  throw_object(AllocReadonlyViolationExceptionObject(message));
}

[[noreturn]] void throwCoeffectViolationExceptionObject(const Variant& message) {
  throw_object(AllocCoeffectViolationExceptionObject(message));
}

[[noreturn]] void throwModuleBoundaryViolationExceptionObject(const Variant& message) {
  throw_object(AllocModuleBoundaryViolationExceptionObject(message));
}

[[noreturn]] void throwDeploymentBoundaryViolationExceptionObject(const Variant& message) {
  throw_object(AllocDeploymentBoundaryViolationExceptionObject(message));
}

#define ALLOC_OBJECT_STUB(name)                                         \
  Object Alloc##name##Object() {                                        \
    return Object{get##name##Class()};                                  \
  }

ALLOC_OBJECT_STUB(Directory)
ALLOC_OBJECT_STUB(PDOException)

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

Unit* findPersistentUnit(const StringData* name) {
  for (auto unit : s_persistent_units) {
    if (unit->filepath() == name) return unit;
  }
  return nullptr;
}

namespace {

Func* setupNullClsMethod(Func* f, Class* cls, StringData* name) {
  assertx(f && f->isPhpLeafFn());
  auto clone = f->clone(cls, name);
  clone->setAttrs(static_cast<Attr>(
                    AttrPublic | AttrNoInjection | AttrDynamicallyCallable));
  clone->setRequiredCoeffects(RuntimeCoeffects::pure());
  return clone;
}

StaticString
  s___86null("__SystemLib\\__86null"),
  s___86single_arg_null("__SystemLib\\__86single_arg_null");

Func* setup86ctorMethod(Class* cls) {
  auto f = funcLoad(s___86null.get(), s_nullFunc);
  return setupNullClsMethod(f, cls, s_86ctor.get());
}

Func* setup86ReifiedInitMethod(Class* cls) {
  auto f = funcLoad(s___86single_arg_null.get(), s_singleArgNullFunc);
  return setupNullClsMethod(f, cls, s_86reifiedinit.get());
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

Func* funcLoad(const StringData* name, Func*& cache) {
  if (UNLIKELY(cache == nullptr)) {
    cache = Func::load(name);
    assertx(cache);
    // This should be here but things are broken right now. Katy will fix it
    // assertx(cache->isPersistent());
    assertx(cache->unit()->isSystemLib());
  }
  return cache;
}

Class* classLoad(const StringData* name, Class*& cache) {
  if (UNLIKELY(cache == nullptr)) {
    cache = Class::load(name);
    assertx(cache);
    assertx(cache->isPersistent());
    assertx(cache->preClass()->unit()->isSystemLib());
  }
  return cache;
}

/////////////////////////////////////////////////////////////////////////////

namespace {

struct Registered {
  std::atomic<bool> m_keep{false};
  std::mutex m_lock;
  std::vector<std::unique_ptr<UnitEmitter>> m_ues;
};

Registered& registered() {
  static Registered r;
  return r;
}

}

void keepRegisteredUnitEmitters(bool b) {
  registered().m_keep.store(b);
}

void registerUnitEmitter(std::unique_ptr<UnitEmitter> ue) {
    // Assertions are typically removed in release builds, so their performance impact is negligible.
    // However, for clarity and to avoid unnecessary checks in non-debug builds,
    // we can re-evaluate the need for the specific path check.
    assertx(ue->m_filepath->data()[0] == '/' && ue->m_filepath->data()[1] == ':');

    auto& r = registered();
    // Early exit without locking if not keeping track of emitters.
    // This is a crucial optimization for performance in scenarios where m_keep is frequently false.
    if (!r.m_keep.load(std::memory_order_relaxed)) {
        return;
    }

    std::scoped_lock<std::mutex> lock(r.m_lock);
    r.m_ues.emplace_back(std::move(ue));
}

std::vector<std::unique_ptr<UnitEmitter>> claimRegisteredUnitEmitters() {
  auto& r = registered();
  std::scoped_lock<std::mutex> _{r.m_lock};
  auto out = std::move(r.m_ues);
  assertx(r.m_ues.empty());
  return out;
}

/////////////////////////////////////////////////////////////////////////////

} // namespace HPHP::SystemLib
