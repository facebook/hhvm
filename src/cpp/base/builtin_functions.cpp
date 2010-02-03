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

#include <cpp/base/builtin_functions.h>
#include <cpp/base/externals.h>
#include <cpp/base/variable_serializer.h>
#include <cpp/base/variable_unserializer.h>
#include <cpp/base/runtime_option.h>
#include <cpp/ext/ext_process.h>
#include <util/logger.h>
#include <util/util.h>
#include <util/process.h>

#include <limits>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct RuntimeInfo {
  std::set<InvocationHandler> invocation_handlers;
  hphp_const_char_map<const char*> renamed_functions;
};

static ThreadLocal<RuntimeInfo> s_runtime_info;

hphp_const_char_map<const char*> &get_renamed_functions() {
  return s_runtime_info->renamed_functions;
}

Variant f_call_user_func_array(CVarRef function, CArrRef params) {
  Array param_arr;
  if (!params.isNull()) {
    if (params->isVectorData()) {
      param_arr = params;
    } else {
      Array escalated = params;
      escalated.escalate();
      for (ArrayIter iter(escalated); iter; ++iter) {
        param_arr.append(ref(iter.secondRef()));
      }
    }
  }

  if (function.isString()) {
    String sfunction = function.toString();
    int c = sfunction.find("::");
    if (c != 0 && c != String::npos && c+2 < sfunction.size()) {
      return invoke_static_method(sfunction.substr(0, c).c_str(),
                                  sfunction.substr(c+2).c_str(), param_arr,
                                  false);
    }
    return invoke(sfunction.c_str(), param_arr, -1, true, false);
  } else if (function.is(KindOfArray)) {
    Array arr = function.toArray();
    if (!(arr.size() == 2 && arr.exists(0LL) && arr.exists(1LL))) {
      throw InvalidArgumentException("function", "not a valid callback array");
    }
    Variant classname = arr.rvalAt(0LL);
    Variant methodname = arr.rvalAt(1LL);
    if (!methodname.isString()) {
      throw InvalidArgumentException("function", "methodname not string");
    }
    String method = methodname.toString();
    if (classname.is(KindOfObject)) {
      int c = method.find("::");
      if (c != 0 && c != String::npos && c+2 < method.size()) {
        String cls = method.substr(0, c);
        if (cls == "self") {
          cls = FrameInjection::getClassName(true);
        } else if (cls == "parent") {
          cls = FrameInjection::getParentClassName(true);
        }
        return classname.toObject()->o_invoke_ex
          (cls.c_str(), method.substr(c+2).c_str(), param_arr, -1, false);
      }
      return classname.toObject()->o_invoke
        (method.c_str(), param_arr, -1, false);
    } else {
      if (!classname.isString()) {
        throw InvalidArgumentException("function", "classname not string");
      }
      String sclass = classname.toString();
      if (sclass == "self") {
        sclass = FrameInjection::getClassName(true);
      } else if (sclass == "parent") {
        sclass = FrameInjection::getParentClassName(true);
      }
      ObjectData *obj = FrameInjection::getThis(true);
      if (obj != NULL && instanceOf(Object(obj), sclass.c_str())) {
        return obj->o_invoke_ex(sclass.c_str(), method.c_str(), param_arr, -1,
                                false);
      }
      return invoke_static_method(sclass.c_str(), method.c_str(),
                                  param_arr, false);
    }
  }
  throw InvalidArgumentException("function", "not string or array");
}

Variant invoke_failed(const char *func, CArrRef params, int64 hash,
                      bool fatal /* = true */) {
  std::set<InvocationHandler> &invocation_handlers =
    s_runtime_info->invocation_handlers;
  for (std::set<InvocationHandler>::iterator it = invocation_handlers.begin();
        it != invocation_handlers.end(); ++it) {
    Variant retval;
    if ((*it)(retval, func, params, hash)) return retval;
  }
  if (fatal) {
    throw InvalidFunctionCallException(func);
  } else {
    Logger::Warning("call_user_func to non-existent function %s", func);
    return false;
  }
}

