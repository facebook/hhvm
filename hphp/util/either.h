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
#ifndef incl_HPHP_EITHER_H_
#define incl_HPHP_EITHER_H_

#include <cstdint>
#include <cstddef>
#include <type_traits>

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
 *   - The pointer types used with this class must be known to be at
 *     least 2-byte aligned, since it discriminates using the low bit.
 *     NB: this is not the case for string literals on gcc.
 *
 *   - This class assumes pointers of L and R types never alias.
 *
 */
template<class L, class R>
struct Either {
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
  {}

  /*
   * Create an Either in the right mode.
   *
   * Post: left() == nullptr && right() == r
   */
  /* implicit */ Either(R r)
    : bits{reinterpret_cast<uintptr_t>(r) | 0x1}
  {}

  /*
   * Equality comparison is shallow.  If the pointers are equal, the
   * Eithers are equal.
   */
  bool operator==(Either<L,R> o) const {
    // We assume L* and R* don't alias, so we don't need to check type
    // tags when comparing.
    return (bits | 0x1) == (o.bits | 0x1);
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
    return rf(reinterpret_cast<R>(bits & ~0x1));
  }

  /*
   * Functions that simultaneously query the type tag and extract the
   * pointer of that type.  Both return nullptr if the Either does not
   * hold that type.
   */
  L left() const {
    return bits & 0x1 ? nullptr : reinterpret_cast<L>(bits);
  }
  R right() const {
    return bits & 0x1 ? reinterpret_cast<R>(bits & ~0x1) : nullptr;
  }

  /*
   * Returns whether this Either contains neither left nor right.
   */
  bool isNull() const { return !(bits & ~0x1); }

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
