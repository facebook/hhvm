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

#ifndef incl_HPHP_SYSTEMLIB_H_
#define incl_HPHP_SYSTEMLIB_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/util/portability.h"

#include <memory>

namespace HPHP {
struct ObjectData;
struct Unit;
struct Class;
struct Func;
struct Object;
struct UnitEmitter;
} //namespace HPHP

namespace HPHP::SystemLib {
///////////////////////////////////////////////////////////////////////////////

#define SYSTEMLIB_CLASSES(x)                    \
  x(stdClass,,)                                 \
  x(Exception,,)                                \
  x(BadMethodCallException,,)                   \
  x(InvalidArgumentException,,)                 \
  x(TypeAssertionException,,)                   \
  x(RuntimeException,,)                         \
  x(OutOfBoundsException,,)                     \
  x(InvalidOperationException,,)                \
  x(pinitSentinel,, __)                         \
  x(resource,, __)                              \
  x(Directory,,)                                \
  x(SplFileInfo,,)                              \
  x(SplFileObject,,)                            \
  x(DateTimeInterface,,)                        \
  x(DateTimeImmutable,,)                        \
  x(DOMException,,)                             \
  x(PDOException,,)                             \
  x(SoapFault,,)                                \
  x(Serializable,,)                             \
  x(ArrayAccess,,)                              \
  x(ArrayIterator,,)                            \
  x(DirectoryIterator,,)                        \
  x(IteratorAggregate,,)                        \
  x(Countable,,)                                \
  x(LazyKVZipIterable,,)                        \
  x(LazyIterableView,,)                         \
  x(LazyKeyedIterableView,,)                    \
  x(CURLFile,,)                                 \
  x(__PHP_Incomplete_Class,,)                   \
  x(DivisionByZeroException,,)                  \
  x(InvalidForeachArgumentException,,)          \
  x(UndefinedPropertyException,,)               \
  x(UndefinedVariableException,,)               \
  x(TypecastException,,)                        \
  x(ReadonlyViolationException,,)               \
  x(CoeffectViolationException,,)               \
  x(ModuleBoundaryViolationException,,)         \
  x(DeploymentBoundaryViolationException,,)     \
  x(Throwable,,)                                \
  x(BaseException,, \\__SystemLib\\)            \
  x(Error,,)                                    \
  x(ArithmeticError,,)                          \
  x(ArgumentCountError,,)                       \
  x(AssertionError,,)                           \
  x(DivisionByZeroError,,)                      \
  x(ParseError,,)                               \
  x(TypeError,,)                                \
  x(MethCallerHelper,, \\__SystemLib\\)         \
  x(DynMethCallerHelper,, \\__SystemLib\\)      \
  x(Traversable, HH_, HH\\)                     \
  x(Iterator, HH_, HH\\)                        \
/* */

extern bool s_anyNonPersistentBuiltins;
extern std::string s_source;
extern Unit* s_unit;
extern Func* s_nullFunc;
extern Func* s_nullCtor;

#define DECLARE_SYSTEMLIB_CLASS(cls, prefix, ...)       \
Class* get ## prefix ## cls ## Class();
  SYSTEMLIB_CLASSES(DECLARE_SYSTEMLIB_CLASS)
#undef DECLARE_SYSTEMLIB_CLASS

Object AllocStdClassObject();
Object AllocPinitSentinel();
Object AllocExceptionObject(const Variant& message);
Object AllocErrorObject(const Variant& message);
Object AllocArithmeticErrorObject(const Variant& message);
Object AllocArgumentCountErrorObject(const Variant& message);
Object AllocDivisionByZeroErrorObject(const Variant& message);
Object AllocParseErrorObject(const Variant& message);
Object AllocTypeErrorObject(const Variant& message);
Object AllocBadMethodCallExceptionObject(const Variant& message);
Object AllocInvalidArgumentExceptionObject(const Variant& message);
Object AllocTypeAssertionExceptionObject(const Variant& message);
Object AllocRuntimeExceptionObject(const Variant& message);
Object AllocOutOfBoundsExceptionObject(const Variant& message);
Object AllocInvalidOperationExceptionObject(const Variant& message);
Object AllocDOMExceptionObject(const Variant& message);
Object AllocDivisionByZeroExceptionObject();
Object AllocDirectoryObject();
Object AllocPDOExceptionObject();
Object AllocSoapFaultObject(const Variant& code,
                            const Variant& message,
                            const Variant& actor = uninit_variant,
                            const Variant& detail = uninit_variant,
                            const Variant& name = uninit_variant,
                            const Variant& header = uninit_variant);
Object AllocLazyKVZipIterableObject(const Variant& mp);

Object AllocLazyIterableViewObject(const Variant& iterable);
Object AllocLazyKeyedIterableViewObject(const Variant& iterable);

[[noreturn]] void throwExceptionObject(const Variant& message);
[[noreturn]] void throwErrorObject(const Variant& message);
[[noreturn]] void throwArithmeticErrorObject(const Variant& message);
[[noreturn]] void throwArgumentCountErrorObject(const Variant& message);
[[noreturn]] void throwDivisionByZeroErrorObject(const Variant& message);
[[noreturn]] void throwParseErrorObject(const Variant& message);
[[noreturn]] void throwTypeErrorObject(const Variant& message);
[[noreturn]]
void throwBadMethodCallExceptionObject(const Variant& message);
[[noreturn]]
void throwInvalidArgumentExceptionObject(const Variant& message);
[[noreturn]] void throwTypeAssertionExceptionObject(const Variant& message);
[[noreturn]] void throwRuntimeExceptionObject(const Variant& message);
[[noreturn]] void throwOutOfBoundsExceptionObject(const Variant& message);
[[noreturn]]
void throwInvalidOperationExceptionObject(const Variant& message);
[[noreturn]]
void throwDOMExceptionObject(const Variant& message);
[[noreturn]] void throwDivisionByZeroExceptionObject();
[[noreturn]]
void throwSoapFaultObject(const Variant& code,
                          const Variant& message,
                          const Variant& actor = uninit_variant,
                          const Variant& detail = uninit_variant,
                          const Variant& name = uninit_variant,
                          const Variant& header = uninit_variant);
[[noreturn]] void throwInvalidForeachArgumentExceptionObject();
[[noreturn]] void throwUndefinedPropertyExceptionObject(const Variant& message);
[[noreturn]] void throwUndefinedVariableExceptionObject(const Variant& message);
[[noreturn]] void throwTypecastExceptionObject(const Variant& message);
[[noreturn]] void throwReadonlyViolationExceptionObject(const Variant& message);
[[noreturn]] void throwCoeffectViolationExceptionObject(const Variant& message);
[[noreturn]] void throwModuleBoundaryViolationExceptionObject(const Variant& message);
[[noreturn]] void throwDeploymentBoundaryViolationExceptionObject(const Variant& message);

/**
 * Register a persistent unit to be re-merged (in non-repo mode)
 */
void addPersistentUnit(Unit* unit);

/**
 * Re-merge all persistent units
 */
void mergePersistentUnits();

/*
 * Setup the shared null constructor.
 */
void setupNullCtor(Class* cls);

/*
 * Return a fresh 86reifiedinit method.
 */
Func* getNull86reifiedinit(Class* cls);

Func* funcLoad(const StringData* name, Func*& cache);
Class* classLoad(const StringData* name, Class*& cache);

///////////////////////////////////////////////////////////////////////////////

/*
 * Keep a list of unit-emitters for systemlib units. Used by HPHPc so
 * it can put the unit-emitters into the repo.
 */
void keepRegisteredUnitEmitters(bool);

void registerUnitEmitter(std::unique_ptr<UnitEmitter>);

std::vector<std::unique_ptr<UnitEmitter>> claimRegisteredUnitEmitters();

///////////////////////////////////////////////////////////////////////////////

template<size_t N>
struct StringLiteral {
  /* implicit */ constexpr StringLiteral(const char (&str)[N]) {
    std::copy_n(str, N, value);
  }
  char value[N];
};

template<StringLiteral NAME>
struct ClassLoader {
  static Class* classof() {
    struct Class* cls = nullptr;
    return SystemLib::classLoad(className().get(), cls);
  }

  static const StaticString& className() {
    static const StaticString name(NAME.value);
    return name;
  }
};

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::SystemLib

#endif
