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

#ifndef incl_HPHP_BUILTIN_FUNCTIONS_H_
#define incl_HPHP_BUILTIN_FUNCTIONS_H_

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/intercept.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/util/case-insensitive.h"
#include "hphp/runtime/base/type-conversions.h"

#if defined(__APPLE__) || defined(__USE_BSD)
/**
 * We don't actually use param.h in this file,
 * but other files which use us do, and we want
 * to enforce clearing of the isset macro from
 * that header by handling the header now
 * and wiping it out.
 */
# include <sys/param.h>
# ifdef isset
#  undef isset
# endif
#endif

/**
 * This file contains a list of functions that HPHP generates to wrap
 * around different expressions to maintain semantics. If we read
 * through all types of expressions in
 * compiler/expression/expression.h, we can find most of them can be
 * directly transformed into C/C++ counterpart without too much
 * syntactical changes. The functions in this file happen to be the
 * ones that are somewhat special.
 *
 * Another way to think about this file is that this file has a list of C-style
 * functions, and the rest of run-time has object/classes for other tasks,
 * although we do have some global functions defined in other files as well,
 * when they are closer to the classes/objects in the same files.
 */

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
extern const StaticString s_self;
extern const StaticString s_parent;
extern const StaticString s_static;

///////////////////////////////////////////////////////////////////////////////
// operators

inline String concat(const String& s1, const String& s2) {
  return s1 + s2;
}

String concat3(const String& s1, const String& s2, const String& s3);
String concat4(const String& s1, const String& s2, const String& s3,
               const String& s4);

///////////////////////////////////////////////////////////////////////////////
// output functions

inline void echo(const char *s) {
  g_context->write(s);
}
inline void echo(const char *s, int len) {
  g_context->write(s, len);
}
inline void echo(const String& s) {
  g_context->write(s);
}

void NEVER_INLINE throw_invalid_property_name(const String& name)
  ATTRIBUTE_NORETURN;
void NEVER_INLINE throw_null_object_prop();
void NEVER_INLINE throw_null_get_object_prop();
void NEVER_INLINE raise_null_object_prop();
void throw_exception(CObjRef e);

///////////////////////////////////////////////////////////////////////////////
// type testing

inline bool is_null(CVarRef v)   { return v.isNull();}
inline bool is_not_null(CVarRef v) { return !v.isNull();}
inline bool is_bool(CVarRef v)   { return v.is(KindOfBoolean);}
inline bool is_int(CVarRef v)    { return v.isInteger();}
inline bool is_double(CVarRef v) { return v.is(KindOfDouble);}
inline bool is_string(CVarRef v) { return v.isString();}
inline bool is_array(CVarRef v)  { return v.is(KindOfArray);}
inline bool is_object(CVarRef var) { return var.is(KindOfObject); }
inline bool is_empty_string(CVarRef v) {
  return v.isString() && v.getStringData()->empty();
}

///////////////////////////////////////////////////////////////////////////////
// misc functions

bool array_is_valid_callback(CArrRef arr);

Variant f_call_user_func_array(CVarRef function, CArrRef params,
                               bool bound = false);

const HPHP::Func*
vm_decode_function(CVarRef function,
                   ActRec* ar,
                   bool forwarding,
                   ObjectData*& this_,
                   HPHP::Class*& cls,
                   StringData*& invName,
                   bool warn = true);

inline void
vm_decode_function(CVarRef function,
                   ActRec* ar,
                   bool forwarding,
                   CallCtx& ctx,
                   bool warn = true) {
  ctx.func = vm_decode_function(function, ar, forwarding, ctx.this_, ctx.cls,
                                ctx.invName, warn);
}

ActRec* vm_get_previous_frame();
Variant vm_call_user_func(CVarRef function, CVarRef params,
                          bool forwarding = false);

/**
 * Invoking an arbitrary static method.
 */
Variant invoke_static_method(const String& s, const String& method,
                             CVarRef params, bool fatal = true);

/**
 * Fallback when a dynamic function call fails to find a user function
 * matching the name.  If no handlers are able to
 * invoke the function, throw an InvalidFunctionCallException.
 */
Variant invoke_failed(const char *func,
                      bool fatal = true);
Variant invoke_failed(CVarRef func,
                      bool fatal = true);

