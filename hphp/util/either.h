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
#ifndef incl_HPHP_EITHER_H_
#define incl_HPHP_EITHER_H_

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <type_traits>

// Workaround a bug that using std::common_type<void, void> causes error with
// libc++. See https://llvm.org/bugs/show_bug.cgi?id=22135
#ifdef _LIBCPP_VERSION
namespace std {

template<>
struct common_type<void, void> {
  typedef void type;
};

}
#endif

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Discriminated pointer to one of two types, or a nullptr.
 *
 * It does not distinguish between two types of null (for left and
 * right)---if the Either is nullptr it is effectively a third state.
 *
 * This class does not do any intelligent automatic ownership, and is
 * guaranteed to be a trivially copyable, standard layout class.
 * Zero-initializing it is guaranteed to have a nullptr value
 * (assuming null is represented with all bits zero, which of course
 * it is on our supported architectures).
 *
 * Requirements:
 *
 *   - The two types must be unrelated, and must be pointers.
 *
 *   - The pointer types used with this class must be known to be at least
 *     2-byte aligned with the default arguments, since it discriminates using
 *     the low bit (NB: this is not the case for string literals on gcc).  You
 *     can pass either_policy::high_bit to use the highest bit as the tag
 *     instead in cases where this doesn't apply.
 *
 *   - This class assumes pointers of L and R types never alias.
 *
 */
namespace either_policy { struct high_bit {}; }
template<class L, class R, class TagBitPolicy = void>
struct Either {
  static_assert(
    std::is_same<TagBitPolicy,void>::value ||
      std::is_same<TagBitPolicy,either_policy::high_bit>::value,
    "Unknown policy in Either"
  );
  static constexpr uintptr_t TagBit =
    std::conditional<
      std::is_same<TagBitPolicy,void>::value,
      std::integral_constant<uintptr_t,0x1>,
      std::integral_constant<uintptr_t,0x8000000000000000>
    >::type::value;

  /*
   * The default constructor creates an Either that isNull.
   *
   * Post: left() == nullptr && right() == nullptr
   *       isNull()
   */
  Either() : bits{0} {}

  /*
   * Create an Either that isNull.
   *
   * Post: left() == nullptr && right() == nullptr
   *       isNull()
   */
  /* implicit */ Either(std::nullptr_t) : bits{0} {}

  /*
   * Create an Either in the left mode.
   *
   * Post: left() == l && right() == nullptr
   */
  /* implicit */ Either(L l)
    : bits{reinterpret_cast<uintptr_t>(l)}
  {
    assert(!(reinterpret_cast<uintptr_t>(l) & TagBit));
  }

  /*
   * Create an Either in the right mode.
   *
   * Post: left() == nullptr && right() == r
   */
  /* implicit */ Either(R r)
    : bits{reinterpret_cast<uintptr_t>(r) | TagBit}
  {
    assert(!(reinterpret_cast<uintptr_t>(r) & TagBit));
  }

  /*
   * Equality comparison is shallow.  If the pointers are equal, the
   * Eithers are equal.
   */
  bool operator==(Either<L,R> o) const {
    // We assume L* and R* don't alias, so we don't need to check type tags
    // when comparing.  But we have to put the TagBit in in case we constructed
    // a null from the right (we'll have a tagged null).
    return (bits | TagBit) == (o.bits | TagBit);
  }
  bool operator!=(Either<L,R> o) const {
    return !(*this == o);
  }

  /*
   * Matching on Eithers takes two functions, for left and right.  If
   * you know that the either is non-null, this is reasonable.  If it's
   * potentially null, the null case is passed as a null pointer to the
   * right branch.
   *
   * Example usage:
   *
   *   Either<A*,B*> foo;
   *   auto const count = foo.match(
   *     [&] (A* a) { return a->size(); },
   *     [&] (B* b) { return b->size(); }
   *   );
   */
  template<class LF, class RF>
  typename std::common_type<
    typename std::result_of<LF(L)>::type,
    typename std::result_of<RF(R)>::type
  >::type match(const LF& lf, const RF& rf) const {
    if (auto const l = left()) return lf(l);
    return rf(reinterpret_cast<R>(bits & ~TagBit));
  }

  /*
   * Functions that simultaneously query the type tag and extract the
   * pointer of that type.  Both return nullptr if the Either does not
   * hold that type.
   */
  L left() const {
    return bits & TagBit ? nullptr : reinterpret_cast<L>(bits);
  }
  R right() const {
    return bits & TagBit ? reinterpret_cast<R>(bits & ~TagBit) : nullptr;
  }

  /*
   * Returns whether this Either contains neither left nor right.
   */
  bool isNull() const { return !(bits & ~TagBit); }

private:
  static_assert(
    !std::is_convertible<L,R>::value && !std::is_convertible<R,L>::value,
    "Either<L,R> should not be used with compatible pointer types"
  );
  // Not implemented in gcc:
  // static_assert(
  //   std::is_standard_layout<Either>::value,
  //   "Either<L,R> should be a standard layout class"
  // );
  static_assert(
    std::is_pointer<L>::value && std::is_pointer<R>::value,
    "Either<L,R> should only be used with pointer types"
  );

private:
  uintptr_t bits;
};

//////////////////////////////////////////////////////////////////////

}

#endif
