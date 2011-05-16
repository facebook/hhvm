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

#ifndef __HPHP_BUILTIN_FUNCTIONS_H__
#define __HPHP_BUILTIN_FUNCTIONS_H__

#include <runtime/base/execution_context.h>
#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/binary_operations.h>
#include <runtime/base/string_offset.h>
#include <runtime/base/frame_injection.h>
#include <runtime/base/intercept.h>
#include <runtime/base/runtime_error.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/taint/taint_data.h>
#include <runtime/base/taint/taint_observer.h>
#include <runtime/base/variable_unserializer.h>
#include <runtime/base/util/request_local.h>
#include <util/case_insensitive.h>

#ifdef __APPLE__
# ifdef isset
#  undef isset
# endif
#endif

/**
 * This file contains a list of functions that HPHP generates to wrap around
 * different expressions to maintain semantics. If we read through all types of
 * expressions in compiler/expression/expression.h, we can find most of them can be
 * directly transformed into C/C++ counterpart without too much syntactical
 * changes. The functions in this file happen to be the ones that are somewhat
 * special.
 *
 * Another way to think about this file is that this file has a list of C-style
 * functions, and the rest of run-time has object/classes for other tasks,
 * although we do have some global functions defined in other files as well,
 * when they are closer to the classes/objects in the same files.
 */

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// empty

inline bool empty(bool    v) { return !v;}
inline bool empty(char    v) { return !v;}
inline bool empty(short   v) { return !v;}
inline bool empty(int     v) { return !v;}
inline bool empty(int64   v) { return !v;}
inline bool empty(double  v) { return !v;}
inline bool empty(litstr  v) { return !v || !*v;}
inline bool empty(const StringData *v) { return v ? v->toBoolean() : false;}
inline bool empty(CStrRef v) { return !v.toBoolean();}
inline bool empty(CArrRef v) { return !v.toBoolean();}
inline bool empty(CObjRef v) { return !v.toBoolean();}
inline bool empty(CVarRef v) { return !v.toBoolean();}

bool empty(CVarRef v, bool    offset);
bool empty(CVarRef v, int64   offset);
inline bool empty(CVarRef v, int  offset) { return empty(v, (int64)offset); }
bool empty(CVarRef v, double  offset);
bool empty(CVarRef v, CArrRef offset);
bool empty(CVarRef v, CObjRef offset);
bool empty(CVarRef v, CVarRef offset);
bool empty(CVarRef v, litstr  offset, bool isString = false);
bool empty(CVarRef v, CStrRef offset, bool isString = false);

///////////////////////////////////////////////////////////////////////////////
// operators

/**
 * These functions are rarely performance bottlenecks, so we are not fully
 * type-specialized, although we could.
 */

inline bool logical_xor(bool v1, bool v2) { return (v1 ? 1:0) ^ (v2 ? 1:0);}
inline Variant bitwise_or (CVarRef v1, CVarRef v2) { return v1 | v2;}
inline Variant bitwise_and(CVarRef v1, CVarRef v2) { return v1 & v2;}
inline Variant bitwise_xor(CVarRef v1, CVarRef v2) { return v1 ^ v2;}
inline Numeric multiply(CVarRef v1, CVarRef v2)    { return v1 * v2;}
inline Variant plus(CVarRef v1, CVarRef v2)        { return v1 + v2;}
inline Numeric minus(CVarRef v1, CVarRef v2)       { return v1 - v2;}
inline Numeric divide(CVarRef v1, CVarRef v2)      { return v1 / v2; }
inline Numeric modulo(int64 v1, int64 v2) {
  if (abs(v2) == 1) {
    return 0;
  }
  if (v2 == 0) {
    raise_warning("Division by zero");
    return false;
  }
  return v1 % v2;
}
inline int64 shift_left(int64 v1, int64 v2)        { return v1 << v2; }
inline int64 shift_right(int64 v1, int64 v2)       { return v1 >> v2; }

inline char    negate(char v)    { return -v; }
inline short   negate(short v)   { return -v; }
inline int     negate(int v)     { return -v; }
inline int64   negate(int64 v)   { return -v; }
inline double  negate(double v)  { return -v; }
inline Variant negate(CVarRef v) { return -(Variant)v; }

