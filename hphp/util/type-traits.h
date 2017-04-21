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

#ifndef incl_HPHP_UTIL_TYPE_TRAITS_H_
#define incl_HPHP_UTIL_TYPE_TRAITS_H_

#include <type_traits>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * Is `T' the same type as `U' or any type in `Tail...'?
 */
template<class T, class U, class... Tail> struct is_any;

template<class T, class U>
struct is_any<T,U> : std::is_same<T,U> {};

template<class T, class U, class V, class... Tail>
struct is_any<T,U,V,Tail...> : std::integral_constant<
  bool,
  std::is_same<T,U>::value || is_any<T,V,Tail...>::value
> {};

///////////////////////////////////////////////////////////////////////////////

/*
 * Whether `T' and `U' are the same type, with the outermost const-qualifier
 * stripped away.
 */
template<class T, class U>
using is_same_upto_const = std::is_same<
  typename std::remove_const<T>::type,
  typename std::remove_const<U>::type
>;

namespace detail {

template<class... Ts> struct maybe_const_pred;

template<>
struct maybe_const_pred<> : std::true_type {};

template<class T>
struct maybe_const_pred<T> : std::true_type {};

template<class T, class U, class... Tail>
struct maybe_const_pred<T, U, Tail...> : std::integral_constant<
  bool,
  is_same_upto_const<T,U>::value &&
    maybe_const_pred<Tail...>::value
> {};

template<class... Ts> struct maybe_const_result;

template<>
struct maybe_const_result<> { using type = void; };

template<class R>
struct maybe_const_result<R> { using type = R; };

template<class T, class U, class... Tail>
struct maybe_const_result<T, U, Tail...> : maybe_const_result<Tail...> {};

}

/*
 * Pattern for templatizing functions to take an optionally-const argument.
 * Writing
 *
 *    template<class T1, class T2, ...>
 *    maybe_const<T1, K1, T2, K2, ..., R>::type
 *
 * is equivalent to
 *
 *    template<class T1, class T2, ...>
 *    std::enable_if<
 *      is_same_upto_const<T1, K1>::value &&
 *      is_same_upto_const<T2, K2>::value &&
 *      ...,
 *      R
 *    >::type
 */
template<class T, class K, class... Tail>
using maybe_const = std::enable_if<
  detail::maybe_const_pred<T, K, Tail...>::value,
  typename detail::maybe_const_result<T, K, Tail...>::type
>;

///////////////////////////////////////////////////////////////////////////////

}

#endif
