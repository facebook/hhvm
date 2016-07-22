/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_RUNTIME_VM_NATIVE_H
#define incl_HPHP_RUNTIME_VM_NATIVE_H

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/tv-helpers.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/util/abi-cxx.h"

#include <type_traits>

namespace HPHP {
struct ActRec;
struct Class;
struct FuncEmitter;
struct Object;
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
 *     Native::registerBuiltinFunction(s_sum, (void*)my_sum_function);
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
#define HHVM_NAMED_FE_STR(fn, fimpl) \
        do { \
          /* This calls Extension::registerExtensionFunction() on the */ \
          /* local 'this' at the call site. */ \
          String name{makeStaticString(fn)}; \
          registerExtensionFunction(name); \
          Native::registerBuiltinFunction(name, fimpl); \
        } while(0)
#define HHVM_NAMED_FE(fn, fimpl) HHVM_NAMED_FE_STR(#fn, fimpl)
#define HHVM_FE(fn) HHVM_NAMED_FE_STR(#fn, HHVM_FN(fn))
#define HHVM_FALIAS(fn, falias) HHVM_NAMED_FE_STR(#fn, HHVM_FN(falias))

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
        Native::registerBuiltinFunction(#cn "->" #fn, mimpl)
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
        Native::registerBuiltinFunction(#cn "::" #fn, mimpl)
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
    makeStaticString(#const_name), int64_t(const_value));
#define HHVM_RC_DBL(const_name, const_value)                         \
  Native::registerConstant<KindOfDouble>(                            \
    makeStaticString(#const_name), double(const_value));
#define HHVM_RC_BOOL(const_name, const_value)                        \
  Native::registerConstant<KindOfBoolean>(                           \
    makeStaticString(#const_name), bool(const_value));

#define HHVM_RC_STR_SAME(const_name)                                 \
  Native::registerConstant<KindOfString>(                            \
    makeStaticString(#const_name), makeStaticString(const_name));
#define HHVM_RC_INT_SAME(const_name)                                 \
  Native::registerConstant<KindOfInt64>(                             \
    makeStaticString(#const_name), int64_t(const_name));
#define HHVM_RC_DBL_SAME(const_name)                                 \
  Native::registerConstant<KindOfDouble>(                            \
    makeStaticString(#const_name), double(const_name));
#define HHVM_RC_BOOL_SAME(const_name)                                \
  Native::registerConstant<KindOfBoolean>(                           \
    makeStaticString(#const_name), bool(const_name));

#define HHVM_RCC_STR(class_name, const_name, const_value)            \
  Native::registerClassConstant<KindOfString>(s_##class_name.get(),  \
    makeStaticString(#const_name), makeStaticString(const_value));
#define HHVM_RCC_INT(class_name, const_name, const_value)            \
  Native::registerClassConstant<KindOfInt64>(s_##class_name.get(),   \
    makeStaticString(#const_name), int64_t(const_value));
#define HHVM_RCC_DBL(class_name, const_name, const_value)            \
  Native::registerClassConstant<KindOfDouble>(s_##class_name.get(),  \
    makeStaticString(#const_name), double(const_value));
#define HHVM_RCC_BOOL(class_name, const_name, const_value)           \
  Native::registerClassConstant<KindOfBoolean>(s_##class_name.get(), \
    makeStaticString(#const_name), bool(const_value));

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

inline int maxFCallBuiltinArgs() {
  return kMaxBuiltinArgs;
}

inline bool allowFCallBuiltinDoubles() {
  return true;
}

enum Attr {
  AttrNone = 0,
  AttrActRec = 1 << 0,
  AttrZendCompat = 1 << 1,
  AttrOpCodeImpl = 1 << 2, //Methods whose implementation is in the emitter
};

/**
 * Prepare function call arguments to match their expected
 * types on a native function/method call.
 *
 * Uses typehints in Func and tvCast*InPlace
 */
bool coerceFCallArgs(TypedValue* args,
                     int32_t numArgs, int32_t numNonDefault,
                     const Func* func, bool useStrictTypes);

/**
 * Dispatches a call to the native function bound to <func>
 * If <ctx> is not nullptr, it is prepended to <args> when
 * calling.
 */
template<bool usesDoubles>
void callFunc(const Func* func, void* ctx,
              TypedValue* args, int32_t numNonDefault,
              TypedValue& ret);

/**
 * Extract the name used to invoke the function from the ActRec where name
 * maybe be stored in invName, or may include the classname (e.g. Class::func)
 */
const StringData* getInvokeName(ActRec* ar);

#define NATIVE_TYPES                                \
  /* kind       arg type              return type */  \
  X(Int32,      int32_t,              int32_t)        \
  X(Int64,      int64_t,              int64_t)        \
  X(Double,     double,               double)         \
  X(Bool,       bool,                 bool)           \
  X(Object,     const Object&,        Object)         \
  X(String,     const String&,        String)         \
  X(Array,      const Array&,         Array)          \
  X(Resource,   const Resource&,      Resource)       \
  X(Mixed,      const Variant&,       Variant)        \
  X(ObjectArg,  ObjectArg,            ObjectArg)      \
  X(StringArg,  StringArg,            StringArg)      \
  X(ArrayArg,   ArrayArg,             ArrayArg)       \
  X(ResourceArg,ResourceArg,          ResourceArg)    \
  X(OutputArg,  OutputArg,            OutputArg)      \
  X(ARReturn,   TypedValue*,          TypedValue*)    \
  X(MixedTV,    TypedValue,           TypedValue)     \
  X(MixedRef,   const VRefParamValue&,VRefParamValue) \
  X(VarArgs,    ActRec*,              ActRec*)        \
  X(This,       ObjectData*,          ObjectData*)    \
  X(Class,      const Class*,         const Class*)   \
  X(Void,       void,                 void)           \
  X(Zend,       ZendFuncType,         ZendFuncType)   \
  /**/

enum class ZendFuncType {};

template <class T>
struct NativeArg {
  /*
   * If we define, or delete a copy/move constructor,
   * the class will be passed by address, which would
   * defeat the point.
   * However, deleting a move assignment operator causes
   * the copy constructor to be implicitly deleted, and
   * its still passed in registers. tada!
   */
  NativeArg& operator=(NativeArg&&) = delete;
  T* operator->()        { return m_px; }
  T* get()               { return m_px; }
  bool operator!() const { return m_px == nullptr; }
  bool isNull() const    { return m_px == nullptr; }
private:
  T* m_px;
};
}

using ObjectArg   = Native::NativeArg<ObjectData>;
using StringArg   = Native::NativeArg<StringData>;
using ArrayArg    = Native::NativeArg<ArrayData>;
using ResourceArg = Native::NativeArg<ResourceData>;
using OutputArg   = Native::NativeArg<RefData>;

namespace Native {

struct NativeSig {
  enum class Type : uint8_t {
#define X(name, ...) name,
    NATIVE_TYPES
#undef X
  };

  explicit NativeSig(ZendFuncType) : ret(Type::Zend) {}
  NativeSig() : ret(Type::Void) {}

  template<class Ret>
  explicit NativeSig(Ret (*ptr)());

  template<class Ret, class... Args>
  explicit NativeSig(Ret (*ptr)(Args...));

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

template<class Ret>
NativeSig::NativeSig(Ret (*ptr)())
  : ret(detail::native_ret_type<Ret>::value)
{}

template<class Ret, class... Args>
NativeSig::NativeSig(Ret (*ptr)(Args...))
  : ret(detail::native_ret_type<Ret>::value)
  , args(detail::build_args<Args...>())
{}

#undef NATIVE_TYPES

struct BuiltinFunctionInfo {
  BuiltinFunctionInfo() : ptr(nullptr) {}

  template<typename Func>
  explicit BuiltinFunctionInfo(Func f)
    : sig(f)
    , ptr((BuiltinFunction)f)
  {}

  NativeSig sig;
  BuiltinFunction ptr;
};

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
void getFunctionPointers(const BuiltinFunctionInfo& info,
                         int nativeAttrs,
                         BuiltinFunction& bif,
                         BuiltinFunction& nif);

/**
 * Fallback method bound to declared methods with no matching
 * internal implementation.
 */
TypedValue* unimplementedWrapper(ActRec* ar);

/////////////////////////////////////////////////////////////////////////////

/**
 * Case insensitive map of "name" to function pointer
 *
 * Extensions should generally add items to this map using
 * the HHVM_FE/ME macros above. The function name (key) must
 * be a static string because this table is shared and outlives
 * individual requests.
 */
typedef hphp_hash_map<const StringData*, BuiltinFunctionInfo,
                      string_data_hash, string_data_isame> BuiltinFunctionMap;

extern BuiltinFunctionMap s_builtinFunctions;

template <class Fun> typename
  std::enable_if<!std::is_member_function_pointer<Fun>::value, void>::type
registerBuiltinFunction(const char* name, Fun func) {
  static_assert(
    std::is_pointer<Fun>::value &&
    std::is_function<typename std::remove_pointer<Fun>::type>::value,
    "You can only register pointers to functions."
  );
  static_assert(
    detail::understandable_sig<Fun>::value,
    "Arguments on builtin function were not understood types"
  );
  s_builtinFunctions[makeStaticString(name)] = BuiltinFunctionInfo(func);
}

template <class Fun> typename
  std::enable_if<!std::is_member_function_pointer<Fun>::value, void>::type
registerBuiltinFunction(const String& name, Fun func) {
  static_assert(
    std::is_pointer<Fun>::value &&
    std::is_function<typename std::remove_pointer<Fun>::type>::value,
    "You can only register pointers to functions."
  );
  static_assert(
    detail::understandable_sig<Fun>::value,
    "Arguments on builtin function were not understood types"
  );
  s_builtinFunctions[makeStaticString(name)] = BuiltinFunctionInfo(func);
}

template <class Fun> typename
  std::enable_if<!std::is_member_function_pointer<Fun>::value, void>::type
registerBuiltinZendFunction(const char* name, Fun func) {
  static_assert(
    std::is_pointer<Fun>::value &&
    std::is_function<typename std::remove_pointer<Fun>::type>::value,
    "You can only register pointers to functions."
  );
  auto bfi = BuiltinFunctionInfo();
  bfi.ptr = (BuiltinFunction)func;
  bfi.sig = NativeSig(ZendFuncType{});
  s_builtinFunctions[makeStaticString(name)] = bfi;
}

template <class Fun> typename
  std::enable_if<!std::is_member_function_pointer<Fun>::value, void>::type
registerBuiltinZendFunction(const String& name, Fun func) {
  static_assert(
    std::is_pointer<Fun>::value &&
    std::is_function<typename std::remove_pointer<Fun>::type>::value,
    "You can only register pointers to function."
  );
  auto bfi = BuiltinFunctionInfo();
  bfi.ptr = (BuiltinFunction)func;
  bfi.sig = NativeSig(ZendFuncType{});
  s_builtinFunctions[makeStaticString(name)] = bfi;
}

// Specializations of registerBuiltinFunction for taking
// Pointers to Member Functions and making them look like HNI wrapper funcs
//
// This allows invoking object method calls directly,
// but ONLY for specialized subclasses of ObjectData.
//
// This API is limited to: Closure, Asio, and Collections
// Do not use it if you are not implementing one of these
template<class Ret, class Cls> typename
  std::enable_if<std::is_base_of<ObjectData, Cls>::value, void>::type
registerBuiltinFunction(const char* name, Ret (Cls::*func)()) {
  registerBuiltinFunction(name,
                          (Ret (*)(ObjectData*))getMethodPtr(func));
}

template<class Ret, class Cls, class... Args> typename
  std::enable_if<std::is_base_of<ObjectData, Cls>::value, void>::type
registerBuiltinFunction(const char* name, Ret (Cls::*func)(Args...)) {
  registerBuiltinFunction(name,
                          (Ret (*)(ObjectData*, Args...))getMethodPtr(func));
}

template<class Ret, class Cls> typename
  std::enable_if<std::is_base_of<ObjectData, Cls>::value, void>::type
registerBuiltinFunction(const char* name, Ret (Cls::*func)() const) {
  registerBuiltinFunction(name,
                          (Ret (*)(ObjectData*))getMethodPtr(func));
}
template<class Ret, class Cls, class... Args> typename
  std::enable_if<std::is_base_of<ObjectData, Cls>::value, void>::type
registerBuiltinFunction(const char* name, Ret (Cls::*func)(Args...) const) {
  registerBuiltinFunction(name,
                          (Ret (*)(ObjectData*, Args...))getMethodPtr(func));
}

/////////////////////////////////////////////////////////////////////////////

const char* checkTypeFunc(const NativeSig& sig,
                          const TypeConstraint& retType,
                          const Func* func);

inline BuiltinFunctionInfo GetBuiltinFunction(const StringData* fname,
                                              const StringData* cname = nullptr,
                                              bool isStatic = false) {
  auto it = s_builtinFunctions.find((cname == nullptr) ? fname :
                    (String(const_cast<StringData*>(cname)) +
                    (isStatic ? "::" : "->") +
                     String(const_cast<StringData*>(fname))).get());
  return (it == s_builtinFunctions.end()) ? BuiltinFunctionInfo() : it->second;
}

inline BuiltinFunctionInfo GetBuiltinFunction(const char* fname,
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

using NativeConstantCallback = const Variant& (*)();
bool registerConstant(const StringData*, NativeConstantCallback);

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

#endif