inline String concat(CStrRef s1, CStrRef s2)         {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
  return s1 + s2;
}
inline String &concat_assign(String &s1, litstr s2)  {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
  return s1 += s2;
}
inline String &concat_assign(String &s1, CStrRef s2) {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
  return s1 += s2;
}

#define MAX_CONCAT_ARGS 6
String concat3(CStrRef s1, CStrRef s2, CStrRef s3);
String concat4(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4);
String concat5(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4, CStrRef s5);
String concat6(CStrRef s1, CStrRef s2, CStrRef s3, CStrRef s4, CStrRef s5,
               CStrRef s6);

inline Variant &concat_assign(Variant &v1, litstr s2) {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);

  if (v1.getType() == KindOfString) {
    StringData *data = v1.getStringData();
    if (data->getCount() == 1) {
      data->append(s2, strlen(s2));
      return v1;
    }
  }
  String s1 = v1.toString();
  s1 += s2;
  v1 = s1;
  return v1;
}

inline Variant &concat_assign(Variant &v1, CStrRef s2) {
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);

  if (v1.getType() == KindOfString) {
    StringData *data = v1.getStringData();
    if (data->getCount() == 1) {
      data->append(s2.data(), s2.size());
      return v1;
    }
  }
  String s1 = v1.toString();
  s1 += s2;

  v1 = s1;
  return v1;
}

inline String &concat_assign(const StringOffset &s1, litstr s2) {
  return concat_assign(s1.lval(), s2);
}

inline String &concat_assign(const StringOffset &s1, CStrRef s2) {
  return concat_assign(s1.lval(), s2);
}

inline bool instanceOf(ObjectData *v, CStrRef s) {
  return v && v->o_instanceof(s);
}

inline bool instanceOf(bool    v, CStrRef s) { return false;}
inline bool instanceOf(char    v, CStrRef s) { return false;}
inline bool instanceOf(short   v, CStrRef s) { return false;}
inline bool instanceOf(int     v, CStrRef s) { return false;}
inline bool instanceOf(int64   v, CStrRef s) { return false;}
inline bool instanceOf(double  v, CStrRef s) { return false;}
inline bool instanceOf(litstr  v, CStrRef s) { return false;}
inline bool instanceOf(CStrRef v, CStrRef s) { return false;}
inline bool instanceOf(CArrRef v, CStrRef s) { return false;}
inline bool instanceOf(CObjRef v, CStrRef s) { return instanceOf(v.get(), s);}
inline bool instanceOf(CVarRef v, CStrRef s) {
  return v.is(KindOfObject) && instanceOf(v.getObjectData(), s);
}

template <class K, class V>
const V &String::set(K key, const V &value) {
  StringData *s = StringData::Escalate(m_px);
  SmartPtr<StringData>::operator=(s);
  m_px->setChar(toInt32(key), toString(value));
  return value;
}

///////////////////////////////////////////////////////////////////////////////
// output functions

inline int print(const char *s) {
  g_context->write(s);
  return 1;
}
inline int print(CStrRef s) {
  // print is not a real function. x_print exists, but this function gets called
  // directly. We therefore need to setup the TaintObserver.
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
  g_context->write(s);
  return 1;
}
inline void echo(const char *s) {
  g_context->write(s);
}
inline void echo(CStrRef s) {
  // echo is not a real function. x_echo exists, but this function gets called
  // directly. We therefore need to setup the TaintObserver.
  TAINT_OBSERVER(TAINT_BIT_NONE, TAINT_BIT_NONE);
  g_context->write(s);
}

String get_source_filename(litstr path,bool dir_component = false);

void throw_exception(CObjRef e);
bool set_line(int line0, int char0 = 0, int line1 = 0, int char1 = 0);

///////////////////////////////////////////////////////////////////////////////
// isset/unset

inline bool isInitialized(CVarRef v) { return v.isInitialized();}
Variant getDynamicConstant(CVarRef v, CStrRef name);
String getUndefinedConstant(CStrRef name);

inline bool isset(CVarRef v) { return !v.isNull();}
inline bool isset(CObjRef v) { return !v.isNull();}

bool isset(CVarRef v, bool    offset);
bool isset(CVarRef v, int64   offset);
inline bool isset(CVarRef v, int  offset) { return isset(v, (int64)offset); }
bool isset(CVarRef v, double  offset);
bool isset(CVarRef v, CArrRef offset);
bool isset(CVarRef v, CObjRef offset);
bool isset(CVarRef v, CVarRef offset);
bool isset(CVarRef v, litstr  offset, bool isString = false);
bool isset(CVarRef v, CStrRef offset, bool isString = false);

