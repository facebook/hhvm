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
     * Use `Shapes::idx` to retrieve a field value in a shape, when the key may or may not exist.
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
    <<__NoAutoDynamic, __SupportDynamicType>>
    public static function idx<Tv>(
      ?shape(...) $shape,
      arraykey $index,
      ?Tv $default = null,
    )[]: Tv;

    /**
     * Check if a field in shape exists.
     * Similar to array_key_exists, but for shapes.
     */
    <<__NoAutoDynamic, __SupportDynamicType>>
    public static function keyExists(
      readonly shape(...) $shape,
      arraykey $index,
    )[]: bool;

    /**
     * Removes the $index field from the $shape (passed in as an inout argument).
     * As with all inout arguments, it can only be used with local variables.
     */
    <<__NoAutoDynamic, __SupportDynamicType>>
    public static function removeKey<T as shape(...)>(
      inout T $shape,
      arraykey $index,
    )[]: void;

    <<__NoAutoDynamic, __SupportDynamicType>>
    public static function toArray(
      FIXME\SUPPORTDYN_MARKER<shape(...)> $shape,
    )[]: darray<arraykey, FIXME\SUPPORTDYN_MARKER<mixed>>;

    <<__NoAutoDynamic, __SupportDynamicType>>
    public static function toDict(
      FIXME\SUPPORTDYN_MARKER<shape(...)> $shape
    )[]: dict<arraykey, FIXME\SUPPORTDYN_MARKER<mixed>>;

    /**
     * Returns the value of the field $index of $shape,
     * throws if the field is missing.
     * Use this to access optional fields on shapes.
     */
    <<__NoAutoDynamic, __SupportDynamicType>>
    public static function at<Tv>(
      shape(...) $shape,
      arraykey $index,
    )[]: Tv;

  }

} // namespace HH
