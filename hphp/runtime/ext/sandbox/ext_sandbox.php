<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

namespace HH\Sandbox {
  /**
   * IMPORTANT: Returns an empty collection of default size in repo mode.
   * IMPORTANT: the capacity parameter is a hint to the compiler, and 
   * can be ignored or modified by HHVM. 
   * IMPORTANT: This function may become a no-op if we implement
   * a compiler optimization or new feature to hint collection size.
   *
   * Creates an empty dict with preallocated capacity.
   *
   * Use this when you know approximately how many elements you'll add
   * to avoid repeated reallocations during population.
   *
   * @param int $capacity - The number of elements to reserve space for
   * @return dict<Tk, Tv> - An empty dict with reserved capacity
   */
  <<__Native>>
  function dict_with_capacity<Tk as arraykey, Tv>(int $capacity)[]: dict<Tk, Tv>;

  /**
   * IMPORTANT: Returns an empty collection of default size in repo mode.
   * IMPORTANT: the capacity parameter is a hint to the compiler, and 
   * can be ignored or modified by HHVM. 
   * IMPORTANT: This function may become a no-op if we implement
   * a compiler optimization or new feature to hint collection size.
   *
   * Creates an empty vec with preallocated capacity.
   *
   * Use this when you know approximately how many elements you'll add
   * to avoid repeated reallocations during population.
   *
   * @param int $capacity - The number of elements to reserve space for
   * @return vec<T> - An empty vec with reserved capacity
   */
  <<__Native>>
  function vec_with_capacity<T>(int $capacity)[]: vec<T>;
}