inline Variant unset(Variant &v)               { v.unset();   return null;}
inline Variant unset(CVarRef v)                {              return null;}
inline Variant setNull(Variant &v)             { v.setNull(); return null;}
inline Variant unset(Object &v)                { v.reset();   return null;}
inline Variant unset(Array &v)                 { v.reset();   return null;}
///////////////////////////////////////////////////////////////////////////////
// special variable contexts

/**
 * lval() is mainly to make this work,
 *
 *   $arr['a']['b'] = $value;
 *
 * where $arr['a'] is in an l-value context. Note that lval() cannot replace
 * those offset classes, because calling these lval() functions will actually
 * insert a null value into an array/object, whereas an offset class can be
 * more powerful by not inserting a dummy value beforehand. For example,
 *
 *   isset($arr['a']); // we have to use offset's exists() function
 *   $obj['a'] = $value; // ArrayAccess's offset is completely customized
 *
 */
template<class T>
T &lval(T &v) { return v; }
inline Variant &lval(Variant &v) { return v;}
inline Array   &lval(Array   &v) { return v;}
inline Variant &lval(CVarRef  v) { // in case generating lval(1)
  throw FatalErrorException("taking reference from an r-value");
}
inline String  &lval(const StringOffset  &v) { return v.lval();}

template<class T>
Variant &unsetLval(Variant &v, const T &key) {
  if (v.isNull()) {
    return v;
  }
  if (v.is(KindOfArray)) {
    if (v.toArray().exists(key)) {
      return v.lvalAt(key);
    }
    return Variant::lvalBlackHole();
  }
  return Variant::lvalInvalid();
}

template<class T>
Variant &unsetLval(Array &v, const T &key) {
  if (!v.isNull() && v.exists(key)) {
    return v.lvalAt(key);
  }
  return Variant::lvalBlackHole();
}

///////////////////////////////////////////////////////////////////////////////
// misc functions

bool class_exists(CStrRef class_name, bool autoload = true);
String get_static_class_name(CVarRef objOrClassName);

Variant f_call_user_func_array(CVarRef function, CArrRef params,
                               bool bound = false);

/**
 * The MethodCallPackage does not hold the reference of the class name or
 * the method name. Therefore caller must provide the holders.
 */
bool get_user_func_handler(CVarRef function, MethodCallPackage& mcp,
                           String &classname, String &methodname);

Variant invoke(CStrRef function, CArrRef params, int64 hash = -1,
               bool tryInterp = true, bool fatal = true);
/**
 * Invoking an arbitrary static method.
 */
Variant invoke_static_method(CStrRef s, CStrRef method,
                             CArrRef params, bool fatal = true);

#ifdef ENABLE_LATE_STATIC_BINDING
/**
 * For "static::" resolution
 */
Variant invoke_static_method_bind(CStrRef s, CStrRef method,
                                  CArrRef params, bool fatal = true);
#endif

/**
 * Fallback when a dynamic function call fails to find a user function
 * matching the name.  If no handlers are able to
 * invoke the function, throw an InvalidFunctionCallException.
 */
Variant invoke_failed(const char *func, CArrRef params, int64 hash,
                      bool fatal = true);

Variant o_invoke_failed(const char *cls, const char *meth,
                        bool fatal = true);

Array collect_few_args(int count, INVOKE_FEW_ARGS_IMPL_ARGS);
Array collect_few_args_ref(int count, INVOKE_FEW_ARGS_IMPL_ARGS);

void get_call_info_or_fail(const CallInfo *&ci, void *&extra, CStrRef name);

/**
 * When fatal coding errors are transformed to this function call.
 */
inline Variant throw_fatal(const char *msg, void *dummy = NULL) {
  throw FatalErrorException(msg);
}
inline Variant throw_missing_class(const char *cls) {
  throw ClassNotFoundException((std::string("unknown class ") + cls).c_str());
}

inline Variant throw_missing_file(const char *cls) {
  throw PhpFileDoesNotExistException(cls);
}
void throw_instance_method_fatal(const char *name);

/**
 * Argument count handling.
 *   - When level is 2, it's from constructors that turn these into fatals
 *   - When level is 1, it's from system funcs that turn both into warnings
 *   - When level is 0, it's from user funcs that turn missing arg in warnings
 */
