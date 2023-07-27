/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <type_traits>

namespace carbon {

template <class T>
struct ListContains<List<>, T> {
  static constexpr bool value = false;
};

template <class T, class X, class... Xs>
struct ListContains<List<X, Xs...>, T> {
  static constexpr bool value =
      std::is_same<T, X>::value || ListContains<List<Xs...>, T>::value;
};

template <class T, class... Ts>
struct Prepend<T, List<Ts...>> {
  using type = List<T, Ts...>;
};

template <>
struct ListDedup<List<>, void> {
  using type = List<>;
};

template <class X, class... Xs>
struct ListDedup<
    List<X, Xs...>,
    typename std::enable_if<!ListContains<List<Xs...>, X>::value>::type> {
  using type = typename Prepend<X, typename ListDedup<List<Xs...>>::type>::type;
};

template <class X, class... Xs>
struct ListDedup<
    List<X, Xs...>,
    typename std::enable_if<ListContains<List<Xs...>, X>::value>::type> {
  using type = typename ListDedup<List<Xs...>>::type;
};

template <typename L, typename Y>
struct ListWithout;

template <typename Y>
struct ListWithout<List<>, Y> {
  using type = List<>;
};

template <typename X, typename... Xs, typename Y>
struct ListWithout<List<X, Xs...>, Y> {
  using tail = typename ListWithout<List<Xs...>, Y>::type;
  using type = std::conditional_t<
      std::is_same_v<X, Y>,
      tail,
      typename Prepend<X, tail>::type>;
};

template <typename L, typename M>
struct ListSubtractUnique;

template <typename L>
struct ListSubtractUnique<L, List<>> {
  using type = L;
};

template <typename L, typename Y, typename... Ys>
struct ListSubtractUnique<L, List<Y, Ys...>> {
  using type = typename ListSubtractUnique<
      typename ListWithout<L, Y>::type,
      List<Ys...>>::type;
};

template <typename L, typename M>
struct ListSubtract {
  using type = typename ListSubtractUnique<
      typename ListDedup<L>::type,
      typename ListDedup<M>::type>::type;
};

template <class First>
struct FindByPairFirst<First, List<>> {
  using type = void;
};

template <class First, class P1, class... Ps>
struct FindByPairFirst<First, List<P1, Ps...>> {
  using type = typename std::conditional<
      std::is_same<First, typename P1::First>::value,
      typename P1::Second,
      typename FindByPairFirst<First, List<Ps...>>::type>::type;
};

template <int K>
struct FindByKey<K, List<>> {
  using type = void;
};
template <int K, class KV1, class... KVs>
struct FindByKey<K, List<KV1, KVs...>> {
  using type = typename std::conditional<
      K == KV1::Key,
      typename KV1::Value,
      typename FindByKey<K, List<KVs...>>::type>::type;
};

} // namespace carbon
