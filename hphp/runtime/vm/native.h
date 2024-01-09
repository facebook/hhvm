/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#pragma once

#include "hphp/runtime/base/record-replay.h"
#include "hphp/runtime/base/recorder.h"
#include "hphp/runtime/base/replayer.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-nonnull-ret.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/util/abi-cxx.h"

#include <type_traits>

namespace HPHP {
struct ActRec;
struct Class;
struct FuncEmitter;
struct Object;
struct Extension;
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
 *   void moduleLoad(const IniSetting::Map& ini, Hdf config) override {
 *     HHVM_FE(sum);
 *   }
 *
 * To finish exposing it to PHP, add an entry to Systemlib
 * using matching hack typehints:
 *   <?hh
 *   <<__Native>>
 *   function sum(int $a, int $b): int;
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
 * Or an explicit call to registerNativeFunc()
 *   static const StaticString s_sum("sum");
 *   virtual moduleLoad(const IniSetting::Map& ini, Hdf config) {
 *     Native::registerNativeFunc(s_sum, (void*)my_sum_function);
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

#define REGISTER_NATIVE_FUNC(functable, name, f) do { \
  if (RO::EvalRecordReplay) { \
    if (RO::EvalRecordSampleRate) { \
      const auto wrapper{Recorder::wrapNativeFunc<f>(name)}; \
      Native::registerNativeFunc(functable, name, wrapper); \
      break; \
    } else if (RO::EvalReplay) { \
      const auto wrapper{Replayer::wrapNativeFunc<f>(name)}; \
      Native::registerNativeFunc(functable, name, wrapper); \
      break; \
    } \
  } \
  Native::registerNativeFunc(functable, name, f); \
} while(0)

#define HHVM_FN(fn) f_ ## fn
#define HHVM_FUNCTION(fn, ...) \
        HHVM_FN(fn)(__VA_ARGS__)
#define HHVM_NAMED_FE_STR(fn, fimpl, functable) \
        do { \
          String name{makeStaticString(fn)}; \
          registerExtensionFunction(name); \
          REGISTER_NATIVE_FUNC(functable, fn, fimpl); \
        } while(0)
#define HHVM_NAMED_FE(fn, fimpl)\
  HHVM_NAMED_FE_STR(#fn, fimpl, nativeFuncs())
#define HHVM_FE(fn) \
  HHVM_NAMED_FE_STR(#fn, HHVM_FN(fn), nativeFuncs())
#define HHVM_FALIAS(fn, falias)\
  HHVM_NAMED_FE_STR(#fn, HHVM_FN(falias), nativeFuncs())
#define HHVM_FALIAS_FE_STR(fn, falias)\
  HHVM_NAMED_FE_STR(fn, HHVM_FN(falias), nativeFuncs())

/* Macros related to declaring/registering internal implementations
 * of <<__Native>> class instance methods.
 *
 * See the definition of function macros above for general explanation.
 * These macros only differ in the following ways:
 * - They accept a classname in addition to the function name
 * - The registered name of the function is "ClassName->FunctionName"
 * - Prototypes include a prepended ObjectData* const parameter (named this_)
 */
#define HHVM_MN(cn,fn) c_ ## cn ## _ni_ ## fn
#define HHVM_METHOD(cn, fn, ...) \
        HHVM_MN(cn,fn)(ObjectData* const this_, ##__VA_ARGS__)
#define HHVM_NAMED_ME(cn,fn,mimpl) \
  REGISTER_NATIVE_FUNC(nativeFuncs(), #cn "->" #fn, mimpl)
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
#define HHVM_NAMED_STATIC_ME(cn,fn,mimpl) \
  REGISTER_NATIVE_FUNC(nativeFuncs(), #cn "::" #fn, mimpl)
#define HHVM_STATIC_ME(cn,fn) HHVM_NAMED_STATIC_ME(cn,fn,HHVM_STATIC_MN(cn,fn))
#define HHVM_STATIC_MALIAS(cn,fn,calias,falias) \
  HHVM_NAMED_STATIC_ME(cn,fn,HHVM_STATIC_MN(calias,falias))

/* Macros related to declaring/registering constants. Note that the
 * HHVM_RCC_* macros expect a StaticString to be present via s_##class_name.
 */
#define HHVM_RC_STR(const_name, const_value)                         \
  Native::registerConstant<KindOfString>(                            \
    makeStaticString(#const_name), makeStaticString(const_value));
#define HHVM_RC_INT(const_name, const_value)                         \
  Native::registerConstant<KindOfInt64>(                             \
    makeStaticString(#const_name), int64_t{const_value});
#define HHVM_RC_BOOL(const_name, const_value)                        \
  Native::registerConstant<KindOfBoolean>(                           \
    makeStaticString(#const_name), bool{const_value});

#define HHVM_RC_STR_SAME(const_name)                                 \
  Native::registerConstant<KindOfString>(                            \
    makeStaticString(#const_name), makeStaticString(const_name));
#define HHVM_RC_INT_SAME(const_name)                                 \
  Native::registerConstant<KindOfInt64>(                             \
    makeStaticString(#const_name), int64_t{const_name});
#define HHVM_RC_BOOL_SAME(const_name)                                \
  Native::registerConstant<KindOfBoolean>(                           \
    makeStaticString(#const_name), bool{const_name});

#define HHVM_RCC_STR(class_name, const_name, const_value)            \
  Native::registerClassConstant<KindOfString>(s_##class_name.get(),  \
    makeStaticString(#const_name), makeStaticString(const_value));
#define HHVM_RCC_INT(class_name, const_name, const_value)            \
  Native::registerClassConstant<KindOfInt64>(s_##class_name.get(),   \
    makeStaticString(#const_name), int64_t{const_value});
#define HHVM_RCC_DBL(class_name, const_name, const_value)            \
  Native::registerClassConstant<KindOfDouble>(s_##class_name.get(),  \
    makeStaticString(#const_name), double{const_value});
#define HHVM_RCC_BOOL(class_name, const_name, const_value)           \
  Native::registerClassConstant<KindOfBoolean>(s_##class_name.get(), \
    makeStaticString(#const_name), bool{const_value});

namespace HPHP { namespace Native {
//////////////////////////////////////////////////////////////////////////////

// Maximum number of args for a native function call
//
// To paraphrase you-know-who, "32 args should be enough for anybody"
//
// Note: If changing this number, update native-func-caller.h
// using make_native-func-caller.php
const int kMaxBuiltinArgs = 32;

inline int maxFCallBuiltinArgs() {
  return kMaxBuiltinArgs;
}

enum Attr {
  AttrNone         = 0,

  // Methods whose implementation is generated by HackC.
  AttrOpCodeImpl   = (1u << 0),
};

/**
 * Prepare function call arguments to match their expected
 * types on a native function/method call.
 *
 * Uses typehints in Func and tvCast*InPlace
 */
void coerceFCallArgsFromLocals(const ActRec* fp,
                               int32_t numArgs,
                               const Func* func);

#define NATIVE_TYPES                                  \
  /* kind       arg type              return type */  \
  X(Int64,      int64_t,              int64_t)        \
  X(Double,     double,               double)         \
  X(Bool,       bool,                 bool)           \
  X(Object,     const Object&,        Object)         \
  X(String,     const String&,        String)         \
  X(Array,      const Array&,         Array)          \
  X(Resource,   const OptResource&,   OptResource)    \
  X(Func,       Func*,                Func*)          \
  X(Class,      const Class*,         const Class*)   \
  X(ClsMeth,    ClsMethDataRef,       ClsMethDataRef) \
  X(Mixed,      const Variant&,       Variant)        \
  X(ObjectNN,   ObjectArg,            ObjectRet)      \
  X(StringNN,   StringArg,            StringRet)      \
  X(ArrayNN,    ArrayArg,             ArrayRet)       \
  X(ResourceArg,ResourceArg,          ResourceArg)    \
  X(MixedTV,    TypedValue,           TypedValue)     \
  X(This,       ObjectData*,          ObjectData*)    \
  X(Void,       void,                 void)           \
  X(IntIO,      int64_t&,             int64_t&)       \
  X(DoubleIO,   double&,              double&)        \
  X(BoolIO,     bool&,                bool&)          \
  X(ObjectIO,   Object&,              Object&)        \
  X(StringIO,   String&,              String&)        \
  X(ArrayIO,    Array&,               Array&)         \
  X(ResourceIO, OptResource&,         OptResource&)   \
  X(FuncIO,     Func*&,               Func*&)         \
  X(ClassIO,    Class*&,              Class*&)        \
  X(ClsMethIO,  ClsMethDataRef&,      ClsMethDataRef&)\
  X(MixedIO,    Variant&,             Variant&)       \
  /**/

template <class T>
struct NativeArg {
  /*
   * Delete move assignment operator to make this non-copyable.
   */
  NativeArg& operator=(NativeArg&&) = delete;
  T* operator->()        { return m_px; }
  T* get()               { return m_px; }
  bool operator!() const { return m_px == nullptr; }
  bool isNull() const    { return m_px == nullptr; }
private:
  /*
   * To be able to pass the values of this class as function parameters
   * by value, define default copy constructor (See Itanium C++ ABI p3.1.1).
   * Make it private to satisfy the non-copyable requirement.
   */
  NativeArg(const NativeArg&) = default;
  T* m_px;

  // These need to be friends so they can forward args to wrapped native funcs
  friend Recorder;
  friend Replayer;
};
}

using ObjectArg   = Native::NativeArg<ObjectData>;
using StringArg   = Native::NativeArg<StringData>;
using ArrayArg    = Native::NativeArg<ArrayData>;
using ResourceArg = Native::NativeArg<ResourceData>;

namespace Native {

struct NativeSig {
  enum class Type : uint8_t {
#define X(name, ...) name,
    NATIVE_TYPES
#undef X
  };

  NativeSig() : ret(Type::Void) {}

  NativeSig(Type ret, const std::vector<Type>& args)
      : ret(ret), args(args)
  {}

  NativeSig(Type ret, std::vector<Type>&& args)
    : ret(ret), args(std::move(args))
  {}

  NativeSig(NativeSig&&) = default;
  NativeSig(const NativeSig&) = default;

  NativeSig& operator=(const NativeSig&) = default;
  NativeSig& operator=(NativeSig&&) = default;

  template<class Ret>
  explicit NativeSig(Ret (*ptr)());

  template<class Ret, class... Args>
  explicit NativeSig(Ret (*ptr)(Args...));

  bool operator==(const NativeSig& other) const {
    return ret == other.ret && args == other.args;
  }

  std::string toString(const char* classname, const char* fname) const;

  Type ret;
  std::vector<Type> args;
};

namespace detail {

template<class T> struct native_arg_type {};
template<class T> struct native_ret_type {};
template<class T> struct known_native_arg : std::false_type {};

#define X(name, argTy, retTy)                                       \
  template<> struct native_ret_type<retTy>                          \
    : std::integral_constant<NativeSig::Type,NativeSig::Type::name> \
  {};                                                               \
  template<> struct native_arg_type<argTy>                          \
    : std::integral_constant<NativeSig::Type,NativeSig::Type::name> \
  {};                                                               \
  template<> struct known_native_arg<argTy> : std::true_type {};

NATIVE_TYPES

#undef X

template<class... Args> struct all_known_arg_type {};
template<class A>       struct all_known_arg_type<A> : known_native_arg<A> {};

template<class A, class... Rest>
struct all_known_arg_type<A,Rest...>
  : std::integral_constant<
      bool,
      known_native_arg<A>::value && all_known_arg_type<Rest...>::value
    >
{};

template<class T> struct understandable_sig          : std::false_type {};
template<class R> struct understandable_sig<R (*)()> : std::true_type {};
template<class R, class... Args>
struct understandable_sig<R (*)(Args...)>
  : all_known_arg_type<Args...>
{};

template<class... Args>
std::vector<NativeSig::Type> build_args() {
  return {
    native_arg_type<Args>::value...
  };
}

}

template <class Ret>
NativeSig::NativeSig(Ret (*/*ptr*/)())
    : ret(detail::native_ret_type<Ret>::value) {}

template <class Ret, class... Args>
NativeSig::NativeSig(Ret (*/*ptr*/)(Args...))
    : ret(detail::native_ret_type<Ret>::value),
      args(detail::build_args<Args...>()) {}

#undef NATIVE_TYPES

// NativeFunctionInfo carries around a NativeSig describing the real signature,
// and a type-erased NativeFunction ptr.
struct NativeFunctionInfo {
  NativeFunctionInfo() : ptr(nullptr) {}

  // generate the signature using template magic
  template<typename Func>
  explicit NativeFunctionInfo(Func f)
    : sig(f)
    , ptr(reinterpret_cast<NativeFunction>(f))
  {}

  // trust the given signature
  template<typename Func>
  NativeFunctionInfo(NativeSig sig, Func f)
    : sig(sig)
    , ptr(reinterpret_cast<NativeFunction>(f))
  {}

  explicit operator bool() const { return ptr != nullptr; }

  bool operator==(const NativeFunctionInfo& other) const {
    return ptr == other.ptr && sig == other.sig;
  }

  NativeSig sig;
  NativeFunction ptr;
};

/*
 * Known output types for inout parameters on builtins and optional default
 * values to be passed to builtins which use inout paramaters purely as out
 * values, ignoring their inputs.
 */
MaybeDataType builtinOutType(const TypeConstraint&, const UserAttributeMap&);
Optional<TypedValue> builtinInValue(const Func* builtin, uint32_t i);

/////////////////////////////////////////////////////////////////////////////

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
void getFunctionPointers(const NativeFunctionInfo& info,
                         int nativeAttrs,
                         ArFunction& bif,
                         NativeFunction& nif);

/////////////////////////////////////////////////////////////////////////////

/**
 * registerNativeFunc() and getNativeFunction() use a provided
 * FuncTable that is a case sensitive map of "name" to function pointer.
 * We require case-correct symbols for the purpose of binding native impls
 * to their PHP decls, regardless of how the language treats the symbols.
 *
 * Extensions should generally add items to this map using the HHVM_FE/ME
 * macros above. The function name (key) must be a static string.
 */

struct FuncTable;
void registerNativeFunc(FuncTable&, const StringData*,
                        const NativeFunctionInfo&);

// Helper accepting a C-string name
template <class Fun> typename
  std::enable_if<!std::is_member_function_pointer<Fun>::value, void>::type
registerNativeFunc(FuncTable& nativeFuncs, const char* name, Fun func) {
  static_assert(
    std::is_pointer<Fun>::value &&
    std::is_function<typename std::remove_pointer<Fun>::type>::value,
    "You can only register pointers to functions."
  );
  static_assert(
    detail::understandable_sig<Fun>::value,
    "Arguments on builtin function were not understood types"
  );
  registerNativeFunc(nativeFuncs, makeStaticString(name),
                     NativeFunctionInfo(func));
}

// Helper accepting a possibly nonstatic HPHP::String name
template <class Fun> typename
  std::enable_if<!std::is_member_function_pointer<Fun>::value, void>::type
registerNativeFunc(FuncTable& nativeFuncs, const String& name, Fun func) {
  static_assert(
    std::is_pointer<Fun>::value &&
    std::is_function<typename std::remove_pointer<Fun>::type>::value,
    "You can only register pointers to functions."
  );
  static_assert(
    detail::understandable_sig<Fun>::value,
    "Arguments on builtin function were not understood types"
  );
  registerNativeFunc(nativeFuncs, makeStaticString(name),
                     NativeFunctionInfo(func));
}

// Specializations of registerNativeFunc for taking pointers to member
// functions and making them look like HNI wrapper funcs.
//
// This allows invoking object method calls directly, but ONLY for specialized
// subclasses of ObjectData.
//
// This API is limited to: Closure, Asio, and Collections. Do not use it if
// you are not implementing one of these
template<class Ret, class Cls> typename
  std::enable_if<std::is_base_of<ObjectData, Cls>::value, void>::type
registerNativeFunc(FuncTable& nativeFuncs, const char* name,
                   Ret (Cls::*func)()) {
  registerNativeFunc(nativeFuncs, name,
                     (Ret (*)(ObjectData*))getMethodPtr(func));
}

template<class Ret, class Cls, class... Args> typename
  std::enable_if<std::is_base_of<ObjectData, Cls>::value, void>::type
registerNativeFunc(FuncTable& nativeFuncs, const char* name,
                   Ret (Cls::*func)(Args...)) {
  registerNativeFunc(
      nativeFuncs, name, (Ret (*)(ObjectData*, Args...))getMethodPtr(func)
  );
}

template<class Ret, class Cls> typename
  std::enable_if<std::is_base_of<ObjectData, Cls>::value, void>::type
registerNativeFunc(FuncTable& nativeFuncs, const char* name,
                          Ret (Cls::*func)() const) {
  registerNativeFunc(nativeFuncs, name,
                     (Ret (*)(ObjectData*))getMethodPtr(func));
}

template<class Ret, class Cls, class... Args> typename
  std::enable_if<std::is_base_of<ObjectData, Cls>::value, void>::type
registerNativeFunc(FuncTable& nativeFuncs, const char* name,
                   Ret (Cls::*func)(Args...) const) {
  registerNativeFunc(
      nativeFuncs, name, (Ret (*)(ObjectData*, Args...))getMethodPtr(func)
  );
}

/////////////////////////////////////////////////////////////////////////////

const char* checkTypeFunc(const NativeSig& sig,
                          const TypeConstraint& retType,
                          const FuncEmitter* func);

String fullName(const StringData* fname, const StringData* cname,
                bool isStatic);

NativeFunctionInfo getNativeFunction(const FuncTable& nativeFuncs,
                                     const StringData* fname,
                                     const StringData* cname = nullptr,
                                     bool isStatic = false);

NativeFunctionInfo getNativeFunction(const FuncTable& nativeFuncs,
                                     const char* fname,
                                     const char* cname = nullptr,
                                     bool isStatic = false);

//////////////////////////////////////////////////////////////////////////////
// Global constants

using ConstantMap = std::map<const StringData*,TypedValueAux>;
extern ConstantMap s_constant_map;

inline
bool registerConstant(const StringData* cnsName, TypedValue cns) {
  assertx(tvIsPlausible(cns) && cns.m_type != KindOfUninit);
  auto& dst = s_constant_map[cnsName];
  *static_cast<TypedValue*>(&dst) = cns;
  return bindPersistentCns(cnsName, cns);
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

using ClassConstantMapMap = hphp_hash_map<const StringData*, ConstantMap,
                      string_data_hash, string_data_tsame>;
extern ClassConstantMapMap s_class_constant_map;

inline
bool registerClassConstant(const StringData *clsName,
                           const StringData *cnsName,
                           TypedValue cns) {
  assertx(tvIsPlausible(cns));
  auto &cls = s_class_constant_map[clsName];
  assertx(cls.find(cnsName) == cls.end());
  *static_cast<TypedValue*>(&cls[cnsName]) = cns;
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

typedef void (*FinishFunc)(Class* cls);

void registerClassExtraDataHandler(const String& clsName, FinishFunc fn);

FinishFunc getClassExtraDataHandler(const StringData* clsName);

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Native