Variant throw_missing_arguments(const char *fn, int num, int level = 0);
Variant throw_toomany_arguments(const char *fn, int num, int level = 0);
Variant throw_wrong_arguments(const char *fn, int count, int cmin, int cmax,
                              int level = 0);
Variant throw_missing_typed_argument(const char *fn, const char *type, int arg);
Variant throw_assign_this();

/**
 * When fatal coding errors are transformed to this function call.
 */
inline Object throw_fatal_object(const char *msg, void *dummy = NULL) {
  throw FatalErrorException(msg);
}

void throw_unexpected_argument_type(int argNum, const char *fnName,
                                    const char *expected, CVarRef val);

/**
 * Handler for exceptions thrown from object destructors. Implemented in
 * program_functions.cpp.
 */
void handle_destructor_exception();

/**
 * If RuntimeOption::ThrowBadTypeExceptions is on, we are running in
 * a restrictive mode that's not compatible with PHP, and this will throw.
 * If RuntimeOption::ThrowBadTypeExceptions is off, we will log a
 * warning and swallow the error.
 */
void throw_bad_type_exception(const char *fmt, ...);
void throw_bad_array_exception();

/**
 * If RuntimeOption::ThrowInvalidArguments is on, we are running in
 * a restrictive mode that's not compatible with PHP, and this will throw.
 * If RuntimeOption::ThrowInvalidArguments is off, we will log a
 * warning and swallow the error.
 */
void throw_invalid_argument(const char *fmt, ...);

/**
 * Unsetting ClassName::StaticProperty.
 */
Variant throw_fatal_unset_static_property(const char *s, const char *prop);

/**
 * Exceptions injected code throws
 */
void throw_infinite_recursion_exception() ATTRIBUTE_COLD;
void generate_request_timeout_exception() ATTRIBUTE_COLD;
void generate_memory_exceeded_exception() ATTRIBUTE_COLD;
void throw_call_non_object() ATTRIBUTE_COLD __attribute__((noreturn));

/**
 * Cloning an object.
 */
Object f_clone(CVarRef v);

/**
 * Serialize/unserialize a variant into/from a string. We need these two
 * functions in runtime/base, as there are functions in runtime/base that depend on
 * these two functions.
 */
String f_serialize(CVarRef value);
Variant unserialize_ex(CStrRef str, VariableUnserializer::Type type);
inline Variant f_unserialize(CStrRef str) {
  return unserialize_ex(str, VariableUnserializer::Serialize);
}

class LVariableTable;
String resolve_include(CStrRef file, const char* currentDir,
                       bool (*tryFile)(CStrRef file, void* ctx), void* ctx);
Variant include(CStrRef file, bool once = false,
                LVariableTable* variables = NULL,
                const char *currentDir = "",
                bool raiseNotice = true);
Variant require(CStrRef file, bool once = false,
                LVariableTable* variables = NULL,
                const char *currentDir = "",
                bool raiseNotice = true);
Variant include_impl_invoke(CStrRef file, bool once = false,
                            LVariableTable* variables = NULL,
                            const char *currentDir = "");

/**
 * For wrapping expressions that have no effect, so to make gcc happy.
 */
inline bool    id(bool    v) { return v; }
inline char    id(char    v) { return v; }
inline short   id(short   v) { return v; }
inline int     id(int     v) { return v; }
inline int64   id(int64   v) { return v; }
inline uint64  id(uint64  v) { return v; }
inline double  id(double  v) { return v; }
inline litstr  id(litstr  v) { return v; }
inline CStrRef id(CStrRef v) { return v; }
inline CArrRef id(CArrRef v) { return v; }
inline CObjRef id(CObjRef v) { return v; }
inline CVarRef id(CVarRef v) { return v; }
template <class T>
inline const SmartObject<T> &id(const SmartObject<T> &v) { return v; }

/**
 * For wrapping return values to prevent elision of copy
 * constructors. This can be a problem if the function
 * returns by value, but a "referenced" variant is returned
 * through copy-constructor elision.
 */
inline Variant wrap_variant(CVarRef x) { return x; }

inline LVariableTable *lvar_ptr(const LVariableTable &vt) {
  return const_cast<LVariableTable*>(&vt);
}

bool function_exists(CStrRef function_name);

