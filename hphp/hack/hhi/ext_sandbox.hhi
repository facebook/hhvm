<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH\Sandbox {
  /**
   * Creates an empty dict with preallocated capacity.
   *
   * Use this when you know approximately how many elements you'll add
   * to avoid repeated reallocations during population.
   *
   * @param int $capacity - The number of elements to reserve space for
   * @return dict<Tk, Tv> - An empty dict with reserved capacity
   */
  function dict_with_capacity<Tk as arraykey, Tv>(int $capacity)[]: dict<Tk, Tv>;

  /**
   * Creates an empty vec with preallocated capacity.
   *
   * Use this when you know approximately how many elements you'll add
   * to avoid repeated reallocations during population.
   *
   * @param int $capacity - The number of elements to reserve space for
   * @return vec<T> - An empty vec with reserved capacity
   */
  function vec_with_capacity<T>(int $capacity)[]: vec<T>;
}
