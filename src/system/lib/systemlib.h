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

#include <runtime/base/types.h>

#ifndef __SYSTEMLIB_H__
#define __SYSTEMLIB_H__

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

class SystemLib {
 public:
  static bool s_inited;
  static HPHP::Eval::PhpFile* s_phpFile;
  static HPHP::VM::Unit* s_unit;
  static HPHP::VM::Unit* s_nativeFuncUnit;
  static HPHP::VM::Unit* s_nativeClassUnit;
  static HPHP::VM::Class* s_stdclassClass;
  static HPHP::VM::Class* s_ExceptionClass;
  static HPHP::VM::Class* s_BadMethodCallExceptionClass;
  static HPHP::VM::Class* s_InvalidArgumentExceptionClass;
  static HPHP::VM::Class* s_RuntimeExceptionClass;
  static HPHP::VM::Class* s_OutOfBoundsExceptionClass;
  static HPHP::VM::Class* s_InvalidOperationExceptionClass;
  static HPHP::VM::Class* s_pinitSentinelClass;
  static HPHP::VM::Class* s_resourceClass;
  static HPHP::VM::Class* s_DirectoryClass;
  static HPHP::VM::Class* s_RecursiveDirectoryIteratorClass;
  static HPHP::VM::Class* s_SplFileInfoClass;
  static HPHP::VM::Class* s_SplFileObjectClass;
  static HPHP::VM::Class* s_DOMExceptionClass;
  static HPHP::VM::Class* s_PDOExceptionClass;
  static HPHP::VM::Class* s_SoapFaultClass;
  static HPHP::VM::Class* s_ContinuationClass;

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
  static ObjectData* AllocSplFileInfoObject();
  static ObjectData* AllocSplFileObjectObject();
  static ObjectData* AllocPDOExceptionObject();
  static ObjectData* AllocSoapFaultObject(CVarRef code,
                                          CVarRef message,
                                          CVarRef actor = null_variant,
                                          CVarRef detail = null_variant,
                                          CVarRef name = null_variant,
                                          CVarRef header = null_variant);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
