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

#include <runtime/base/builtin_functions.h>
#include <runtime/base/externals.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/variable_unserializer.h>
#include <runtime/base/runtime_option.h>
#include <runtime/ext/ext_process.h>
#include <util/logger.h>
#include <util/util.h>
#include <util/process.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/util/request_local.h>

#include <limits>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// static strings

static StaticString s_offsetExists("offsetExists");
static StaticString s_class_exists("class_exists");
static StaticString s___autoload("__autoload");

///////////////////////////////////////////////////////////////////////////////
// fb_rename_function()

class RenameManager : public RequestEventHandler {
public:
  RenameManager() : use_allowed_functions(false) {
  }

  void clear() {
    // Note: ExecutionContext::onRequestShutdown() is called before
    // MemoryManager::rollback(), so it is safe to store smart-allocated
    // StringData objects here.
    use_allowed_functions = false;
    allowed_functions.clear();
    renamed_functions.clear();
    unmapped_functions.clear();
  }

  virtual void requestInit() {
    clear();
  }

  virtual void requestShutdown() {
    clear();
  }

  bool use_allowed_functions;
  StringISet allowed_functions;
  StringIMap<String> renamed_functions;
  StringISet unmapped_functions;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(RenameManager, s_rename_manager);

void check_renamed_functions(CArrRef names) {
  s_rename_manager->use_allowed_functions = true;
  StringISet &allowed = s_rename_manager->allowed_functions;
  for (ArrayIter iter(names); iter; ++iter) {
    String name = iter.second().toString();
    if (!name.empty()) {
      allowed.insert(name);
    }
  }
}

bool check_renamed_function(CStrRef name) {
  if (s_rename_manager->use_allowed_functions) {
    StringISet &allowed = s_rename_manager->allowed_functions;
    return allowed.find(name) != allowed.end();
  }
  return true;
}

StringIMap<String> &get_renamed_functions() {
  return s_rename_manager->renamed_functions;
}

StringISet &get_unmapped_functions() {
  return s_rename_manager->unmapped_functions;
}

///////////////////////////////////////////////////////////////////////////////

bool class_exists(CStrRef class_name, bool autoload /* = true */) {
  return f_call_user_func_array(s_class_exists,
      CREATE_VECTOR2(class_name, autoload));
}

String get_static_class_name(CVarRef objOrClassName) {
  if (objOrClassName.isString()) {
    return objOrClassName.toString();
  }
  if (objOrClassName.isObject()) {
    return objOrClassName.toObject()->o_getClassName();
  }
  raise_error("Class name must be a valid object or a string");
  return "";
}

Variant getDynamicConstant(CVarRef v, CStrRef name) {
  if (isInitialized(v)) return v;
  raise_notice("Use of undefined constant %s -- assumed '%s'.",
               name.c_str(), name.c_str());
  return name;
}

String getUndefinedConstant(CStrRef name) {
  raise_notice("Use of undefined constant %s -- assumed '%s'.",
               name.c_str(), name.c_str());
  return name;
}

Variant f_call_user_func_array(CVarRef function, CArrRef params) {
  if (function.isString()) {
    String sfunction = function.toString();
    int c = sfunction.find("::");
    if (c != 0 && c != String::npos && c+2 < sfunction.size()) {
      return invoke_static_method_mil(sfunction.substr(0, c).c_str(),
                                      sfunction.substr(c+2).c_str(), params,
                                      false);
    }
    return invoke(sfunction, params, -1, true, false);
  } else if (function.is(KindOfArray)) {
    Array arr = function.toArray();
    if (!(arr.size() == 2 && arr.exists(0LL) && arr.exists(1LL))) {
      throw_invalid_argument("function: not a valid callback array");
      return null;
    }
    Variant classname = arr.rvalAt(0LL);
    Variant methodname = arr.rvalAt(1LL);
    if (!methodname.isString()) {
      throw_invalid_argument("function: methodname not string");
      return null;
    }
    String method = methodname.toString();
    if (classname.is(KindOfObject)) {
      int c = method.find("::");
      if (c != 0 && c != String::npos && c+2 < method.size()) {
        // e.g. call_user_func_array(array($g,'G::f'),array(2));
        String cls = method.substr(0, c);
        if (cls == "self") {
          cls = FrameInjection::GetClassName(true);
        } else if (cls == "parent") {
          cls = FrameInjection::GetParentClassName(true);
        }
        return classname.toObject()->o_invoke_ex_mil
          (cls.c_str(), method.substr(c+2).c_str(), params, -1, false);
      }
      return classname.toObject()->o_invoke_mil(method.c_str(), params,
                                                -1, false);
    } else {
      if (!classname.isString()) {
        throw_invalid_argument("function: classname not string");
        return null;
      }
      String sclass = classname.toString();
      if (sclass == "self") {
        sclass = FrameInjection::GetClassName(true);
      } else if (sclass == "parent") {
        sclass = FrameInjection::GetParentClassName(true);
      }
      Object obj = FrameInjection::GetThis(true);
      if (obj.instanceof(sclass.c_str())) {
        return obj->o_invoke_ex_mil(sclass.c_str(), method.c_str(), params,
                                    -1, false);
      }
      return invoke_static_method_mil(sclass.c_str(), method.c_str(),
                                      params, false);
    }
  }
  throw_invalid_argument("function: not string or array");
  return null;
}

Variant invoke(CStrRef function, CArrRef params, int64 hash /* = -1 */,
               bool tryInterp /* = true */, bool fatal /* = true */) {
  StringData *sd = function.get();
  ASSERT(sd && sd->data());
  return invoke(sd->data(), params, hash < 0 ? sd->hash() : hash,
                tryInterp, fatal);
}

Variant invoke_failed(const char *func, CArrRef params, int64 hash,
                      bool fatal /* = true */) {
  if (fatal) {
    throw InvalidFunctionCallException(func);
  } else {
    raise_warning("call_user_func to non-existent function %s", func);
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
    raise_warning("call_user_func to non-existent method %s::%s", cls, meth);
    return false;
  }
}

void throw_instance_method_fatal(const char *name) {
  if (!strstr(name, "::__destruct")) {
    raise_error("Non-static method %s() cannot be called statically", name);
  }
}

void RequestInjection::checkSurprise(ThreadInfo *info) {
  RequestInjectionData &p = info->m_reqInjectionData;
  bool do_timedout, do_memExceeded, do_signaled;

  p.surpriseMutex.lock();

  // Even though we checked surprise outside of the lock, we don't need to
  // check again, because the only code that can ever set surprised to false
  // is right here - and this function is never called from another thread.

  p.surprised = false;

  do_timedout = p.timedout;
  do_memExceeded = p.memExceeded;
  do_signaled = p.signaled;

  p.timedout = false;
  p.memExceeded = false;
  p.signaled = false;

  p.surpriseMutex.unlock();

  if (do_timedout) throw_request_timeout_exception();
  if (do_memExceeded) throw_memory_exceeded_exception();
  if (do_signaled) f_pcntl_signal_dispatch();
}

Variant throw_missing_arguments(const char *fn, int num, int level /* = 0 */) {
  if (level == 2 || RuntimeOption::ThrowMissingArguments) {
    raise_error("Missing argument %d for %s()", num, fn);
  } else {
    raise_warning("Missing argument %d for %s()", num, fn);
  }
  return null;
}

Variant throw_toomany_arguments(const char *fn, int num, int level /* = 0 */) {
  if (level == 2 || RuntimeOption::ThrowTooManyArguments) {
    raise_error("Too many arguments for %s(), expected %d", fn, num);
  } else if (level == 1 || RuntimeOption::WarnTooManyArguments) {
    raise_warning("Too many arguments for %s(), expected %d", fn, num);
  }
  return null;
}

Variant throw_wrong_arguments(const char *fn, int count, int cmin, int cmax,
                              int level /* = 0 */) {
  if (cmin >= 0 && count < cmin) {
    return throw_missing_arguments(fn, count + 1, level);
  }
  if (cmax >= 0 && count > cmax) {
    return throw_toomany_arguments(fn, cmax, level);
  }
  ASSERT(false);
  return null;
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

  raise_warning("Invalid operand type was used: %s", msg.c_str());
}

void throw_invalid_argument(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  string msg;
  Logger::VSNPrintf(msg, fmt, ap);
  va_end(ap);

  if (RuntimeOption::ThrowInvalidArguments) {
    throw InvalidArgumentException(msg.c_str());
  }

  raise_warning("Invalid argument: %s", msg.c_str());
}

void throw_infinite_loop_exception() {
  if (!RuntimeOption::NoInfiniteLoopDetection) {
    throw FatalErrorException("infinite loop detected");
  }
}
void throw_infinite_recursion_exception() {
  if (!RuntimeOption::NoInfiniteRecursionDetection) {
    // Reset profiler otherwise it might recurse further causing segfault
    DECLARE_THREAD_INFO
    info->m_profiler = NULL;
    throw UncatchableException("infinite recursion detected");
  }
}
void throw_request_timeout_exception() {
  ThreadInfo *info = ThreadInfo::s_threadInfo.get();
  RequestInjectionData &data = info->m_reqInjectionData;
  if (data.timeoutSeconds > 0) {
    // This extra checking is needed, because there may be a race condition
    // a TimeoutThread sets flag "true" right after an old request finishes and
    // right before a new requets resets "started". In this case, we flag
    // "timedout" back to "false".
    if (time(0) - data.started >= data.timeoutSeconds) {
      throw FatalErrorException("request has timed-out");
    }
  }
}

void throw_memory_exceeded_exception() {
  // NOTE: This is marked as __attribute__((noreturn))
  // in base/types.h AND base/builtin_functions.h
  throw UncatchableException("request has exceeded memory limit");
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
  case KindOfStaticString:
  case KindOfString:  otype = "string";      break;
  case KindOfArray:   otype = "array";       break;
  case KindOfObject:  otype = val.getObjectData()->o_getClassName(); break;
  default:
    ASSERT(false);
  }
  raise_recoverable_error
    ("Argument %d passed to %s must be an instance of %s, %s given",
     argNum, fnName, expected, otype);
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
    raise_notice("Unable to unserialize: [%s]. [%s] %s.", (const char *)str,
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
  if (buf == NULL) {
    throw FatalErrorException("malloc failed: %d", len);
  }
  memcpy(buf, s1.data(), len1);
  memcpy(buf + len1, s2.data(), len2);
  memcpy(buf + len1 + len2, s3.data(), len3);
  buf[len] = 0;
  #ifndef TAINTED
  return String(buf, len, AttachString);
  #else
  String res = String(buf, len, AttachString);
  propagate_tainting3(s1, s2, s3, res);
  return res;
  #endif
}

String concat4(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4) {
  int len1 = s1.size();
  int len2 = s2.size();
  int len3 = s3.size();
  int len4 = s4.size();
  int len = len1 + len2 + len3 + len4;
  char *buf = (char *)malloc(len + 1);
  if (buf == NULL) {
    throw FatalErrorException("malloc failed: %d", len);
  }
  memcpy(buf, s1.data(), len1);
  memcpy(buf + len1, s2.data(), len2);
  memcpy(buf + len1 + len2, s3.data(), len3);
  memcpy(buf + len1 + len2 + len3, s4.data(), len4);
  buf[len] = 0;
  #ifndef TAINTED
  return String(buf, len, AttachString);
  #else
  String res = String(buf, len, AttachString);
  propagate_tainting4(s1, s2, s3, s4, res);
  return res;
  #endif
}

String concat5(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4, CStrRef s5) {
  int len1 = s1.size();
  int len2 = s2.size();
  int len3 = s3.size();
  int len4 = s4.size();
  int len5 = s5.size();
  int len = len1 + len2 + len3 + len4 + len5;
  char *buf = (char *)malloc(len + 1);
  if (buf == NULL) {
    throw FatalErrorException("malloc failed: %d", len);
  }
  memcpy(buf, s1.data(), len1);
  memcpy(buf + len1, s2.data(), len2);
  memcpy(buf + len1 + len2, s3.data(), len3);
  memcpy(buf + len1 + len2 + len3, s4.data(), len4);
  memcpy(buf + len1 + len2 + len3 + len4, s5.data(), len5);
  buf[len] = 0;
  #ifndef TAINTED
  return String(buf, len, AttachString);
  #else
  String res = String(buf, len, AttachString);
  propagate_tainting5(s1, s2, s3, s4, s5, res);
  return res;
  #endif
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
  if (buf == NULL) {
    throw FatalErrorException("malloc failed: %d", len);
  }
  memcpy(buf, s1.data(), len1);
  memcpy(buf + len1, s2.data(), len2);
  memcpy(buf + len1 + len2, s3.data(), len3);
  memcpy(buf + len1 + len2 + len3, s4.data(), len4);
  memcpy(buf + len1 + len2 + len3 + len4, s5.data(), len5);
  memcpy(buf + len1 + len2 + len3 + len4 + len5, s6.data(), len6);
  buf[len] = 0;
  #ifndef TAINTED
  return String(buf, len, AttachString);
  #else
  String res = String(buf, len, AttachString);
  propagate_tainting6(s1, s2, s3, s4, s5, s6, res);
  return res;
  #endif
}

String concat_assign(ObjectOffset v1, CStrRef s2) {
  Variant &v = v1.lval();
  String s1 = v.toString();
  s1 += s2;
  #ifdef TAINTED
  propagate_tainting2(s1, s2, s1);
  #endif
  v1 = s1;
  return s1;
}

bool empty(CVarRef v, bool    offset, int64 prehash /* = -1 */) {
  return empty(v, Variant(offset), prehash);
}
bool empty(CVarRef v, char    offset, int64 prehash /* = -1 */) {
  return empty(v, Variant(offset), prehash);
}
bool empty(CVarRef v, short   offset, int64 prehash /* = -1 */) {
  return empty(v, Variant(offset), prehash);
}
bool empty(CVarRef v, int     offset, int64 prehash /* = -1 */) {
  return empty(v, Variant(offset), prehash);
}
bool empty(CVarRef v, int64   offset, int64 prehash /* = -1 */) {
  return empty(v, Variant(offset), prehash);
}
bool empty(CVarRef v, double  offset, int64 prehash /* = -1 */) {
  return empty(v, Variant(offset), prehash);
}
bool empty(CVarRef v, CArrRef offset, int64 prehash /* = -1 */) {
  return empty(v, Variant(offset), prehash);
}
bool empty(CVarRef v, CObjRef offset, int64 prehash /* = -1 */) {
  return empty(v, Variant(offset), prehash);
}

bool empty(CVarRef v, litstr offset, int64 prehash /* = -1 */,
           bool isString /* = false */) {
  if (!v.isArray()) {
    return empty(v, Variant(offset), prehash);
  }
  return !toBoolean(v.rvalAt(offset, prehash, false, isString));
}

bool empty(CVarRef v, CStrRef offset, int64 prehash /* = -1 */,
           bool isString /* = false */) {
  if (!v.isArray()) {
    return empty(v, Variant(offset), prehash);
  }
  return !toBoolean(v.rvalAt(offset, prehash, false, isString));
}

bool empty(CVarRef v, CVarRef offset, int64 prehash /* = -1 */) {
  if (v.is(KindOfObject)) {
    if (!v.getArrayAccess()->o_invoke(s_offsetExists, Array::Create(offset))) {
      return true;
    }
    // fall through to check for 'empty'ness of the value.
  } else if (v.isString()) {
    int pos = offset.toInt32();
    if (pos < 0 || pos >= v.toString().size()) {
      return true;
    }
  }
  return !toBoolean(v.rvalAt(offset, prehash));
}

bool isset(CVarRef v, bool    offset, int64 prehash /* = -1 */) {
  return isset(v, Variant(offset), prehash);
}
bool isset(CVarRef v, char    offset, int64 prehash /* = -1 */) {
  return isset(v, Variant(offset), prehash);
}
bool isset(CVarRef v, short   offset, int64 prehash /* = -1 */) {
  return isset(v, Variant(offset), prehash);
}
bool isset(CVarRef v, int     offset, int64 prehash /* = -1 */) {
  return isset(v, Variant(offset), prehash);
}
bool isset(CVarRef v, int64   offset, int64 prehash /* = -1 */) {
  return isset(v, Variant(offset), prehash);
}
bool isset(CVarRef v, double  offset, int64 prehash /* = -1 */) {
  return isset(v, Variant(offset), prehash);
}
bool isset(CVarRef v, CArrRef offset, int64 prehash /* = -1 */) {
  return isset(v, Variant(offset), prehash);
}
bool isset(CVarRef v, CObjRef offset, int64 prehash /* = -1 */) {
  return isset(v, Variant(offset), prehash);
}

bool isset(CVarRef v, CVarRef offset, int64 prehash /* = -1 */) {
  if (v.is(KindOfObject)) {
    return v.getArrayAccess()->o_invoke(s_offsetExists,
                                        Array::Create(offset), -1);
  }
  if (v.isString()) {
    int pos = offset.toInt32();
    return pos >= 0 && pos < v.toString().size();
  }
  return isset(v.rvalAt(offset, prehash));
}

bool isset(CVarRef v, litstr offset, int64 prehash /* = -1 */,
           bool isString /* = false */) {
  if (v.is(KindOfObject) || v.isString()) {
    return isset(v, Variant(offset), prehash);
  }
  return isset(v.rvalAt(offset, prehash, false, isString));
}

bool isset(CVarRef v, CStrRef offset, int64 prehash /* = -1 */,
           bool isString /* = false */) {
  if (v.is(KindOfObject) || v.isString()) {
    return isset(v, Variant(offset), prehash);
  }
  return isset(v.rvalAt(offset, prehash, false, isString));
}

String get_source_filename(litstr path) {
  if (path[0] == '/') return path;
  if (RuntimeOption::SourceRoot.empty()) {
    return Process::GetCurrentDirectory() + "/" + path;
  }
  return RuntimeOption::SourceRoot + path;
}

Variant include_impl_invoke(CStrRef file, bool once,
                                   LVariableTable* variables,
                                   const char *currentDir) {
  if (file[0] == '/') {
    try {
      return invoke_file(file, once, variables, currentDir);
    } catch(PhpFileDoesNotExistException &e) {}

    string server_root = RuntimeOption::SourceRoot;
    if (server_root.empty()) {
      server_root = string(g_context->getCwd()->data()) + "/";
    }

    String rel_path(Util::relativePath(server_root, string(file.data())));

    // Don't try/catch - We want the exception to be passed along
    return invoke_file(rel_path, once, variables, currentDir);
  } else {
    // Don't try/catch - We want the exception to be passed along
    return invoke_file(file, once, variables, currentDir);
  }
}

static Variant include_impl(CStrRef file, bool once,
                            LVariableTable* variables,
                            const char *currentDir, bool required) {

  const char* c_file = file->data();

  if (c_file[0] == '/') {
    String can_path(Util::canonicalize(file.c_str(), file.size()));

    try {
      return include_impl_invoke(can_path, once, variables, currentDir);
    } catch (PhpFileDoesNotExistException &e) {}

  } else if ((c_file[0] == '.' && (c_file[1] == '/' || (
    c_file[1] == '.' && c_file[2] == '/')))) {

    String path(String(g_context->getCwd() + "/" + file));
    String can_path(Util::canonicalize(path.c_str(), path.size()));

    try {
      return include_impl_invoke(can_path, once, variables, currentDir);
    } catch (PhpFileDoesNotExistException &e) {}


  } else {

    unsigned int path_count = RuntimeOption::IncludeSearchPaths.size();

    for (unsigned int i = 0; i < path_count; i++) {
      String path("");

      if (RuntimeOption::IncludeSearchPaths[i][0] != '/') {
        path += (g_context->getCwd() + "/");
      }

      if (path[path.size() - 1] != '/') {
        path += "/";
      }

      path += file;
      String can_path(Util::canonicalize(path.c_str(), path.size()));

      try {
        return include_impl_invoke(can_path, once, variables, currentDir);
      } catch (PhpFileDoesNotExistException &e) {}
    }

    if (currentDir[0] == '/') {
      // We are in hphpi, which passes an absolute path
      String path(currentDir);
      path += "/";
      path += file;
      String can_path(Util::canonicalize(path.c_str(), path.size()));

      try {
        return include_impl_invoke(can_path, once, variables, currentDir);
      } catch (PhpFileDoesNotExistException &e) {}
    } else {
      // Regular hphp
      String path(g_context->getCwd() + "/" + currentDir + file);
      String can_path(Util::canonicalize(path.c_str(), path.size()));

      try {
        return include_impl_invoke(can_path, once, variables, currentDir);
      } catch (PhpFileDoesNotExistException &e) {}
    }
  }

  // Failure
  raise_notice("Tried to invoke %s but file not found.", file->data());
  if (required) {
    String ms = "Required file that does not exist: ";
    ms += file;
    throw FatalErrorException(ms.data());
  }
  return false;
}

Variant include(CStrRef file, bool once /* = false */,
                LVariableTable* variables /* = NULL */,
                const char *currentDir /* = NULL */) {
  return include_impl(file, once, variables, currentDir, false);
}

Variant require(CStrRef file, bool once /* = false */,
                LVariableTable* variables /* = NULL */,
                const char *currentDir /* = NULL */) {
  return include_impl(file, once, variables, currentDir, true);
}

///////////////////////////////////////////////////////////////////////////////
// class Limits

bool function_exists(CStrRef function_name) {
  String name = function_name;

  StringISet &unmap = get_unmapped_functions();
  if (unmap.find(function_name) != unmap.end()) {
    return false;
  }
  StringIMap<String> &remap = get_renamed_functions();
  StringIMap<String>::iterator it = remap.find(function_name);
  if (it != remap.end()) {
    name = it->second;
  }
  const ClassInfo::MethodInfo *info = ClassInfo::FindFunction(name);
  if (info) {
    if (info->attribute & ClassInfo::IsSystem) return true;
    if (info->attribute & ClassInfo::IsVolatile) {
      return ((Globals *)get_global_variables())->function_exists(name);
    } else {
      return true;
    }
  } else {
    return false;
  }
}

void checkClassExists(CStrRef name, Globals *g, bool nothrow /* = false */) {
  if (g->class_exists(name)) return;
  if (function_exists(s___autoload)) {
    invoke(s___autoload, CREATE_VECTOR1(name), -1, true, false);
  }
  if (nothrow) return;
  if (!g->class_exists(name)) {
    string msg = "unknown class ";
    msg += name.c_str();
    throw_fatal(msg.c_str());
  }
}

bool checkClassExists(CStrRef name, const bool *declared, bool autoloadExists,
                      bool nothrow /* = false */) {
  if (declared && *declared) return true;
  if (autoloadExists) {
    invoke(s___autoload, CREATE_VECTOR1(name), -1, true, false);
  }
  if (declared && *declared) return true;
  if (nothrow) return false;
  string msg = "unknown class ";
  msg += name.c_str();
  throw_fatal(msg.c_str());
  return false;
}

bool checkInterfaceExists(CStrRef name, const bool *declared,
                          bool autoloadExists, bool nothrow /* = false */) {
  if (*declared) return true;
  if (autoloadExists) {
    invoke("__autoload", CREATE_VECTOR1(name), -1, true, false);
  }
  if (!*declared) {
    if (nothrow) return false;
    string msg = "unknown interface ";
    msg += name.c_str();
    throw_fatal(msg.c_str());
  }
  return true;
}

Variant &get_static_property_lval(const char *s, const char *prop) {
  Variant *ret = get_static_property_lv(s, prop);
  if (ret) return *ret;
  return Variant::lvalBlackHole();
}

Variant invoke_static_method_bind(CStrRef s, MethodIndex methodIndex,
                                  const char *method,
                                  const Array &params,
                                  bool fatal /* = true */) {
  ThreadInfo *info = ThreadInfo::s_threadInfo.get();

  String cls = s;
  bool isStatic = (strcasecmp(cls.data(), "static") == 0);
  if (isStatic) {
    cls = FrameInjection::GetStaticClassName(info);
  } else {
    FrameInjection::SetStaticClassName(info, cls);
  }
  Variant ret = invoke_static_method(cls.data(), methodIndex,
                                     method, params, fatal);
  if (!isStatic) {
    FrameInjection::ResetStaticClassName(info);
  }
  return ref(ret);
}

Variant invoke_static_method_bind_mil(CStrRef s,
                                      const char *method,
                                      const Array &params,
                                      bool fatal /* = true */) {
  MethodIndex methodIndex(MethodIndex::fail());
  if (RuntimeOption::FastMethodCall) {
    methodIndex = methodIndexExists(method);
    if (methodIndex.isFail()) {
      // do this here, where we still have the method name,
      // instead of invoke_builtin_static_method.
      if (fatal) {
        return throw_missing_class(s);
      } else {
        raise_warning("call_user_func to non-existent class's method %s::%s",
                      s.c_str(), method);
        return false;
      }
    }
  }
  return invoke_static_method_bind(s, methodIndex, method, params, fatal);

}

///////////////////////////////////////////////////////////////////////////////
}
