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
#ifndef incl_HPHP_UTIL_MATCH_H_
#define incl_HPHP_UTIL_MATCH_H_

#include <type_traits>
#include <utility>

#include <boost/variant.hpp>

#include <folly/DiscriminatedPtr.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * This is a utility for short-hand visitors (using lambdas) with
 * boost::apply_visitor.
 *
 * Usage e.g.:
 *
 *   match<return_type>(
 *     thing,
 *     [&] (TypeA a) { ... },
 *     [&] (TypeB b) { ... }
 *   );
 */

//////////////////////////////////////////////////////////////////////

namespace match_detail {

template<class Ret, class... Lambdas> struct visitor;

template<class Ret, class L, class... Lambdas>
struct visitor<Ret,L,Lambdas...> : L, visitor<Ret,Lambdas...> {
  using L::operator();
  using visitor<Ret,Lambdas...>::operator();
  visitor(L l, Lambdas&&... lambdas)
    : L(l)
    , visitor<Ret,Lambdas...>(std::forward<Lambdas>(lambdas)...)
  {}
};

template<class Ret, class L>
struct visitor<Ret,L> : L {
  typedef Ret result_type;
  using L::operator();
  /* implicit */ visitor(L l) : L(l) {}
};

template<class Ret> struct visitor<Ret> {
  typedef Ret result_type;
  visitor() {}
};

template<class Ret, class... Funcs>
visitor<Ret,Funcs...> make_visitor(Funcs&&... funcs) {
  return { std::forward<Funcs>(funcs)... };
}

template <typename T> struct is_variant {
  static constexpr bool value = false;
};
template <typename... T> struct is_variant<boost::variant<T...>> {
  static constexpr bool value = true;
};

template <typename T> struct is_discriminated_ptr {
  static constexpr bool value = false;
};
template <typename... T>
struct is_discriminated_ptr<folly::DiscriminatedPtr<T...>> {
  static constexpr bool value = true;
};

}

template<class Ret, class Var, class... Funcs>
typename std::enable_if<
  match_detail::is_variant<
    typename std::remove_cv<Var>::type
    >::value,
  Ret>::type
match(Var& v, Funcs&&... funcs) {
  return boost::apply_visitor(
    match_detail::make_visitor<Ret>(std::forward<Funcs>(funcs)...),
    v
  );
}

template<class Ret, class Var, class... Funcs>
typename std::enable_if<
  match_detail::is_discriminated_ptr<
    typename std::remove_cv<Var>::type
    >::value,
  Ret>::type
match(Var& v, Funcs&&... funcs) {
  return v.apply(
    match_detail::make_visitor<Ret>(std::forward<Funcs>(funcs)...)
  );
}

//////////////////////////////////////////////////////////////////////

}

#endif