/**
 * For autoload support
 */
class Globals;

class AutoloadHandler : public RequestEventHandler {
public:
  virtual void requestInit();
  virtual void requestShutdown();

  void fiberInit(AutoloadHandler *handler, FiberReferenceMap &refMap);
  void fiberExit(AutoloadHandler *handler, FiberReferenceMap &refMap,
                 FiberAsyncFunc::Strategy default_strategy);

  CArrRef getHandlers() { return m_handlers; }
  bool addHandler(CVarRef handler, bool prepend);
  void removeHandler(CVarRef handler);
  void removeAllHandlers();
  bool isRunning();

  bool invokeHandler(CStrRef className, bool checkDeclared,
                     const bool *declared = NULL, bool autoloadExists = false);

  DECLARE_STATIC_REQUEST_LOCAL(AutoloadHandler, s_instance);

private:
  static String getSignature(CVarRef handler);

  Array m_handlers;
  bool m_running;
};

void checkClassExists(CStrRef name, Globals *g, bool nothrow = false);
bool checkClassExists(CStrRef name, const bool *declared, bool autoloadExists,
                      bool nothrow = false);
bool checkInterfaceExists(CStrRef name, const bool *declared,
                          bool autoloadExists, bool nothrow = false);

class CallInfo;

class MethodCallPackage {
public:
  MethodCallPackage()
  : ci(NULL), extra(NULL), isObj(false), obj(NULL),
    m_fatal(true), m_isFunc(false) {}

  // e->n() style method call
  void methodCall(CObjRef self, CStrRef method, int64 prehash = -1) {
    methodCall(self.objectForCall(), method, prehash);
  }
  void methodCall(ObjectData *self, CStrRef method, int64 prehash = -1);
  void methodCall(CVarRef self, CStrRef method, int64 prehash = -1);
  void methodCallWithIndex(CObjRef self, CStrRef method, MethodIndex mi,
                           int64 prehash = -1) {
    methodCallWithIndex(self.objectForCall(), method, mi, prehash);
  }
  void methodCallWithIndex(ObjectData *self, CStrRef method, MethodIndex mi,
                           int64 prehash = -1);
  void methodCallWithIndex(CVarRef self, CStrRef method, MethodIndex mi,
                           int64 prehash = -1);
  // K::n() style call, where K is a parent and n is not static and in an
  // instance method. Lookup is done outside since K is known.
  void methodCallEx(CObjRef self, CStrRef method) {
    isObj = true;
    rootObj = self.objectForCall();
    name = &method;
  }
  void methodCallEx(ObjectData *self, CStrRef method) {
    isObj = true;
    rootObj = self;
    name = &method;
  }
  // K::n() style call where K::n() is a static method. Lookup is done outside
  void staticMethodCall(CStrRef cname, CStrRef method) {
    rootCls = cname.get();
    name = &method;
  }
  // e::n() call. e could evaluate to be either a string or object.
  void dynamicNamedCall(CVarRef self, CStrRef method, int64 prehash = -1);
  void dynamicNamedCallWithIndex(CVarRef self, CStrRef method,
      MethodIndex mi, int64 prehash = -1);
  // e::n() call where e is definitely a string
  void dynamicNamedCall(CStrRef self, CStrRef method, int64 prehash = -1);
  void dynamicNamedCallWithIndex(CStrRef self, CStrRef method,
      MethodIndex mi, int64 prehash = -1);
  // function call
  void functionNamedCall(CStrRef func);
  // Get constructor
  void construct(CObjRef self);

  void noFatal() { m_fatal = false; }
  void fail();
  void lateStaticBind(ThreadInfo *ti);
  const CallInfo *bindClass(FrameInjection &fi);
  String getClassName();

  // Data members
  const CallInfo *ci;
  void *extra;
  bool isObj;
  union { // object or class name
    ObjectData *rootObj;
    StringData *rootCls;
  };
  ObjectData *obj;
  const String *name;
  bool m_fatal;
  bool m_isFunc;
};

