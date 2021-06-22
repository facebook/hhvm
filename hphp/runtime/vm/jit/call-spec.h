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

#include "hphp/runtime/ext/asio/asio-blockable.h"

#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/record-data.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"

#include <array>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace HPHP {

struct ActRec;

namespace jit {

struct CallDest;

///////////////////////////////////////////////////////////////////////////////

namespace detail {

/*
 * Type::operator| can't reasonably be made constexpr. This macro constructs a
 * constexpr Type that's the union of two other non-Mem, non-specialized Types.
 */
#define U(t1, t2) Type(Type::k##t1 | Type::k##t2, Ptr::NotPtr, Mem::NotMem)

#define CPP_TYPES                \
  T(ActRec*, TFramePtr)          \
  T(AsioBlockableChain, TABC)    \
  T(Class*, TCls)                \
  T(LazyClassData, TLazyCls)     \
  T(Func*, TFunc)                \
  T(RFuncData*, TRFunc)          \
  T(ClsMethDataRef, TClsMeth)    \
  T(RClsMethData*, TRClsMeth)    \
  T(NamedEntity*, TNamedEntity)  \
  T(ResourceHdr*, TRes)          \
  T(StringData*, TStr)           \
  T(TCA, TTCA)                   \
  T(TypedValue&, TPtrToCell)     \
  T(TypedValue*, TPtrToCell)     \
  T(TypedValue, TCell)           \
  T(bool*, TPtrToBool)           \
  T(bool, TBool)                 \
  T(double, TDbl)                \
  T(int, TInt)                   \
  T(long, TInt)                  \
  T(long long, TInt)             \
  T(unsigned long, TInt)         \
  T(unsigned long long, TInt)    \
  T(unsigned int, U(Int, RDSHandle)) \
  T(tv_lval, TLvalToCell)        \
  T(tv_rval, TLvalToCell)

/*
 * jit_cpp_type<> handles all types that are the same for parameters and return
 * values.
 */
template<typename T, typename Enable = void> struct jit_cpp_type;

#define T(native_t, jit_t)                         \
  template<> struct jit_cpp_type<native_t> {       \
    static auto constexpr type() { return jit_t; } \
  };
CPP_TYPES
#undef T
#undef U
#undef CPP_TYPES

/*
 * All subtypes of ObjectData and ArrayData map to TObj or TArrLike,
 * respectively.
 */
template<typename O> struct jit_cpp_type<
  O*, std::enable_if_t<std::is_base_of<ObjectData, O>::value>
> {
  static auto constexpr type() { return TObj; }
};

template<typename A> struct jit_cpp_type<
  A*, std::enable_if_t<std::is_base_of<ArrayData, A>::value>
> {
  static auto constexpr type() { return TArrLike; }
};

template<typename A> struct jit_cpp_type<
  A*, std::enable_if_t<std::is_base_of<RecordData, A>::value>
> {
  static auto constexpr type() { return TRecord; }
};

template<typename A> struct jit_cpp_type<
  A*, std::enable_if_t<std::is_base_of<RecordDesc, A>::value>
> {
  static auto constexpr type() { return TRecDesc; }
};
/*
 * Parameter types: Many helper functions take various enums or pointers to
 * runtime types that have no jit::Type equivalent. These are usually passed as
 * untyped arguments to ArgGroup, which come through as TBottom, but some are
 * from constant TInt values. Default to TInt for types not handled by
 * jit_cpp_type<>, which allows both TInt and TBottom arguments.
 */
template<typename T, typename Enable = Type>
struct jit_param_type {
  static auto constexpr type() { return TInt; }
};

template<typename T>
struct jit_param_type<T, decltype(jit_cpp_type<T>::type())> {
  static auto constexpr type() { return jit_cpp_type<T>::type(); }
};

/*
 * Return types: Unlike argument types, we don't have a default Type for return
 * types. All returned values must map to a valid hhir value, so we require a
 * jit::Type for every possible C++ return type.
 *
 * There are two return types that should never be used as an hhir value: void
 * and void*. Map those to TTop, so they aren't accepted by any desired return
 * type.
 */
template<typename T> struct jit_ret_type : jit_cpp_type<T> {};

template<> struct jit_ret_type<void>  {
  static auto constexpr type() { return TTop; }
};
template<> struct jit_ret_type<void*> {
  static auto constexpr type() { return TTop; }
};


template<typename T>
struct strip_inner_const { using type = T; };

template<typename T>
struct strip_inner_const<T const*> { using type = T*; };

template<typename T>
using strip_t = typename strip_inner_const<std::remove_cv_t<T>>::type;

}

/*
 * FuncType holds the signature of a C++ function, with jit::Type equivalents
 * for all C++ types involved.
 */
struct FuncType {
  FuncType() = default;

  template<typename Ret, typename... Args>
  FuncType(Ret (*f)(Args...))
    : ret{detail::jit_ret_type<detail::strip_t<Ret>>::type()}
    , params{detail::jit_param_type<detail::strip_t<Args>>::type()...}
  {}

  Type ret;
  std::vector<Type> params;
};

/*
 * Return a pointer to a static FuncType for the given function pointer.
 */
template<typename Ret, typename... Args>
const FuncType* get_func_type(Ret (*f)(Args...)) {
  static const FuncType type{f};
  return &type;
}

/*
 * Information about how to make different sorts of calls from the JIT.
 */
struct CallSpec {
  enum class Kind : uint8_t {
    /*
     * Normal direct call to a known C++ function.
     *
     * We don't distinguish between our representations for direct calls to
     * free functions and those to member functions, with the assumption that
     * they have the same calling convention.
     */
    Direct,

