<?hh
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
  <<__NoAutoLikes>>
  function as_mut<T>(readonly T $x)[]: T;

  abstract final class Shapes {
    /**
     * Use `Readonly\Shapes::idx` to retrieve a field value in a shape, when the key may or may not exist.
     * If `$index` does not exist in the shape, the default value will be returned (`$default`), if one has been set.
     * It behaves similarily to `idx()` for Collections.
     *
     * A few examples:
     * * `Shapes::idx(shape('x' => 123), 'x') // 123`
     * * `Shapes::idx(shape('x' => 123), 'y') // null`
     * * `Shapes::idx(shape('x' => 123), 'y', 456) // 456`
     *
     * * `Shapes::idx(null, 'y', 456) // 456`
     *
     * Use `Shapes::idx` when the key in your shape is optional (e.g., `?x`, in `shape(?'x' => int`).
     * If the key in your shape is always present, access the value directly: `$my_shape['x']`.
     *
     * The second argument, `$index` must always be a literal.
     *
     * @param shape(...) $shape   - shape to search for $index.
     * @param arraykey $index     - Key ($index) to search. Must be a literal!
     * @param mixed $default      - Default value to return if $index does not exist. By default, returns `null`.
     *
     * @return $value              - Value at $index, if it exists, or $default.
     *
     */
    public static function idx<Tv>(
      readonly ?shape(...) $shape,
      readonly arraykey $index,
      ?Tv $default = null,
    )[]: readonly Tv;

    public static function toArray(
      readonly shape(...) $shape,
    )[]: readonly darray<arraykey, mixed>;

    public static function toDict(
      readonly shape(...) $shape,
    )[]: readonly dict<arraykey, mixed>;

    /**
     * Returns the value of the field $index of a readonly $shape,
     * throws if the field is missing.
     * Use this to access optional fields on readonly shapes.
     */
    public static function at<Tv>(
      readonly shape(...) $shape,
      readonly arraykey $index,
    )[]: readonly Tv;
  }
}
