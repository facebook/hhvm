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
#include "hphp/util/portability.h"

namespace HPHP {
struct ObjectData;
struct Unit;
struct Class;
struct Func;
struct Object;
} //namespace HPHP

namespace HPHP { namespace SystemLib {
///////////////////////////////////////////////////////////////////////////////

#define SYSTEMLIB_CLASSES(x)                    \
  x(stdclass)                                   \
  x(Exception)                                  \
  x(BadMethodCallException)                     \
  x(InvalidArgumentException)                   \
  x(RuntimeException)                           \
  x(OutOfBoundsException)                       \
  x(InvalidOperationException)                  \
  x(pinitSentinel)                              \
  x(resource)                                   \
  x(Directory)                                  \
  x(SplFileInfo)                                \
  x(SplFileObject)                              \
  x(DateTimeInterface)                          \
  x(DateTimeImmutable)                          \
  x(DOMException)                               \
  x(PDOException)                               \
  x(SoapFault)                                  \
  x(Serializable)                               \
  x(ArrayAccess)                                \
  x(ArrayObject)                                \
  x(ArrayIterator)                              \
  x(Iterator)                                   \
  x(IteratorAggregate)                          \
  x(Traversable)                                \
  x(Countable)                                  \
  x(LazyKVZipIterable)                          \
  x(LazyIterableView)                           \
  x(LazyKeyedIterableView)                      \
  x(Phar)                                       \
  x(CURLFile)                                   \
  x(__PHP_Incomplete_Class)                     \
  x(APCIterator)

extern bool s_inited;
extern bool s_anyNonPersistentBuiltins;
extern std::string s_source;
extern Unit* s_unit;
extern Unit* s_hhas_unit;
extern Unit* s_nativeFuncUnit;
extern Unit* s_nativeClassUnit;
extern Func* s_nullFunc;

#define DECLARE_SYSTEMLIB_CLASS(cls)       \
extern Class* s_ ## cls ## Class;
  SYSTEMLIB_CLASSES(DECLARE_SYSTEMLIB_CLASS)
#undef DECLARE_SYSTEMLIB_CLASS

extern Class* s_ThrowableClass;
extern Class* s_BaseExceptionClass;
extern Class* s_ErrorClass;
extern Class* s_ArithmeticErrorClass;
extern Class* s_AssertionErrorClass;
extern Class* s_DivisionByZeroErrorClass;
extern Class* s_ParseErrorClass;
extern Class* s_TypeErrorClass;

Object AllocStdClassObject();
Object AllocPinitSentinel();
Object AllocExceptionObject(const Variant& message);
Object AllocErrorObject(const Variant& message);
Object AllocArithmeticErrorObject(const Variant& message);
Object AllocDivisionByZeroErrorObject(const Variant& message);
Object AllocParseErrorObject(const Variant& message);
Object AllocTypeErrorObject(const Variant& message);
Object AllocBadMethodCallExceptionObject(const Variant& message);
Object AllocInvalidArgumentExceptionObject(const Variant& message);
Object AllocRuntimeExceptionObject(const Variant& message);
Object AllocOutOfBoundsExceptionObject(const Variant& message);
Object AllocInvalidOperationExceptionObject(const Variant& message);
Object AllocDOMExceptionObject(const Variant& message,
                               const Variant& code);
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
[[noreturn]] void throwDivisionByZeroErrorObject(const Variant& message);
[[noreturn]] void throwParseErrorObject(const Variant& message);
[[noreturn]] void throwTypeErrorObject(const Variant& message);
[[noreturn]]
void throwBadMethodCallExceptionObject(const Variant& message);
[[noreturn]]
void throwInvalidArgumentExceptionObject(const Variant& message);
[[noreturn]] void throwRuntimeExceptionObject(const Variant& message);
[[noreturn]] void throwOutOfBoundsExceptionObject(const Variant& message);
[[noreturn]]
void throwInvalidOperationExceptionObject(const Variant& message);
[[noreturn]]
void throwDOMExceptionObject(const Variant& message,
                             const Variant& code);
[[noreturn]]
void throwSoapFaultObject(const Variant& code,
                          const Variant& message,
                          const Variant& actor = uninit_variant,
                          const Variant& detail = uninit_variant,
                          const Variant& name = uninit_variant,
                          const Variant& header = uninit_variant);


/**
 * Register a persistent unit to be re-merged (in non-repo mode)
 */
void addPersistentUnit(Unit* unit);

/**
 * Re-merge all persistent units
 */
void mergePersistentUnits();

///////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::SystemLib

#endif