    /*
     * Smashable direct call.
     *
     * Just like `Direct', except we ensure that the call target can be safely
     * mutated even in the presence of concurrent execution.
     */
    Smashable,

    /*
     * Call the appropriate destructor (i.e., release) function for the unitary
     * argument.
     *
     * A Vreg containing the data pointer's DataType is used to determine the
     * correct function to call.
     */
    Destructor,

    /*
     * Call a unique stub.
     *
     * Produce a call to JIT code which begins with a prologue{} Vinstr.
     *
     * TODO(#8425101): Make this true.
     */
    Stub,

    /*
     * Call the appropriate release function for an object.
     *
     * A Vreg containing the object's class is used to determine the correct
     * function to call.
     */
    ObjDestructor
  };

  CallSpec() = delete;
  CallSpec(const CallSpec&) = default;
  CallSpec& operator=(const CallSpec&) = default;

  /////////////////////////////////////////////////////////////////////////////
  // Static constructors.

  /*
   * A Direct call to the free C++ function `fp'. By default, the signature
   * will be inferred from the given function pointer. This can be avoided by
   * explicitly passing nullptr for the FuncType.
   */
  template<class Ret, class... Args>
  static CallSpec direct(Ret (*fp)(Args...), const FuncType* type) {
    return CallSpec { Kind::Direct, reinterpret_cast<void*>(fp), type };
  }

  template<class F>
  static CallSpec direct(F f) {
    return direct(f, get_func_type(f));
  }

  /*
   * A Direct call to the /non-virtual/ C++ instance method function `fp'.
   *
   * This uses ABI-specific tricks to get the address of the code out of the
   * pointer-to-member.  Additionally, we use the same Kind to represent both
   * free functions and member functions, which assumes that they have the same
   * calling convention.
   */
  template<class Ret, class Cls, class... Args>
  static CallSpec method(Ret (Cls::*fp)(Args...)) {
    return CallSpec { Kind::Direct, getMethodPtr(fp) };
  }
  template<class Ret, class Cls, class... Args>
  static CallSpec method(Ret (Cls::*fp)(Args...) const) {
    return CallSpec { Kind::Direct, getMethodPtr(fp) };
  }

  /*
   * A Smashable direct call to the free C++ function `fp'.
   */
  template<class Ret, class... Args>
  static CallSpec smashable(Ret (*fp)(Args...)) {
    return CallSpec { Kind::Smashable, reinterpret_cast<void*>(fp) };
  }

  static CallSpec smashable(TCA fp) {
    return CallSpec { Kind::Smashable, fp };
  }

  /*
   * A Destructor call for the DataType in `r'.
   */
  static CallSpec destruct(Vreg r) {
    return CallSpec { Kind::Destructor, r };
  }

  /*
   * A Stub call to `addr'.
   */
  static CallSpec stub(TCA addr) {
    return CallSpec { Kind::Stub, addr };
  }

  /*
   * A Destructor for an object with class `cls'.
   */
  static CallSpec objDestruct(Vreg cls) {
    return CallSpec { Kind::ObjDestructor, cls };
  }

  /////////////////////////////////////////////////////////////////////////////
  // Accessors.

  /*
   * Return the type tag.
   */
  Kind kind() const { return m_typeKind.tag(); }

  /*
   * The address of a C++ function, for Direct or Smashable calls.
   */
  void* address() const {
    assertx(kind() == Kind::Direct ||
            kind() == Kind::Smashable);
    return m_u.fp;
  }

  /*
   * The register containing the DataType, for Destructor calls.
   */
  Vreg reg() const {
    assertx(kind() == Kind::Destructor ||
            kind() == Kind::ObjDestructor);
    return m_u.reg;
  }

  /*
   * The address of the unique stub, for Stub calls.
   */
  TCA stubAddr() const {
    assertx(kind() == Kind::Stub);
    return m_u.stub;
  }

  /*
   * If this CallSpec was constructed with a FuncKind, verify that the given
   * argument types and expected return type match the FuncKind.
   */
  bool verifySignature(const CallDest& dest,
                       const std::vector<Type>& args) const;

  /////////////////////////////////////////////////////////////////////////////

  bool operator==(const CallSpec& o) const {
    auto const k1 = kind();
    auto const k2 = o.kind();
    if (k1 != k2) return false;
    switch (k1) {
      case CallSpec::Kind::Direct:
      case CallSpec::Kind::Smashable:  return address() == o.address();
      case CallSpec::Kind::Destructor: return reg() == o.reg();
      case CallSpec::Kind::Stub:       return stubAddr() == o.stubAddr();
      case CallSpec::Kind::ObjDestructor: return reg() == o.reg();
    }
    always_assert(false);
  }
  bool operator!=(const CallSpec& o) const { return !(*this == o); }

private:
  union U {
    /* implicit */ U(void* fp) : fp(fp) {}
    /* implicit */ U(Vreg reg) : reg(reg) {}
    /* implicit */ U(TCA stub) : stub(stub) {}

    void* fp;
    Vreg reg;
    TCA stub;
  };

private:
  CallSpec(Kind k, U u, const FuncType* type = nullptr)
    : m_typeKind{k, type}
    , m_u{u}
  {}

private:
  CompactTaggedPtr<const FuncType, Kind> m_typeKind;
  U m_u;
};

///////////////////////////////////////////////////////////////////////////////

}}
