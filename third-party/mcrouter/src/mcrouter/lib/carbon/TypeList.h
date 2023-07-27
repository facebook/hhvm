/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace carbon {

/**
 * List for holding arbitrary types
 */
template <class... Xs>
struct List {};

/**
 * ListContains<L, T>::value == true if and only if T appears in L
 */
template <class L, class T>
struct ListContains;

/**
 * (T, List<Ts...>) -> List<T, Ts...>
 */
template <class T, class L>
struct Prepend;

/**
 * ListDedup<L>::type, contains a List of unique items from L
 */
template <class L, class Enable = void>
struct ListDedup;

/**
 * ListSubtract<L, M>::type is a list of all elements of L that are not in M
 */
template <typename L, typename M>
struct ListSubtract;

/**
 * (First, List<P...>) -> Second
 */
template <class First, class PairList>
struct FindByPairFirst;

/**
 * (K, List<KV...>) -> V
 */
template <int K, class L>
struct FindByKey;

} // namespace carbon

#include "TypeList-inl.h"
