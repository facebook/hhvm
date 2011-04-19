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
#include <runtime/base/class_statics.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/runtime_error.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(ClassStatics)
///////////////////////////////////////////////////////////////////////////////

ClassStatics::ClassStatics(int redecId) : m_clsname(NULL), m_redecId(redecId) {
}

ClassStatics::ClassStatics(litstr name) : m_clsname(name), m_redecId(-1) {
}

///////////////////////////////////////////////////////////////////////////////

void ClassStatics::throwUnknownClass() {
  if (m_clsname && *m_clsname) {
    throw FatalErrorException(0, "unknown class %s", m_clsname);
  } else {
    throw FatalErrorException("unknown class ");
  }
}

Variant ClassStatics::os_getInit(CStrRef s) {
  throwUnknownClass();
  return null;
}

Variant ClassStatics::os_get(CStrRef s) {
  throwUnknownClass();
  return null;
}

Variant &ClassStatics::os_lval(CStrRef s) {
  throwUnknownClass();
  throw 0; // suppress compiler error
}

Variant ClassStatics::os_invoke(const char *c, const char *s,
                                CArrRef params, int64 hash /* = -1 */,
                                bool fatal /* = true */) {
  if (fatal) {
    throwUnknownClass();
  }

  raise_warning("call_user_func to non-existent method %s::%s", c, s);
  return false;
}

Object ClassStatics::create(CArrRef params, bool init /* = true */,
                            ObjectData* root /* = NULL */) {
  Object o(createOnly(root));
  if (init) {
    MethodCallPackage mcp;
    mcp.construct(o);
    if (mcp.ci) {
      (mcp.ci->getMeth())(mcp, params);
    }
  }
  return o;
}

Object ClassStatics::createOnly(ObjectData* root /* = NULL */) {
  throwUnknownClass();
  return null_object;
}

Variant ClassStatics::os_constant(const char *s) {
  throwUnknownClass();
  return null;
}

bool ClassStatics::os_get_call_info(MethodCallPackage &info,
    int64 hash /* = -1 */) {
  info.fail();
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
