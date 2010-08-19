/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
#include <runtime/base/runtime_option.h>

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(ClassStatics)
///////////////////////////////////////////////////////////////////////////////

ClassStatics::ClassStatics(int redecId) : m_redecId(redecId) {
  m_msg = "unknown class";
}

ClassStatics::ClassStatics(const std::string& name) : m_redecId(-1) {
  m_msg = "unknown class ";
  m_msg += name.c_str();
}

///////////////////////////////////////////////////////////////////////////////

Variant ClassStatics::os_getInit(CStrRef s) {
  throw FatalErrorException(m_msg);
}

Variant ClassStatics::os_get(CStrRef s) {
  throw FatalErrorException(m_msg);
}

Variant &ClassStatics::os_lval(CStrRef s) {
  throw FatalErrorException(m_msg);
}

Variant ClassStatics::os_invoke(const char *c, MethodIndex methodIndex,
                                const char *s,
                                CArrRef params, int64 hash,
                                bool fatal /* = true */) {
  if (fatal) {
    throw FatalErrorException(m_msg);
  } else {
    if (RuntimeOption::FastMethodCall) {
      // only call with known methodIndex, must succeed
      s = g_bypassMILR ? s : methodIndexLookupReverse(methodIndex);
    }
    raise_warning("call_user_func to non-existent method %s::%s",
                    c, s);
    return false;
  }
}

Variant ClassStatics::os_invoke_mil(const char *c,
                                    const char *s,
                                    CArrRef params, int64 hash,
                                    bool fatal /* = true */) {

  MethodIndex methodIndex (MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(s);
    if (methodIndex.isFail()) {
      raise_warning("call_user_func to non-existent method %s::%s", c, s);
    }
  }
  // redispatch
  return os_invoke(c, methodIndex, s, params, hash, fatal);
}

Object ClassStatics::create(CArrRef params, bool init /* = true */,
                            ObjectData* root /* = NULL */) {
  throw FatalErrorException(m_msg);
}

Variant ClassStatics::os_constant(const char *s) {
  throw FatalErrorException(m_msg);
}

Variant ClassStatics::os_invoke_from_eval
(const char *c, const char *s,
 Eval::VariableEnvironment &env,
 const Eval::FunctionCallExpression *call, int64 hash,
 bool fatal /* = true */) {
  if (fatal) {
    throw FatalErrorException(m_msg);
  } else {
    raise_warning("call_user_func to non-existent method %s::%s",
                    c, s);
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