Variant o_invoke_failed(const char *cls, const char *meth,
                        bool fatal /* = true */) {
   if (fatal) {
    string msg = "Unknown method ";
    msg += cls;
    msg += "::";
    msg += meth;
    throw FatalErrorException(msg.c_str());
  } else {
    Logger::Warning("call_user_func to non-existent method %s::%s",
                    cls, meth);
    return false;
  }
}

void throw_bad_type_exception(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  string msg;
  Logger::VSNPrintf(msg, fmt, ap);
  va_end(ap);

  if (RuntimeOption::ThrowBadTypeExceptions) {
    throw InvalidOperandException(msg.c_str());
  }

  Logger::Warning("Invalid operand type was used: %s", msg.c_str());
}

void throw_infinite_loop_exception() {
  throw FatalErrorException("infinite loop detected");
}
void throw_infinite_recursion_exception() {
  throw FatalErrorException("infinite recursion detected");
}
void throw_request_timeout_exception() {
  if (RuntimeOption::RequestTimeoutSeconds > 0) {
    RequestInjectionData &data = *RequestInjection::s_reqInjectionData;
    ASSERT(data.timedout);
    data.timedout = false; // avoid going through here twice in a row

    // This extra checking is needed, because there may be a race condition
    // a TimeoutThread sets flag "true" right after an old request finishes and
    // right before a new requets resets "started". In this case, we flag
    // "timedout" back to "false".
    if (time(0) - data.started >= RuntimeOption::RequestTimeoutSeconds) {
      throw FatalErrorException("request has timed-out");
    }
  }
}

void throw_unexpected_argument_type(int argNum, const char *fnName,
                                    const char *expected, CVarRef val) {
  const char *otype = NULL;
  switch (val.getType()) {
  case KindOfNull:    otype = "null";        break;
  case KindOfBoolean: otype = "bool";        break;
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:   otype = "int";         break;
  case KindOfDouble:  otype = "double";      break;
  case LiteralString:
  case KindOfString:  otype = "string";      break;
  case KindOfArray:   otype = "array";       break;
  case KindOfObject:  otype = val.getObjectData()->o_getClassName(); break;
  default:
    ASSERT(false);
  }
  throw FatalErrorException
    ("Argument %d passed to %s must be an instance of %s, %s given",
     argNum, fnName, expected, otype);
}

void register_invocation_handler(InvocationHandler fn) {
  std::set<InvocationHandler> &invocation_handlers =
    s_runtime_info->invocation_handlers;
  invocation_handlers.insert(fn);
}

Object f_clone(Object obj) {
  Object clone = Object(obj->clone());
  clone->t___clone();
  return clone;
}

String f_serialize(CVarRef value) {
  VariableSerializer vs(VariableSerializer::Serialize);
  return vs.serialize(value, true);
}

Variant f_unserialize(CStrRef str) {
  if (str.empty()) {
    return false;
  }

  istringstream in(std::string(str.data(), str.size()));
  VariableUnserializer vu(in);
  Variant v;
  try {
    v = vu.unserialize();
  } catch (Exception &e) {
    Logger::Verbose("Unable to unserialize: [%s]. [%s] %s.", (const char *)str,
                    e.getStackTrace().hexEncode().c_str(),
                    e.getMessage().c_str());
    return false;
  }
  return v;
}

String concat3(CStrRef s1, CStrRef s2, CStrRef s3) {
  int len1 = s1.size();
  int len2 = s2.size();
  int len3 = s3.size();
  int len = len1 + len2 + len3;
  char *buf = (char *)malloc(len + 1);
  memcpy(buf, s1.data(), len1);
  memcpy(buf + len1, s2.data(), len2);
  memcpy(buf + len1 + len2, s3.data(), len3);
  buf[len] = 0;
  return String(buf, len, AttachString);
}

