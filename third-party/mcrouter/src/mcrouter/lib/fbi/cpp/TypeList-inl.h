/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <type_traits>

namespace facebook {
namespace memcache {
namespace detail {

/* Concatenate implementation */

template <class... Items1, class... Items2>
struct ConcatenateListsImpl<List<Items1...>, List<Items2...>> {
  using type = List<Items1..., Items2...>;
};

template <class List1, class... Lists>
struct ConcatenateListsImpl<List1, Lists...> {
  using type = typename ConcatenateListsImpl<
      List1,
      typename ConcatenateListsImpl<Lists...>::type>::type;
};

/* Concatenate unit tests */
static_assert(
    std::is_same<
        List<int, double, float, long, char>,
        ConcatenateListsT<List<int, double>, List<float, long>, List<char>>>::
        value,
    "concatenate is broken");

/* Sort implementation */

/* Single bubble sort iteration: for each i, order (i, i+1) */
template <class MessageList, class Enable = void>
struct SortIter;
template <class MessageList>
using SortIterT = typename SortIter<MessageList>::type;

/* Case element i > element i + 1 */
template <class Tx, class Ty, class... Ts>
struct SortIter<
    List<Tx, Ty, Ts...>,
    typename std::enable_if<(Tx::typeId > Ty::typeId)>::type> {
  using type = PrependT<Ty, SortIterT<PrependT<Tx, List<Ts...>>>>;
};

/* Case element i <= element i + 1 */
template <class Tx, class Ty, class... Ts>
struct SortIter<
    List<Tx, Ty, Ts...>,
    typename std::enable_if<(Tx::typeId <= Ty::typeId)>::type> {
  using type = PrependT<Tx, SortIterT<PrependT<Ty, List<Ts...>>>>;
};

template <class T>
struct SortIter<List<T>> {
  using type = List<T>;
};

/* Sort: run the iteration above N times (N = size of list) */
template <class MessageList, size_t N>
using SortImplT = typename SortImpl<MessageList, N>::type;

template <class... Ts, size_t N>
struct SortImpl<
    List<Ts...>,
    N,
    typename std::enable_if<N == sizeof...(Ts)>::type> {
  using type = List<Ts...>;
};
template <class... Ts, size_t N>
struct SortImpl<
    List<Ts...>,
    N,
    typename std::enable_if<(N < sizeof...(Ts))>::type> {
  using type = SortImplT<SortIterT<List<Ts...>>, N + 1>;
};

/* Sort unit test */
namespace detail {
namespace type_list_sort_test {
struct A {
  static constexpr size_t typeId = 0;
};
struct B {
  static constexpr size_t typeId = 1;
};
struct C {
  static constexpr size_t typeId = 2;
};
struct D {
  static constexpr size_t typeId = 3;
};
struct E {
  static constexpr size_t typeId = 4;
};
struct F {
  static constexpr size_t typeId = 5;
};
struct G {
  static constexpr size_t typeId = 6;
};
struct H {
  static constexpr size_t typeId = 7;
};
static_assert(
    std::is_same<
        List<A, B, C, D, E, F, G, H>,
        SortT<List<E, B, G, C, D, H, F, A>>>::value,
    "SortT is broken");
} // namespace type_list_sort_test
} // namespace detail

/* Expand implementation */
template <int Start, class MessageList>
using ExpandImplT = typename ExpandImpl<Start, MessageList>::type;

/* Case start == first element in the list */
template <int Start, class T, class... Ts>
struct ExpandImpl<
    Start,
    List<T, Ts...>,
    typename std::enable_if<(Start == T::typeId)>::type> {
  using type = PrependT<T, ExpandImplT<Start + 1, List<Ts...>>>;
};

/* Case start < first element in the list, insert
   a fake element of type void */
template <int Start, class T, class... Ts>
struct ExpandImpl<
    Start,
    List<T, Ts...>,
    typename std::enable_if<(Start < T::typeId)>::type> {
  using type = PrependT<void, ExpandImplT<Start + 1, List<T, Ts...>>>;
};
template <int Start>
struct ExpandImpl<Start, List<>> {
  using type = List<>;
};

/* PairListFirst test */
static_assert(
    std::is_same<
        PairListFirstT<List<Pair<int, double>, Pair<float, char>>>,
        List<int, float>>::value,
    "PairListFirst list is broken");

/* PairListSecond test */
static_assert(
    std::is_same<
        PairListSecondT<List<Pair<int, double>, Pair<float, char>>>,
        List<double, char>>::value,
    "PairListSecond list is broken");
} // namespace detail
} // namespace memcache
} // namespace facebook