class CallInfo {
public:
  enum Flags {
    VarArgs = 0x1,
    RefVarArgs = 0x2,
    Method = 0x4,
    StaticMethod = 0x8,
    CallMagicMethod = 0x10, // Special flag for __call handler
    MixedVarArgs = 0x20
  };
  CallInfo(void *inv, void *invFa, int ac, int flags, int64 refs)
    : m_invoker(inv), m_invokerFewArgs(invFa),
      m_argCount(ac), m_flags(flags), m_refFlags(refs) {}
  void *m_invoker;
  void *m_invokerFewArgs; // remove in time
  int m_argCount;
  int m_flags;
  int64 m_refFlags;
  // isRef() returns true if parameter n should be passed by reference.
  bool isRef(int n) const {
    // The RefVarArgs and MixedVarArgs flags cannot both be set
    ASSERT(!(m_flags & RefVarArgs) || !(m_flags & MixedVarArgs));
    bool res = (n < m_argCount ? (m_refFlags & (1 << n)) :
                                 (m_flags & (RefVarArgs | MixedVarArgs)));
    return res;
  }
  // mustBeRef() returns true if parameter n should MUST be passed by
  // reference. If a non-refable expression is supplied for parameter
  // then a fatal error should be raised.
  bool mustBeRef(int n) const {
    // The RefVarArgs and MixedVarArgs flags cannot both be set
    ASSERT(!(m_flags & RefVarArgs) || !(m_flags & MixedVarArgs));
    bool res = (n < m_argCount ? (m_refFlags & (1 << n)) :
                                 (m_flags & RefVarArgs));
    // If this function returns true for a given parameter, than isRef() must
    // also return true for this parameter.
    ASSERT(!res || isRef(n));
    return res;
  }
  typedef Variant (*FuncInvoker)(void*, CArrRef);
  typedef Variant (*FuncInvokerFewArgs)(void*, int,
      INVOKE_FEW_ARGS_IMPL_ARGS);
  FuncInvoker getFunc() const { return (FuncInvoker)m_invoker; }
  FuncInvokerFewArgs getFuncFewArgs() const {
    return (FuncInvokerFewArgs)m_invokerFewArgs;
  }
  typedef Variant (*FuncInvoker0Args)(
    void*, int);
  typedef Variant (*FuncInvoker1Args)(
    void*, int, CVarRef);
  typedef Variant (*FuncInvoker2Args)(
    void*, int, CVarRef, CVarRef);
  typedef Variant (*FuncInvoker3Args)(
    void*, int, CVarRef, CVarRef, CVarRef);
  typedef Variant (*FuncInvoker4Args)(
    void*, int, CVarRef, CVarRef, CVarRef,
    CVarRef);
  typedef Variant (*FuncInvoker5Args)(
    void*, int, CVarRef, CVarRef, CVarRef,
    CVarRef, CVarRef);
  typedef Variant (*FuncInvoker6Args)(
    void*, int, CVarRef, CVarRef, CVarRef,
    CVarRef, CVarRef, CVarRef);
  typedef Variant (*FuncInvoker7Args)(
    void*, int, CVarRef, CVarRef, CVarRef,
    CVarRef, CVarRef, CVarRef, CVarRef);
  typedef Variant (*FuncInvoker8Args)(
    void*, int, CVarRef, CVarRef, CVarRef,
    CVarRef, CVarRef, CVarRef, CVarRef, CVarRef);
  typedef Variant (*FuncInvoker9Args)(
    void*, int, CVarRef, CVarRef, CVarRef,
    CVarRef, CVarRef, CVarRef, CVarRef, CVarRef, CVarRef);
  typedef Variant (*FuncInvoker10Args)(
    void*, int, CVarRef, CVarRef, CVarRef,
    CVarRef, CVarRef, CVarRef, CVarRef, CVarRef, CVarRef, CVarRef);
  FuncInvoker0Args getFunc0Args() const {
    return (FuncInvoker0Args)m_invokerFewArgs;
  }
  FuncInvoker1Args getFunc1Args() const {
    return (FuncInvoker1Args)m_invokerFewArgs;
  }
  FuncInvoker2Args getFunc2Args() const {
    return (FuncInvoker2Args)m_invokerFewArgs;
  }
  FuncInvoker3Args getFunc3Args() const {
    return (FuncInvoker3Args)m_invokerFewArgs;
  }
  FuncInvoker4Args getFunc4Args() const {
    return (FuncInvoker4Args)m_invokerFewArgs;
  }
  FuncInvoker5Args getFunc5Args() const {
    return (FuncInvoker5Args)m_invokerFewArgs;
  }
  FuncInvoker6Args getFunc6Args() const {
    return (FuncInvoker6Args)m_invokerFewArgs;
  }
  FuncInvoker7Args getFunc7Args() const {
    return (FuncInvoker7Args)m_invokerFewArgs;
  }
  FuncInvoker8Args getFunc8Args() const {
    return (FuncInvoker8Args)m_invokerFewArgs;
  }
  FuncInvoker9Args getFunc9Args() const {
    return (FuncInvoker9Args)m_invokerFewArgs;
  }
  FuncInvoker10Args getFunc10Args() const {
    return (FuncInvoker10Args)m_invokerFewArgs;
  }

