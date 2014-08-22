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
#ifndef _incl_HPHP_RUNTIME_VM_NATIVE_H
#define _incl_HPHP_RUNTIME_VM_NATIVE_H

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/runtime/vm/func.h"

#include <type_traits>

namespace HPHP {
struct ActRec;
struct Class;
class Object;
};

/* Macros related to declaring/registering internal implementations
 * of <<__Native>> global functions.
 *
 * Declare a function in ext_foo.h using:
 *   ReturnType HHVM_FUNCTION(functionName, parameterList...)
 * For example:
 *   int64_t HHVM_FUNCTION(sum, int64_t a, int64_t b) {
 *     return a + b;
 *   }
 *
 * Then register it from your Extension's moduleLoad() hook:
 *   virtual moduleLoad(const IniSetting::Map& ini, Hdf config) {
 *     HHVM_FE(sum);
 *   }
 *
 * To finish exposing it to PHP, add an entry to Systemlib
 * using matching hack typehints:
 *   <?php
 *   <<__Native>>
 *   function sum(int $a, int $b): int;
 *
 ****************************************************************************
 *
 * For functions with variadic args or other "special" handling,
 * declare the prototype as a BuiltinFuncPtr:
 *   TypedValue* HHVM_FN(sum)(ActRec* ar) { ... }
 * And declare it in Systemlib with the "ActRec" subattribute
 *   <?php
 *   <<__Native("ActRec")>>
 *   function sum(int $a, int $b): int;
 * Registering the function in moduleLoad() remains the same.
 *
 ****************************************************************************
 *
 * If, for whatever reason, the standard declaration doesn't work,
 * you may declare the function directly as:
 *   ReturnType localSymbolName(parameterList...)
 * For example:
 *   int64_t my_sum_function(int64_t a, int64_t b) {
 *     return a + b;
 *   }
 *
 * In which case you will need to use a different macro in moduleLoad()
 *   virtual moduleLoad(const IniSetting::Map& ini, Hdf config) {
 *     HHVM_NAME_FE(sum, my_sum_function)
 *   }
 * Or an explicit call to registerBuildinFunction()
 *   static const StaticString s_sum("sum");
 *   virtual moduleLoad(const IniSetting::Map& ini, Hdf config) {
 *     Native::registerBuiltinFunction(s_sum.get(), (void*)my_sum_function);
 *   }
 *
 ****************************************************************************
 *
 * The macros HHVM_FALIAS, HHVM_MALIAS, and HHVM_STATIC_MALIAS allow
 * giving different names to the C++ implementation and the exported
 * C++ function. This ca be useful for creating multiple names for one
 * function or for registering functions that live in a namespace.
 *
 */
#define HHVM_FN(fn) f_ ## fn
#define HHVM_FUNCTION(fn, ...) \
        HHVM_FN(fn)(__VA_ARGS__)
