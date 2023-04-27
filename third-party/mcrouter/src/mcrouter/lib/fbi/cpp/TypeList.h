/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>

#include <cstddef>
#include <type_traits>

#include "mcrouter/lib/carbon/TypeList.h"

namespace facebook {
namespace memcache {

/**
 * Pair of arbitrary types.
 */
template <class L, class R>
struct Pair {
  using First = L;
  using Second = R;
};

/**
 * Type list and int list manipulation routines
 */

/**
 * List holding arbitrary types
 */
template <class... Xs>
using List = carbon::List<Xs...>;

/**
 * Concatenates several lists into one.
 */
namespace detail {
template <class... Lists>
struct ConcatenateListsImpl;
} // namespace detail

template <class... Lists>
struct ConcatenateLists {
  using type = typename detail::ConcatenateListsImpl<Lists...>::type;
};

template <class... Lists>
using ConcatenateListsT = typename ConcatenateLists<Lists...>::type;

/**
 * List<KV...> can be used as an {int -> T} map
 */
template <int Id, class T>
struct KV {
  static constexpr int Key = Id;
  using Value = T;
};

/**
 * (T, List<Ts...>) -> List<T, Ts...>
 */
template <class T, class L>
using Prepend = typename carbon::Prepend<T, L>;
template <class T, class L>
using PrependT = typename Prepend<T, L>::type;

/**
 * Sorts a list of Carbon messages by typeId.
 * List<T...> -> List<T...>
 */
namespace detail {
template <class MessageList, size_t N, class Enable = void>
struct SortImpl;
} // namespace detail

template <class MessageList>
using Sort = detail::SortImpl<MessageList, 0>;
template <class MessageList>
using SortT = typename Sort<MessageList>::type;

/**
 * Given a sorted list of Carbon messages (typeIds >= 0), fills up holes in ID
 * space
 *
 * List<M2 (typeId=2), M4 (typeId=4)> ->
 *   List<void, void, M2, void, M4>
 */
namespace detail {
template <int Start, class MessageList, class Enable = void>
struct ExpandImpl;
} // namespace detail

template <class MessageList>
using Expand = detail::ExpandImpl<0, MessageList>;
template <class MessageList>
using ExpandT = typename Expand<MessageList>::type;

/**
 * (F, Xs) -> F::op(X1, F::op(X2, ...))
 */
template <class F, int... Xs>
struct Fold;
template <class F, int X>
struct Fold<F, X> {
  static constexpr int value = X;
};
template <class F, int X, int... Xs>
struct Fold<F, X, Xs...> {
  static constexpr int value = F::op(X, Fold<F, Xs...>::value);
};

/**
 * max(Xs...)
 */
struct MaxOp {
  static constexpr int op(int a, int b) {
    return a > b ? a : b;
  }
};
template <int... Xs>
using Max = Fold<MaxOp, Xs...>;

/**
 * min(Xs...)
 */
struct MinOp {
  static constexpr int op(int a, int b) {
    return a < b ? a : b;
  }
};
template <int... Xs>
using Min = Fold<MinOp, Xs...>;

/**
 * (Y, Xs) -> true iff Y is in Xs
 */
template <int Y, int... Xs>
struct HasInt;
template <int Y>
struct HasInt<Y> {
  static constexpr bool value = false;
};
template <int Y, int X, int... Xs>
struct HasInt<Y, X, Xs...> {
  static constexpr bool value = (Y == X ? true : HasInt<Y, Xs...>::value);
};

template <class Y, class... Xs>
struct Has;
template <class Y>
struct Has<Y> {
  static constexpr bool value = false;
};
template <class Y, class X, class... Xs>
struct Has<Y, X, Xs...> {
  static constexpr bool value =
      (std::is_same<Y, X>::value ? true : Has<Y, Xs...>::value);
};

/**
 * (T, Ts...) -> Index of first occurrence of T in Ts...
 */
template <class T, class... Ts>
struct IndexOf;

template <class T>
struct IndexOf<T> {
  static constexpr int32_t value = -1;
};

template <class T, class U, class... Us>
struct IndexOf<T, U, Us...> {
  static constexpr int32_t value = std::is_same<T, U>::value
      ? 0
      : (IndexOf<T, Us...>::value >= 0 ? 1 + IndexOf<T, Us...>::value : -1);
};

/**
 * Xs -> true iff all Xs are pairwise distinct types
 */
template <class... Xs>
struct Distinct;
template <>
struct Distinct<> {
  static constexpr bool value = true;
};
template <class X, class... Xs>
struct Distinct<X, Xs...> {
  static constexpr bool value = !Has<X, Xs...>::value && Distinct<Xs...>::value;
};

/**
 * Xs -> true iff all Xs are distinct
 */
template <int... Xs>
struct DistinctInt;
template <int X>
struct DistinctInt<X> {
  static constexpr bool value = true;
};
template <int X, int... Xs>
struct DistinctInt<X, Xs...> {
  static constexpr bool value =
      (HasInt<X, Xs...>::value ? false : DistinctInt<Xs...>::value);
};

/**
 * Utilities for working with lists of pairs.
 */

/**
 * List<Pair<X, Y>...> -> List<X...>
 */
template <class List>
struct PairListFirst;

template <class... First, class... Second>
struct PairListFirst<List<Pair<First, Second>...>> {
  using type = List<First...>;
};

template <class List>
using PairListFirstT = typename PairListFirst<List>::type;

/**
 * List<Pair<X, Y>...> -> List<Y...>
 */
template <class List>
struct PairListSecond;

template <class... First, class... Second>
struct PairListSecond<List<Pair<First, Second>...>> {
  using type = List<Second...>;
};

template <class List>
using PairListSecondT = typename PairListSecond<List>::type;

/**
 * ListContains<L, T>::value == true if and only if T appears in L
 */
template <class L, class T>
using ListContains = typename carbon::ListContains<L, T>;

/**
 * Map a template template over a List of types.
 */
namespace detail {
template <template <typename> class F, class List>
struct MapTImpl;

template <template <typename> class F>
struct MapTImpl<F, List<>> {
  using type = List<>;
};

template <template <typename> class F, typename X, typename... Xs>
struct MapTImpl<F, List<X, Xs...>> {
  using type = PrependT<F<X>, typename MapTImpl<F, List<Xs...>>::type>;
};
} // namespace detail

template <template <typename> class F, typename List>
using MapT = typename detail::MapTImpl<F, List>::type;

} // namespace memcache
} // namespace facebook

#include "TypeList-inl.h"