Variant o_invoke_failed(const char *cls, const char *meth,
                        bool fatal = true);

/**
 * When fatal coding errors are transformed to this function call.
 */
inline Variant throw_fatal(const char *msg, void *dummy = nullptr) {
  throw FatalErrorException(msg);
}
inline Variant throw_missing_class(const char *cls) {
  throw ClassNotFoundException((std::string("unknown class ") + cls).c_str());
}

inline Variant throw_missing_file(const char *file) {
  if (file[0] == '\0') {
    throw NoFileSpecifiedException();
  }
  throw PhpFileDoesNotExistException(file);
}
void throw_instance_method_fatal(const char *name);

void throw_iterator_not_valid() ATTRIBUTE_NORETURN;
void throw_collection_modified() ATTRIBUTE_NORETURN;
void throw_collection_property_exception() ATTRIBUTE_NORETURN;
void throw_collection_compare_exception() ATTRIBUTE_NORETURN;
void throw_param_is_not_container() ATTRIBUTE_NORETURN;
void check_collection_compare(ObjectData* obj);
void check_collection_compare(ObjectData* obj1, ObjectData* obj2);
void check_collection_cast_to_array();

Object create_object_only(const String& s);
Object create_object(const String& s, const Array &params, bool init = true);

inline bool isContainer(const Cell& c) {
  assert(cellIsPlausible(c));
  return c.m_type == KindOfArray ||
         (c.m_type == KindOfObject && c.m_data.pobj->isCollection());
}

inline bool isContainer(CVarRef v) {
  return isContainer(*v.asCell());
}

inline bool isContainerOrNull(const Cell& c) {
  assert(cellIsPlausible(c));
  return IS_NULL_TYPE(c.m_type) || c.m_type == KindOfArray ||
         (c.m_type == KindOfObject && c.m_data.pobj->isCollection());
}

inline bool isContainerOrNull(CVarRef v) {
  return isContainerOrNull(*v.asCell());
}

inline size_t getContainerSize(const Cell& c) {
  assert(isContainer(c));
  if (c.m_type == KindOfArray) {
    return c.m_data.parr->size();
  }
  assert(c.m_type == KindOfObject && c.m_data.pobj->isCollection());
  return c.m_data.pobj->getCollectionSize();
}

inline size_t getContainerSize(CVarRef v) {
  return getContainerSize(*v.asCell());
}

/**
 * Argument count handling.
 *   - When level is 2, it's from constructors that turn these into fatals
 *   - When level is 1, it's from system funcs that turn both into warnings
 *   - When level is 0, it's from user funcs that turn missing arg in warnings
 */
void throw_missing_arguments_nr(const char *fn, int expected, int got,
                                int level = 0, TypedValue *rv = nullptr)
  __attribute__((cold));
void throw_toomany_arguments_nr(const char *fn, int num, int level = 0,
                                TypedValue *rv = nullptr)
  __attribute__((cold));
void throw_wrong_arguments_nr(const char *fn, int count, int cmin, int cmax,
                              int level = 0, TypedValue *rv = nullptr)
  __attribute__((cold));

/**
 * Handler for exceptions thrown from user functions that we don't
 * allow exception propagation from.  E.g., object destructors or
 * certain callback hooks (user profiler). Implemented in
 * program-functions.cpp.
 */
void handle_destructor_exception(const char* situation = "Destructor");

/**
 * If RuntimeOption::ThrowBadTypeExceptions is on, we are running in
 * a restrictive mode that's not compatible with PHP, and this will throw.
 * If RuntimeOption::ThrowBadTypeExceptions is off, we will log a
 * warning and swallow the error.
 */
void throw_bad_type_exception(const char *fmt, ...) ATTRIBUTE_PRINTF(1,2);
void throw_expected_array_exception();
void throw_expected_array_or_collection_exception();

/**
 * If RuntimeOption::ThrowInvalidArguments is on, we are running in
 * a restrictive mode that's not compatible with PHP, and this will throw.
 * If RuntimeOption::ThrowInvalidArguments is off, we will log a
 * warning and swallow the error.
 */
void throw_invalid_argument(const char *fmt, ...) ATTRIBUTE_PRINTF(1,2);

/**
 * Unsetting ClassName::StaticProperty.
 */