String concat4(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4) {
  int len1 = s1.size();
  int len2 = s2.size();
  int len3 = s3.size();
  int len4 = s4.size();
  int len = len1 + len2 + len3 + len4;
  char *buf = (char *)malloc(len + 1);
  memcpy(buf, s1.data(), len1);
  memcpy(buf + len1, s2.data(), len2);
  memcpy(buf + len1 + len2, s3.data(), len3);
  memcpy(buf + len1 + len2 + len3, s4.data(), len4);
  buf[len] = 0;
  return String(buf, len, AttachString);
}

String concat5(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4, CStrRef s5) {
  int len1 = s1.size();
  int len2 = s2.size();
  int len3 = s3.size();
  int len4 = s4.size();
  int len5 = s5.size();
  int len = len1 + len2 + len3 + len4 + len5;
  char *buf = (char *)malloc(len + 1);
  memcpy(buf, s1.data(), len1);
  memcpy(buf + len1, s2.data(), len2);
  memcpy(buf + len1 + len2, s3.data(), len3);
  memcpy(buf + len1 + len2 + len3, s4.data(), len4);
  memcpy(buf + len1 + len2 + len3 + len4, s5.data(), len5);
  buf[len] = 0;
  return String(buf, len, AttachString);
}

String concat6(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4, CStrRef s5,
               CStrRef s6) {
  int len1 = s1.size();
  int len2 = s2.size();
  int len3 = s3.size();
  int len4 = s4.size();
  int len5 = s5.size();
  int len6 = s6.size();
  int len = len1 + len2 + len3 + len4 + len5 + len6;
  char *buf = (char *)malloc(len + 1);
  memcpy(buf, s1.data(), len1);
  memcpy(buf + len1, s2.data(), len2);
  memcpy(buf + len1 + len2, s3.data(), len3);
  memcpy(buf + len1 + len2 + len3, s4.data(), len4);
  memcpy(buf + len1 + len2 + len3 + len4, s5.data(), len5);
  memcpy(buf + len1 + len2 + len3 + len4 + len5, s6.data(), len6);
  buf[len] = 0;
  return String(buf, len, AttachString);
}

Variant &concat_assign(ObjectOffset v1, CStrRef s2) {
  Variant &v = v1.lval();
  String s1 = v.toString();
  s1 += s2;
  v1 = s1;
  return v;
}

bool empty(CVarRef v, const CVarRef offset, int64 prehash /* = -1 */) {
  try {
    return !toBoolean(v.rvalAt(offset, prehash));
  } catch (UninitializedOffsetException &e) {
    // ignore this as "false" return
  }
  return false;
}

bool isset(CVarRef v, const CVarRef offset, int64 prehash /* = -1 */) {
  if (v.is(KindOfObject)) {
    return v.getArrayAccess()->o_invoke("offsetexists",
                                        Array::Create(offset.toKey()),
                                        -1);
  }
  try {
    return isset(v.rvalAt(offset, prehash));
  } catch (UninitializedOffsetException &e) {
    // ignore this as "false" return
  }
  return false;
}

String get_source_filename(litstr path) {
  if (path[0] == '/') return path;
  if (RuntimeOption::SourceRoot.empty()) {
    return Process::GetCurrentDirectory() + "/" + path;
  }
  return RuntimeOption::SourceRoot + "/" + path;
}


Variant include(CStrRef file, bool once /* = false */,
                LVariableTable* variables /* = NULL */,
                const char *currentDir /* = NULL */) {
  try {
    return invoke_file(file, once, variables, currentDir);
  } catch (PhpFileDoesNotExistException &e) {
    return false;
  }
}
Variant require(CStrRef file, bool once /* = false */,
                LVariableTable* variables /* = NULL */,
                const char *currentDir /* = NULL */) {
  try {
    return invoke_file(file, once, variables, currentDir);
  } catch (PhpFileDoesNotExistException &e) {
    String ms = "Required file that does not exist: ";
    ms += file;
    throw FatalErrorException(ms.data());
  }
}

///////////////////////////////////////////////////////////////////////////////
// class Limits

double Limits::inf_double = numeric_limits<double>::infinity();
double Limits::nan_double = numeric_limits<double>::quiet_NaN();

///////////////////////////////////////////////////////////////////////////////
}
