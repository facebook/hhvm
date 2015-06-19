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

#ifndef incl_HPHP_EXT_HHBC_H_
#define incl_HPHP_EXT_HHBC_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/preclass.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ActRec;
struct TypedValue;

struct HhbcExtFuncInfo {
  const char* m_name;
  TypedValue* (*m_builtinFunc)(ActRec* ar);
  void* m_nativeFunc;
};

struct HhbcExtMethodInfo {
  const char* m_name;
  TypedValue* (*m_pGenericMethod)(ActRec* ar);
  void* m_nativeFunc;
};

struct HhbcExtClassInfo {
  const char* m_name;
  BuiltinCtorFunction m_instanceCtor;
  BuiltinDtorFunction m_instanceDtor;
  size_t m_totalSize;
  ptrdiff_t m_objectDataOffset;
  long long m_methodCount;
  const HhbcExtMethodInfo* m_methods;
  LowPtr<Class>* m_clsPtr;
};

extern const long long hhbc_ext_funcs_count;
extern const HhbcExtFuncInfo hhbc_ext_funcs[];
extern const long long hhbc_ext_class_count;
extern const HhbcExtClassInfo hhbc_ext_classes[];

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_HHBC_H_
