<?hh // decl
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

abstract final class Shapes {

/**
 * Shapes::idx is a helper function for accessing shape field value, or getting
 * a default if it's not set - similar to idx(), but for shapes.
 *
 * This behavior cannot be expressed with type hints, so it's hardcoded in the
 * typechecker. Depending on arity, it will be one of the
 *
 * idx(S $shape, arraykey $index) : ?Tv,
 * idx(S $shape, arraykey $index, Tv $default) : Tv,
 *
 * where $index must be statically known (literal or class constant), and S is
 * a shape containing such key:
 *
 * type S = shape(
 *   ...
 *   $index => Tv,
 *   ...
 * )
 */
  public static function idx(
    shape() $shape,
    arraykey $index,
    $default = null,
  ) {}

  /**
   * Check if a field in shape exists.
   * Similar to array_key_exists, but for shapes.
   */
  public static function keyExists(shape() $shape, arraykey $index): bool {}

  /**
   * Returns a $shape with $index field removed. Currently allowed only for
   * local variables.
   */
  public static function removeKey(shape() $shape, arraykey $index): void {}

  public static function toArray(shape() $shape): array<arraykey, mixed>;
}
