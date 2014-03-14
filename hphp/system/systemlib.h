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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ObjectData;
class Unit;
class Class;
class Func;
namespace Eval {
  class PhpFile;
}

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
  x(DOMDocument)                                \
  x(DOMException)                               \
  x(PDOException)                               \
  x(SoapFault)                                  \
  x(Closure)                                    \
  x(Continuation)                               \
  x(Serializable)                               \
  x(ArrayAccess)                                \
  x(ArrayObject)                                \
  x(Iterator)                                   \
  x(IteratorAggregate)                          \
  x(Traversable)                                \
  x(Countable)                                  \
  x(LazyKVZipIterable)                          \
  x(LazyIterableView)                           \
  x(LazyKeyedIterableView)                      \
  x(Phar)                                       \
  x(__PHP_Incomplete_Class)                     \
  x(__PHP_Unserializable_Class)                 \

class SystemLib {
 public:
  static bool s_inited;
  static std::string s_source;
  static HPHP::Unit* s_unit;
  static HPHP::Unit* s_hhas_unit;
  static HPHP::Unit* s_nativeFuncUnit;
  static HPHP::Unit* s_nativeClassUnit;


#define DECLARE_SYSTEMLIB_CLASS(cls)       \
  static HPHP::Class* s_ ## cls ## Class;
  SYSTEMLIB_CLASSES(DECLARE_SYSTEMLIB_CLASS)
#undef DECLARE_SYSTEMLIB_CLASS

  static HPHP::Func* s_nullFunc;

  static ObjectData* AllocStdClassObject();
  static ObjectData* AllocPinitSentinel();
  static ObjectData* AllocExceptionObject(const Variant& message);
  static ObjectData* AllocBadMethodCallExceptionObject(const Variant& message);
  static ObjectData* AllocInvalidArgumentExceptionObject(const Variant& message);
  static ObjectData* AllocRuntimeExceptionObject(const Variant& message);
  static ObjectData* AllocOutOfBoundsExceptionObject(const Variant& message);
  static ObjectData* AllocInvalidOperationExceptionObject(const Variant& message);
  static ObjectData* AllocDOMDocumentObject(const String& version = null_string,
                                            const String& encoding = null_string);
  static ObjectData* AllocDOMExceptionObject(const Variant& message,
                                             const Variant& code);
  static ObjectData* AllocDirectoryObject();
  static ObjectData* AllocPDOExceptionObject();
  static ObjectData* AllocSoapFaultObject(const Variant& code,
                                          const Variant& message,
                                          const Variant& actor = null_variant,
                                          const Variant& detail = null_variant,
                                          const Variant& name = null_variant,
                                          const Variant& header = null_variant);
  static ObjectData* AllocLazyKVZipIterableObject(const Variant& mp);

  static ObjectData* AllocLazyIterableViewObject(const Variant& iterable);
  static ObjectData* AllocLazyKeyedIterableViewObject(const Variant& iterable);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
