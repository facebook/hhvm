<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH {

abstract final class Shapes {

/**
 * Shapes::idx is a helper function for accessing shape field value, or getting
 * a default if it's not set - similar to idx(), but for shapes.
 *
 * This behavior cannot be expressed with type hints, so it's hardcoded in the
 * typechecker. Depending on arity, it will be one of the
 *
 * idx(?S $shape, arraykey $index) : ?Tv,
 * idx(?S $shape, arraykey $index, Tv $default) : Tv,
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
  <<__Pure>>
  public static function idx(
    ?shape(...) $shape,
    arraykey $index,
    $default = null,
  );

  /**
   * Check if a field in shape exists.
   * Similar to array_key_exists, but for shapes.
   */
  <<__Pure>>
  public static function keyExists(
    shape(...) $shape,
    arraykey $index
  ): bool;

  /**
   * Removes the $index field from the $shape (passed in as an inout argument).
   * As with all inout arguments, it can only be used with local variables.
   */
  <<__Pure>>
  public static function removeKey<T as shape(...)>(
    inout T $shape,
    arraykey $index
  ): void;

  <<__Pure>>
  public static function toArray(
    shape(...) $shape
  ): darray<arraykey, mixed>;

  <<__Pure>>
  public static function toDict(
    shape(...) $shape
  ): dict<arraykey, mixed>;

  /**
   * Returns the value of the field $index of $shape,
   * throws if the field is missing.
   * Use this to access optional fields on shapes.
   */
  <<__Pure>>
  public static function at(
    shape(...) $shape,
    arraykey $index,
  );

}

} // namespace HH