#define HHVM_NAMED_FE(fn, fimpl) Native::registerBuiltinFunction(\
                          makeStaticString(#fn), fimpl)
#define HHVM_FE(fn) HHVM_NAMED_FE(fn, HHVM_FN(fn))
#define HHVM_FALIAS(fn, falias) HHVM_NAMED_FE(fn, HHVM_FN(falias))

/* Macros related to declaring/registering internal implementations
 * of <<__Native>> class instance methods.
 *
 * See the definition of function macros above for general explanation.
 * These macros only differ in the following ways:
 * - They accept a classname in addition to the function name
 * - The registered name of the function is "ClassName->FunctionName"
 * - Prototypes include a prepended const Object& parameter (named this_)
 */
#define HHVM_MN(cn,fn) c_ ## cn ## _ni_ ## fn
#define HHVM_METHOD(cn, fn, ...) \
        HHVM_MN(cn,fn)(ObjectData* const this_, ##__VA_ARGS__)
#define HHVM_NAMED_ME(cn,fn,mimpl) Native::registerBuiltinFunction(\
                          makeStaticString(#cn "->" #fn), \
                          mimpl)
#define HHVM_ME(cn,fn) HHVM_NAMED_ME(cn,fn, HHVM_MN(cn,fn))
#define HHVM_MALIAS(cn,fn,calias,falias) \
  HHVM_NAMED_ME(cn,fn,HHVM_MN(calias,falias))

/* Macros related to declaring/registering internal implementations
 * of <<__Native>> class static methods.
 *
 * See the definition of function macros above for general explanation.
 * These macros only differ in the following ways:
 * - They accept a classname in addition to the function name
 * - The registered name of the function is "ClassName::FunctionName"
 * - Prototypes include a prepended const Class* parameter (named self_)
 */
#define HHVM_STATIC_MN(cn,fn) c_ ## cn ## _ns_ ## fn
#define HHVM_STATIC_METHOD(cn, fn, ...) \
        HHVM_STATIC_MN(cn,fn)(const Class *self_, ##__VA_ARGS__)
#define HHVM_NAMED_STATIC_ME(cn,fn,mimpl) Native::registerBuiltinFunction(\
                          makeStaticString(#cn "::" #fn), \
                          mimpl)
#define HHVM_STATIC_ME(cn,fn) HHVM_NAMED_STATIC_ME(cn,fn,HHVM_STATIC_MN(cn,fn))
#define HHVM_STATIC_MALIAS(cn,fn,calias,falias) \
  HHVM_NAMED_STATIC_ME(cn,fn,HHVM_STATIC_MN(calias,falias))

namespace HPHP { namespace Native {
//////////////////////////////////////////////////////////////////////////////

// Maximum number of args for a native function call
// Beyond this number, a function/method will have to
// take a raw ActRec (using <<__Native("ActRec")>>) and
// deal with the args using getArg<KindOf*>(ar, argNum)
//
// To paraphrase you-know-who, "32 args should be enough for anybody"
//
// Note: If changing this number, update native-func-caller.h
// using make_native-func-caller.php
const int kMaxBuiltinArgs = 32;

// t#3982283 - Our ARM code gen doesn't support stack args yet.
// In fact, it only supports six of the eight register args.
// Put a hard limit of five to account for the return register for now.
constexpr int kMaxFCallBuiltinArgsARM = 5;

inline int maxFCallBuiltinArgs() {
#ifdef __AARCH64EL__
  return kMaxFCallBuiltinArgsARM;
#else
  if (UNLIKELY(RuntimeOption::EvalSimulateARM)) {
    return kMaxFCallBuiltinArgsARM;
  }
  return kMaxBuiltinArgs;
#endif
}

// t#3982283 - Our ARM code gen doesn't support FP args/returns yet.
inline bool allowFCallBuiltinDoubles() {
#ifdef __AARCH64EL__
  return false;
#else
  if (UNLIKELY(RuntimeOption::EvalSimulateARM)) {
    return false;
  }
  return true;
#endif
}

enum Attr {
  AttrNone = 0,
  AttrActRec = 1 << 0,
  AttrZendCompat = 1 << 1,
};

/**
 * Prepare function call arguments to match their expected
 * types on a native function/method call.
 *
 * Uses typehints in Func and tvCast*InPlace
 */
bool coerceFCallArgs(TypedValue* args,
                     int32_t numArgs, int32_t numNonDefault,
                     const Func* func);

/**
 * Dispatches a call to the native function bound to <func>
 * If <ctx> is not nullptr, it is prepended to <args> when
 * calling.
 */
template<bool usesDoubles, bool variadic>
void callFunc(const Func* func, void* ctx,
              TypedValue* args, TypedValue& ret);

/**
 * Returns a specialization of either functionWrapper or methodWrapper
 *
 * functionWrapper() Unpacks args and coerces types according
 * to Func typehints. Calls C++ function in the form:
 * ret f_foo(type arg1, type arg2, ...)
 * Marshalls return into TypedValue.
 *
 * methodWrapper() behaves the same as functionWrapper(),
 * but also prepends either an ObjectData* (instance) or Class* (static)
 * argument to the signature. i.e.:
 * ret c_class_ni_method(ObjectData* this_, type arg1, type arg2, ...)
 * ret c_class_ns_method(Class* self_, type arg1, type arg2, ...)
 */
BuiltinFunction getWrapper(bool method, bool usesDoubles, bool variadic);

/**
 * Fallback method bound to declared methods with no matching
 * internal implementation.
 */
TypedValue* unimplementedWrapper(ActRec* ar);

/**
 * Case insensitive map of "name" to function pointer
 *
 * Extensions should generally add items to this map using
 * the HHVM_FE/ME macros above
 */
typedef hphp_hash_map<const StringData*, BuiltinFunction,
                      string_data_hash, string_data_isame> BuiltinFunctionMap;

extern BuiltinFunctionMap s_builtinFunctions;

template <class Fun>
inline void registerBuiltinFunction(const StringData* fname, Fun func) {
  static_assert(
    std::is_pointer<Fun>::value &&
    std::is_function<typename std::remove_pointer<Fun>::type>::value,
    "You can only register pointers to function.");
  s_builtinFunctions[fname] = (BuiltinFunction)func;
}

inline BuiltinFunction GetBuiltinFunction(const StringData* fname,
                                          const StringData* cname = nullptr,
                                          bool isStatic = false) {
  auto it = s_builtinFunctions.find((cname == nullptr) ? fname :
                    (String(const_cast<StringData*>(cname)) +
                    (isStatic ? "::" : "->") +
                     String(const_cast<StringData*>(fname))).get());
  return (it == s_builtinFunctions.end()) ? nullptr : it->second;
}

inline BuiltinFunction GetBuiltinFunction(const char* fname,
                                          const char* cname = nullptr,
                                          bool isStatic = false) {
  return GetBuiltinFunction(makeStaticString(fname),
                    cname ? makeStaticString(cname) : nullptr,
                    isStatic);
}

//////////////////////////////////////////////////////////////////////////////
// Global constants

typedef std::map<const StringData*,TypedValue> ConstantMap;
extern ConstantMap s_constant_map;

inline
bool registerConstant(const StringData* cnsName, Cell cns) {
  assert(cellIsPlausible(cns));
  s_constant_map[cnsName] = cns;
  return Unit::defCns(cnsName, &cns, true);
}

template<DataType DType>
typename std::enable_if<
  !std::is_same<typename DataTypeCPPType<DType>::type,void>::value,
  bool>::type
registerConstant(const StringData* cnsName,
                 typename DataTypeCPPType<DType>::type val) {
  return registerConstant(cnsName, make_tv<DType>(val));
}

template<DataType DType>
typename std::enable_if<
  std::is_same<typename DataTypeCPPType<DType>::type,void>::value,
  bool>::type
registerConstant(const StringData* cnsName) {
  return registerConstant(cnsName, make_tv<DType>());
}

inline
const ConstantMap& getConstants() {
  return s_constant_map;
}

//////////////////////////////////////////////////////////////////////////////
// Class Constants

typedef hphp_hash_map<const StringData*, ConstantMap,
                      string_data_hash, string_data_isame> ClassConstantMapMap;
extern ClassConstantMapMap s_class_constant_map;

inline
bool registerClassConstant(const StringData *clsName,
                           const StringData *cnsName,
                           Cell cns) {
  assert(cellIsPlausible(cns));
  auto &cls = s_class_constant_map[clsName];
  assert(cls.find(cnsName) == cls.end());
  cls[cnsName] = cns;
  return true;
}

template<DataType DType>
typename std::enable_if<
  !std::is_same<typename DataTypeCPPType<DType>::type,void>::value,
  bool>::type
registerClassConstant(const StringData* clsName,
                      const StringData* cnsName,
                      typename DataTypeCPPType<DType>::type val) {
  return registerClassConstant(clsName, cnsName, make_tv<DType>(val));
}

template<DataType DType>
typename std::enable_if<
  std::is_same<typename DataTypeCPPType<DType>::type,void>::value,
  bool>::type
registerClassConstant(const StringData* clsName,
                      const StringData* cnsName) {
  return registerClassConstant(clsName, cnsName, make_tv<DType>());
}

inline
const ConstantMap* getClassConstants(const StringData* clsName) {
  auto clsit = s_class_constant_map.find(const_cast<StringData*>(clsName));
  if (clsit == s_class_constant_map.end()) {
    return nullptr;
  }
  return &clsit->second;
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Native

#endif // _incl_HPHP_RUNTIME_VM_NATIVE_H