  typedef Variant (*MethInvoker)(MethodCallPackage &mcp, CArrRef);
  typedef Variant (*MethInvokerFewArgs)(
    MethodCallPackage &mcp, int, INVOKE_FEW_ARGS_IMPL_ARGS);

  MethInvoker getMeth() const {
    return (MethInvoker)m_invoker;
  }
  MethInvokerFewArgs getMethFewArgs() const {
    return (MethInvokerFewArgs)m_invokerFewArgs;
  }
  typedef Variant (*MethInvoker0Args)(
    MethodCallPackage &mcp, int);
  typedef Variant (*MethInvoker1Args)(
    MethodCallPackage &mcp, int, CVarRef);
  typedef Variant (*MethInvoker2Args)(
    MethodCallPackage &mcp, int, CVarRef, CVarRef);
  typedef Variant (*MethInvoker3Args)(
    MethodCallPackage &mcp, int, CVarRef, CVarRef, CVarRef);
  typedef Variant (*MethInvoker4Args)(
    MethodCallPackage &mcp, int, CVarRef, CVarRef, CVarRef,
    CVarRef);
  typedef Variant (*MethInvoker5Args)(
    MethodCallPackage &mcp, int, CVarRef, CVarRef, CVarRef,
    CVarRef, CVarRef);
  typedef Variant (*MethInvoker6Args)(
    MethodCallPackage &mcp, int, CVarRef, CVarRef, CVarRef,
    CVarRef, CVarRef, CVarRef);
  typedef Variant (*MethInvoker7Args)(
    MethodCallPackage &mcp, int, CVarRef, CVarRef, CVarRef,
    CVarRef, CVarRef, CVarRef, CVarRef);
  typedef Variant (*MethInvoker8Args)(
    MethodCallPackage &mcp, int, CVarRef, CVarRef, CVarRef,
    CVarRef, CVarRef, CVarRef, CVarRef, CVarRef);
  typedef Variant (*MethInvoker9Args)(
    MethodCallPackage &mcp, int, CVarRef, CVarRef, CVarRef,
    CVarRef, CVarRef, CVarRef, CVarRef, CVarRef, CVarRef);
  typedef Variant (*MethInvoker10Args)(
    MethodCallPackage &mcp, int, CVarRef, CVarRef, CVarRef,
    CVarRef, CVarRef, CVarRef, CVarRef, CVarRef, CVarRef, CVarRef);
  MethInvoker0Args getMeth0Args() const {
    return (MethInvoker0Args)m_invokerFewArgs;
  }
  MethInvoker1Args getMeth1Args() const {
    return (MethInvoker1Args)m_invokerFewArgs;
  }
  MethInvoker2Args getMeth2Args() const {
    return (MethInvoker2Args)m_invokerFewArgs;
  }
  MethInvoker3Args getMeth3Args() const {
    return (MethInvoker3Args)m_invokerFewArgs;
  }
  MethInvoker4Args getMeth4Args() const {
    return (MethInvoker4Args)m_invokerFewArgs;
  }
  MethInvoker5Args getMeth5Args() const {
    return (MethInvoker5Args)m_invokerFewArgs;
  }
  MethInvoker6Args getMeth6Args() const {
    return (MethInvoker6Args)m_invokerFewArgs;
  }
  MethInvoker7Args getMeth7Args() const {
    return (MethInvoker7Args)m_invokerFewArgs;
  }
  MethInvoker8Args getMeth8Args() const {
    return (MethInvoker8Args)m_invokerFewArgs;
  }
  MethInvoker9Args getMeth9Args() const {
    return (MethInvoker9Args)m_invokerFewArgs;
  }
  MethInvoker10Args getMeth10Args() const {
    return (MethInvoker10Args)m_invokerFewArgs;
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_BUILTIN_FUNCTIONS_H__
