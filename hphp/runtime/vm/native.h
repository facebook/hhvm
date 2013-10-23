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
#ifndef _incl_HPHP_RUNTIME_VM_NATIVE_H
#define _incl_HPHP_RUNTIME_VM_NATIVE_H

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/vm/func.h"
#include <type_traits>

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
 *   virtual moduleLoad(Hdf config) {
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
 *   virtual moduleLoad(Hdf config) {
 *     HHVM_NAME_FE(sum, my_sum_function)
 *   }
 * Or an explicit call to registerBuildinFunction()
 *   static const StaticString s_sum("sum");
 *   virtual moduleLoad(Hdf config) {
 *     Native::registerBuiltinFunction(s_sum.get(), (void*)my_sum_function);
 *   }
 */
#define HHVM_FN(fn) f_ ## fn
#define HHVM_FUNCTION(fn, ...) \
        HHVM_FN(fn)(__VA_ARGS__)
#define HHVM_NAMED_FE(fn, fimpl) Native::registerBuiltinFunction(\
                          makeStaticString(#fn), fimpl)
#define HHVM_FE(fn) HHVM_NAMED_FE(fn, HHVM_FN(fn))

/* Macros related to declaring/registering internal implementations
 * of <<__Native>> class instance methods.
 *
 * See the definition of function macros above for general explanation.
 * These macros only differ in the following ways:
 * - They accept a classname in addition to the function name
 * - The registered name of the function is "ClassName->FunctionName"
 * - Prototypes include a prepended CObjRef parameter (named this_)
 */
#define HHVM_MN(cn,fn) c_ ## cn ## _ni_ ## fn
#define HHVM_METHOD(cn, fn, ...) \
        HHVM_MN(cn,fn)(CObjRef this_, ##__VA_ARGS__)
#define HHVM_NAMED_ME(cn,fn,mimpl) Native::registerBuiltinFunction(\
                          makeStaticString(#cn "->" #fn), \
                          mimpl)
#define HHVM_ME(cn,fn) HHVM_NAMED_ME(cn,fn, HHVM_MN(cn,fn))

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

namespace HPHP { namespace Native {
//////////////////////////////////////////////////////////////////////////////

enum Attr {
  AttrNone = 0,
  AttrActRec = 1 << 0,
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
void callFunc(const Func* func, TypedValue *ctx,
              TypedValue* args, int32_t numArgs,
              TypedValue &ret);
/**
 * ActRec to native function call wrapper
 *
 * Unpacks args and coerces types according to Func typehints
 * Calls C++ function in the form ret f_foo(type arg1, type arg2, ...)
 * Marshalls return into TypedValue
 */
TypedValue* functionWrapper(ActRec* ar);

/**
 * Method version of nativeFunctionWrapper() above.
 *
 * Also prepends a calling context:
 *   CObjRef for instance methods
 *   Class* for static methods
 */
TypedValue* methodWrapper(ActRec* ar);

/**
 * Fallback method bound to declared methods with no matching
 * internal implementation.
 *
 * Throws a NotImplementedException
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

inline
bool registerConstant(const StringData* cnsName, Cell cns) {
  assert(cellIsPlausible(cns));
  return Unit::defCns(cnsName, &cns, true);
}

template<DataType DType>
typename std::enable_if<
  !std::is_same<typename detail::DataTypeCPPType<DType>::type,void>::value,
  bool>::type
registerConstant(const StringData* cnsName,
                 typename detail::DataTypeCPPType<DType>::type val) {
  return registerConstant(cnsName, make_tv<DType>(val));
}

template<DataType DType>
typename std::enable_if<
  std::is_same<typename detail::DataTypeCPPType<DType>::type,void>::value,
  bool>::type
registerConstant(const StringData* cnsName) {
  return registerConstant(cnsName, make_tv<DType>());
}

//////////////////////////////////////////////////////////////////////////////
// Class Constants

typedef std::map<const StringData*,TypedValue> ClassConstantMap;
typedef hphp_hash_map<const StringData*, ClassConstantMap,
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
  !std::is_same<typename detail::DataTypeCPPType<DType>::type,void>::value,
  bool>::type
registerClassConstant(const StringData* clsName,
                      const StringData* cnsName,
                      typename detail::DataTypeCPPType<DType>::type val) {
  return registerClassConstant(clsName, cnsName, make_tv<DType>(val));
}

template<DataType DType>
typename std::enable_if<
  std::is_same<typename detail::DataTypeCPPType<DType>::type,void>::value,
  bool>::type
registerClassConstant(const StringData* clsName,
                      const StringData* cnsName) {
  return registerClassConstant(clsName, cnsName, make_tv<DType>());
}

inline
const ClassConstantMap* getClassConstants(const StringData* clsName) {
  auto clsit = s_class_constant_map.find(const_cast<StringData*>(clsName));
  if (clsit == s_class_constant_map.end()) {
    return nullptr;
  }
  return &clsit->second;
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Native

#endif // _incl_HPHP_RUNTIME_VM_NATIVE_H
