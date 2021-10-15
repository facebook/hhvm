<?hh
<<file:__EnableUnstableFeatures("readonly")>>
/**
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH\Readonly {
  /**
   * Converts a readonly value type into a mutable one.
   * Value types include numerics, strings, bools, null and Hack arrays of value
   * types.
   */
  function as_mut<T>(readonly T $x)[]: T;
}