Variant throw_fatal_unset_static_property(const char *s, const char *prop);

/**
 * Exceptions injected code throws
 */
void throw_infinite_recursion_exception();
Exception* generate_request_timeout_exception();
Exception* generate_memory_exceeded_exception();
void throw_call_non_object() ATTRIBUTE_NORETURN;
void throw_call_non_object(const char *methodName)
  ATTRIBUTE_NORETURN;

// unserializable default value arguments such as TimeStamp::Current()
// are serialized as "\x01"
char const kUnserializableString[] = "\x01";

/**
 * Serialize/unserialize a variant into/from a string. We need these
 * two functions in runtime/base, as there are functions in
 * runtime/base that depend on these two functions.
 */
String f_serialize(CVarRef value);
Variant unserialize_ex(const String& str,
                       VariableUnserializer::Type type,
                       CArrRef class_whitelist = null_array);
Variant unserialize_ex(const char* str, int len,
                       VariableUnserializer::Type type,
                       CArrRef class_whitelist = null_array);

inline Variant unserialize_from_buffer(const char* str, int len,
                                       CArrRef class_whitelist = null_array) {
  return unserialize_ex(str, len,
                        VariableUnserializer::Type::Serialize,
                        class_whitelist);
}

inline Variant unserialize_from_string(const String& str,
                                       CArrRef class_whitelist = null_array) {
  return unserialize_from_buffer(str.data(), str.size(), class_whitelist);
}

String resolve_include(const String& file, const char* currentDir,
                       bool (*tryFile)(const String& file, void* ctx),
                       void* ctx);
Variant include(const String& file, bool once = false,
                const char *currentDir = "",
                bool raiseNotice = true);
Variant require(const String& file, bool once = false,
                const char *currentDir = "",
                bool raiseNotice = true);
Variant include_impl_invoke(const String& file, bool once = false,
                            const char *currentDir = "");
Variant invoke_file(const String& file, bool once = false,
                    const char *currentDir = nullptr);
bool invoke_file_impl(Variant &res, const String& path, bool once,
                      const char *currentDir);

bool function_exists(const String& function_name);

/**
 * For autoload support
 */

class AutoloadHandler : public RequestEventHandler {
  enum Result {
    Failure,
    Success,
    StopAutoloading,
    ContinueAutoloading
  };

  struct HandlerBundle {
    HandlerBundle() = delete;
    HandlerBundle(CVarRef handler,
                  smart::unique_ptr<CufIter>& cufIter) :
      m_handler(handler) {
      m_cufIter = std::move(cufIter);
    }

    Variant m_handler; // used to respond to f_spl_autoload_functions
    smart::unique_ptr<CufIter> m_cufIter; // used to invoke handlers
  };

  class CompareBundles {
public:
    explicit CompareBundles(CufIter* cufIter) : m_cufIter(cufIter) { }
    bool operator()(const HandlerBundle& hb);
private:
    CufIter* m_cufIter;
  };

public:
  AutoloadHandler() { }

  ~AutoloadHandler() {
    m_map.detach();
    m_map_root.detach();
    // m_handlers won't run a destructor so nothing to do here
    m_loading.detach();
  }

  virtual void requestInit();
  virtual void requestShutdown();

  Array getHandlers();
  bool addHandler(CVarRef handler, bool prepend);
  void removeHandler(CVarRef handler);
  void removeAllHandlers();
  bool isRunning();

  bool invokeHandler(const String& className, bool forceSplStack = false);
  bool autoloadFunc(StringData* name);
  bool autoloadConstant(StringData* name);
  bool autoloadType(const String& name);
  bool setMap(CArrRef map, const String& root);
  DECLARE_STATIC_REQUEST_LOCAL(AutoloadHandler, s_instance);

private:
  template <class T>
  Result loadFromMap(const String& name, const String& kind, bool toLower,
                     const T &checkExists);
  static String getSignature(CVarRef handler);

  Array m_map;
  String m_map_root;
  bool m_spl_stack_inited;
  union {
    smart::deque<HandlerBundle> m_handlers;
  };
  Array m_loading;
};

#define CALL_USER_FUNC_FEW_ARGS_COUNT 6

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_BUILTIN_FUNCTIONS_H_
