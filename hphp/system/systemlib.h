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

#ifndef incl_HPHP_SYSTEMLIB_H_
#define incl_HPHP_SYSTEMLIB_H_

#include "hphp/runtime/base/types.h"
#include "hphp/util/portability.h"

namespace HPHP {
class ObjectData;
class Unit;
class Class;
class Func;
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
  x(Closure)                                    \
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

Object AllocStdClassObject();
Object AllocPinitSentinel();
Object AllocExceptionObject(const Variant& message);
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
                            const Variant& actor = null_variant,
                            const Variant& detail = null_variant,
                            const Variant& name = null_variant,
                            const Variant& header = null_variant);
Object AllocLazyKVZipIterableObject(const Variant& mp);

Object AllocLazyIterableViewObject(const Variant& iterable);
Object AllocLazyKeyedIterableViewObject(const Variant& iterable);

void throwExceptionObject(const Variant& message) ATTRIBUTE_NORETURN;
void throwBadMethodCallExceptionObject(const Variant& message)
  ATTRIBUTE_NORETURN;
void throwInvalidArgumentExceptionObject(const Variant& message)
  ATTRIBUTE_NORETURN;
void throwRuntimeExceptionObject(const Variant& message) ATTRIBUTE_NORETURN;
void throwOutOfBoundsExceptionObject(const Variant& message) ATTRIBUTE_NORETURN;
void throwInvalidOperationExceptionObject(const Variant& message)
  ATTRIBUTE_NORETURN;
void throwDOMExceptionObject(const Variant& message,
                             const Variant& code) ATTRIBUTE_NORETURN;
void throwSoapFaultObject(const Variant& code,
                          const Variant& message,
                          const Variant& actor = null_variant,
                          const Variant& detail = null_variant,
                          const Variant& name = null_variant,
                          const Variant& header = null_variant)
  ATTRIBUTE_NORETURN;


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
