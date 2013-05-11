/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ActRec;

namespace VM {
  class Instance;
};

struct HhbcExtFuncInfo {
  const char* m_name;
  TypedValue* (*m_builtinFunc)(ActRec* ar);
  void* m_nativeFunc;
};

struct HhbcExtMethodInfo {
  const char* m_name;
  TypedValue* (*m_pGenericMethod)(ActRec* ar);
};

struct HhbcExtClassInfo {
  const char* m_name;
  HPHP::VM::Instance* (*m_InstanceCtor)(HPHP::VM::Class*);
  int m_sizeof;
  long long m_methodCount;
  const HhbcExtMethodInfo* m_methods;
  HPHP::VM::Class** m_clsPtr;
};

extern const long long hhbc_ext_funcs_count;
extern const HhbcExtFuncInfo hhbc_ext_funcs[];
extern const long long hhbc_ext_class_count;
extern const HhbcExtClassInfo hhbc_ext_classes[];

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_HHBC_H_

