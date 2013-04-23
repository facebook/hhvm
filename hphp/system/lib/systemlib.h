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

#ifndef incl_HPHP_SYSTEMLIB_H_
#define incl_HPHP_SYSTEMLIB_H_

#include <runtime/base/types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ObjectData;
namespace VM {
  class Unit;
  class Class;
  class Func;
}
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
  x(RecursiveDirectoryIterator)                 \
  x(RecursiveIteratorIterator)                  \
  x(DirectoryIterator)                          \
  x(SplFileInfo)                                \
  x(SplFileObject)                              \
  x(DOMException)                               \
  x(PDOException)                               \
  x(SoapFault)                                  \
  x(Continuation)                               \
  x(Serializable)                               \
  x(ArrayAccess)                                \
  x(Iterator)                                   \
  x(IteratorAggregate)                          \
  x(JsonSerializable)                           \
  x(Traversable)                                \
  x(Countable)                                  \
  x(KeysIterable)                               \
  x(KVZippedIterable)                           \
  x(MappedKeyedIterable)                        \
  x(FilteredKeyedIterable)                      \
  x(ZippedKeyedIterable)                        \
  x(IterableView)                               \
  x(KeyedIterableView)                          \
  x(__PHP_Incomplete_Class)                     \

class SystemLib {
 public:
  static bool s_inited;
  static HPHP::VM::Unit* s_unit;
  static HPHP::VM::Unit* s_nativeFuncUnit;
  static HPHP::VM::Unit* s_nativeClassUnit;


#define DECLARE_SYSTEMLIB_CLASS(cls)       \
  static HPHP::VM::Class* s_ ## cls ## Class;
  SYSTEMLIB_CLASSES(DECLARE_SYSTEMLIB_CLASS)
#undef DECLARE_SYSTEMLIB_CLASS

  static HPHP::VM::Func* GetNullFunction();


  static ObjectData* AllocStdClassObject();
  static ObjectData* AllocPinitSentinel();
  static ObjectData* AllocExceptionObject(CVarRef message);
  static ObjectData* AllocBadMethodCallExceptionObject(CVarRef message);
  static ObjectData* AllocInvalidArgumentExceptionObject(CVarRef message);
  static ObjectData* AllocRuntimeExceptionObject(CVarRef message);
  static ObjectData* AllocOutOfBoundsExceptionObject(CVarRef message);
  static ObjectData* AllocInvalidOperationExceptionObject(CVarRef message);
  static ObjectData* AllocDOMExceptionObject(CVarRef message,
                                             CVarRef code);
  static ObjectData* AllocDirectoryObject();
  static ObjectData* AllocRecursiveDirectoryIteratorObject();
  static ObjectData* AllocSplFileInfoObject(CVarRef filename);
  static ObjectData* AllocSplFileObjectObject(CVarRef filename,
                                              CVarRef open_mode,
                                              CVarRef use_include_path,
                                              CVarRef context);
  static ObjectData* AllocPDOExceptionObject();
  static ObjectData* AllocSoapFaultObject(CVarRef code,
                                          CVarRef message,
                                          CVarRef actor = null_variant,
                                          CVarRef detail = null_variant,
                                          CVarRef name = null_variant,
                                          CVarRef header = null_variant);
  static ObjectData* AllocKeysIterableObject(CVarRef mp);
  static ObjectData* AllocKVZippedIterableObject(CVarRef mp);
  static ObjectData* AllocMappedKeyedIterableObject(CVarRef iterable,
                                                    CVarRef callback);
  static ObjectData* AllocFilteredKeyedIterableObject(CVarRef iterable,
                                                      CVarRef callback);
  static ObjectData* AllocZippedKeyedIterableObject(CVarRef iterable1,
                                                    CVarRef iterable2);

  static ObjectData* AllocIterableViewObject(CVarRef iterable);
  static ObjectData* AllocKeyedIterableViewObject(CVarRef iterable);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
