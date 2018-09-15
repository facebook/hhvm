<?hh // decl
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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
  <<__Rx>>
  public static function idx<T as shape(...)>(
    T $shape,
    arraykey $index,
    $default = null,
  );

  /**
   * Check if a field in shape exists.
   * Similar to array_key_exists, but for shapes.
   */
  <<__Rx>>
  public static function keyExists<T as shape(...)>(
    T $shape,
    arraykey $index
  ): bool;

  /**
   * Returns a $shape with $index field removed. Currently allowed only for
   * local variables.
   */
  <<__Rx>>
  public static function removeKey<T as shape(...)>(
    inout T $shape,
    arraykey $index
  ): void;

  <<__Rx>>
  public static function toArray<T as shape(...)>(
    T $shape
  ): darray<arraykey, mixed>;

  <<__Rx>>
  public static function toDict<T as shape(...)>(
    T $shape
  ): dict<arraykey, mixed>;

}
