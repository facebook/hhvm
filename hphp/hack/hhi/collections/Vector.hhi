<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
<<file: __EnableUnstableFeatures('readonly')>>

/**
 * This file provides type information for some of HHVM's builtin classes.
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

namespace HH {

  /**
   * **Prefer `vec` over `Vector`**. If you need reference semantics,
   * you can wrap the `vec` in a `Ref`.
   *
   * `Vector` is a stack-like collection. HHVM provides a native implementation
   * for this class. The PHP class definition below is not actually used at run
   * time; it is simply provided for the typechecker and for developer reference.
   *
   * Like all objects in PHP, `Vector`s have reference-like semantics. When a
   * caller passes a `Vector` to a callee, the callee can modify the `Vector` and
   * the caller will see the changes. `Vector`s do not have "copy-on-write"
   * semantics.
   *
   * `Vector`s only support integer keys. If a non-integer key is used, an
   * exception will be thrown.
   *
   * `Vector`s support `$m[$k]` style syntax for getting and setting values by
   * key. `Vector`s also support `isset($m[$k])` and `empty($m[$k])` syntax, and
   * they provide similar semantics as arrays. Elements can be added to a
   * `Vector` using `$m[] = ..` syntax.
   *
   * `Vector`s do not support iterating while new elements are being added or
   * elements are being removed. When a new element is added or removed, all
   * iterators that point to the `Vector` shall be considered invalid.
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/classes
   */
  final class Vector<Tv> implements \MutableVector<Tv> {
    /**
     * Creates a `Vector` from the given `Traversable`, or an empty `Vector`
     * if `null` is passed.
     *
     * @param $it - Any `Traversable` object from which to create
     *              the `Vector` (e.g., `array`). If `null`, then an empty
     *              `Vector` is created.
     */
    public function __construct(?Traversable<Tv> $it)[];

    /**
     * Returns an `array` containing the values from the current `Vector`.
     *
     * This method is interchangeable with `toArray()`.
     *
     * @return - An `array` containing the values from the current `Vector`.
     */
    public function toValuesArray()[]: varray<Tv>;

    /**
     * Returns an `array` whose values are the keys from the current `Vector`.
     *
     * @return - An `array` with the integer keys from the current `Vector`.
     */
    <<__NoAutoLikes>>
    public readonly function toKeysArray()[]: varray<int>;

    /**
     * Returns a copy of the current `Vector`.
     *
     * @return - A `Vector` that is a copy of the current `Vector`.
     */
    <<__NoAutoLikes>>
    public function toVector()[]: Vector<Tv>;

    /**
     * Returns an immutable copy (`ImmVector`) of the current `Vector`.
     *
     * @return - A `Vector` that is an immutable copy of the current `Vector`.
     */
    <<__NoAutoLikes>>
    public function toImmVector()[]: ImmVector<Tv>;

    /**
     * Returns an integer-keyed `Map` based on the values of the current `Vector`.
     *
     * @return - A `Map` that has the integer keys and associated values of the
     *           current `Vector`.
     */
    <<__NoAutoLikes>>
    public function toMap()[]: Map<int, Tv>;

    /**
     * Returns an immutable, integer-keyed map (`ImmMap`) based on the values of
     * the current `Vector`.
     *
     * @return - An `ImmMap` that has the integer keys and associated values
     *           of the current `Vector`.
     */
    <<__NoAutoLikes>>
    public function toImmMap()[]: ImmMap<int, Tv>;

    /**
     * Returns a `Set` based on the values of the current `Vector`.
     *
     * @return - A `Set` containing the unique values of the current `Vector`.
     */
    <<__NoAutoLikes>>
    public function toSet()[]: Set<Tv> where Tv as arraykey;

    /**
     * Returns an immutable set (`ImmSet`) based on the values of the current
     * `Vector`.
     *
     * @return - An `ImmSet` containing the unique values of the current `Vector`.
     */
    <<__NoAutoLikes>>
    public function toImmSet()[]: ImmSet<Tv> where Tv as arraykey;

    /**
     * Returns an immutable copy (`ImmVector`) of the current `Vector`.
     *
     * This method is interchangeable with `toImmVector()`.
     *
     * @return - An `ImmVector` copy of the current `Vector`.
     */
    <<__NoAutoLikes>>
    public function immutable()[]: ImmVector<Tv>;

    /**
     * Returns a lazy, access-elements-only-when-needed view of the current
     * `Vector`.
     *
     * Normally, memory is allocated for all of the elements of the `Vector`.
     * With a lazy view, memory is allocated for an element only when needed or
     * used in a calculation like in `map()` or `filter()`.
     *
     * @return - An integer-keyed `KeyedIterable` representing the lazy view into
     *           the current `Vector`.
     *
     * @guide /hack/collections/examples
     */
    public function lazy()[]: KeyedIterable<int, Tv>;

    /**
     * Returns a `Vector` containing the values of the current `Vector`.
     * Essentially a copy of the current `Vector`.
     *
     * This method is interchangeable with `toVector()`.
     *
     * @return - A `Vector` containing the values of the current `Vector`.
     */
    <<__NoAutoLikes>>
    public function values()[]: Vector<Tv>;

    /**
     * Returns a `Vector` containing the keys of the current `Vector`.
     *
     * @return - A `Vector` containing the integer keys of the current `Vector`.
     */
    <<__NoAutoLikes>>
    public readonly function keys()[]: Vector<int>;

    /**
     * Returns a `Vector` containing the results of applying an operation to each
     * value in the current `Vector`.
     *
     * `map()`'s result contains a value for every value in the current `Vector`;
     * unlike `filter()`, where only values that meet a certain criterion are
     * included in the resulting `Vector`.
     *
     * @param $fn - The callback containing the operation to apply to the
     *                    current `Vector`'s values.
     *
     * @return - A `Vector` containing the results of applying a user-specified
     *           operation to each value of the current `Vector` in turn.
     *
     * @guide /hack/collections/examples
     */
    public function map<Tu>((function(Tv)[_]: Tu) $fn)[ctx $fn]: Vector<Tu>;

    /**
     * Returns a `Vector` containing the results of applying an operation to each
     * key/value pair in the current `Vector`.
     *
     * `mapWithKey()`'s result contains a value for every key/value pair in the
     * current `Vector`; unlike `filterWithKey()`, where only values whose
     * key/value pairs meet a certain criterion are included in the resulting
     * `Vector`.
     *
     * @param $fn - The callback containing the operation to apply to the
     *                    current `Vector`'s key/value pairs.
     *
     * @return - A `Vector` containing the results of applying a user-specified
     *           operation to each key/value pair of the current `Vector` in turn.
     */
    public function mapWithKey<Tu>(
      (function(int, Tv)[_]: Tu) $fn,
    )[ctx $fn]: Vector<Tu>;

    /**
     * Returns a `Vector` containing the values of the current `Vector` that meet
     * a supplied condition.
     *
     * `filter()`'s result contains only values that meet the provided criterion;
     * unlike `map()`, where a value is included for each value in the original
     * `Vector`.
     *
     * @param $fn - The callback containing the condition to apply to the
     *                    `Vector` values.
     *
     * @return - A `Vector` containing the values after a user-specified condition
     *           is applied.
     *
     * @guide /hack/collections/examples
     */
    public function filter((function(Tv)[_]: bool) $fn)[ctx $fn]: Vector<Tv>;

    /**
     * Returns a `Vector` containing the values of the current `Vector` that meet
     * a supplied condition applied to its keys and values.
     *
     * `filterWithKey()`'s result contains only values whose key/value pairs
     * satisfy the provided criterion; unlike `mapWithKey()`, which contains
     * results derived from every key/value pair in the original `Vector`.
     *
     * @param $fn - The callback containing the condition to apply to the
     *                    `Vector`'s key/value pairs. For each key/value pair,
     *                    the key is passed as the first parameter to the
     *                    callback, and the value is passed as the second
     *                    parameter.
     *
     * @return - A `Vector` containing the values of the current `Vector` for
     *           which a user-specified test condition returns true when applied
     *           to the corresponding key/value pairs.
     *
     */
    public function filterWithKey(
      (function(int, Tv)[_]: bool) $fn,
    )[ctx $fn]: Vector<Tv>;

    /**
     * Returns a `Vector` where each element is a `Pair` that combines the
     * element of the current `Vector` and the provided `Traversable`.
     *
     * If the number of elements of the `Vector` are not equal to the number of
     * elements in the `Traversable`, then only the combined elements up to and
     * including the final element of the one with the least number of elements
     * is included.
     *
     * @param $traversable - The `Traversable` to use to combine with the
     *                       elements of the current `Vector`.
     *
     * @return - A `Vector` that combines the values of the current `Vector`
     *           with the provided `Traversable`.
     */
    public function zip<Tu>(
      Traversable<Tu> $traversable,
    )[]: Vector<Pair<Tv, Tu>>;

    /**
     * Returns a `Vector` containing the first `$n` values of the current
     * `Vector`.
     *
     * The returned `Vector` will always be a subset (but not necessarily a
     * proper subset) of the current `Vector`. If `$n` is greater than the length
     * of the current `Vector`, the returned `Vector` will contain all elements of
     * the current `Vector`.
    *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     * @param $n - The last element that will be included in the returned
     *             `Vector`.
     *
     * @return - A `Vector` that is a subset of the current `Vector` up to `$n`
     *           elements.
     */
    public function take(int $n)[]: Vector<Tv>;

    /**
     * Returns a `Vector` containing the values of the current `Vector` up to but
     * not including the first value that produces `false` when passed to the
     * specified callback. That is, takes the continuous prefix of values in
     * the current `Vector` for which the specified callback returns `true`.
     *
     * The returned `Vector` will always be a subset (but not necessarily a
     * proper subset) of the current `Vector`.
     *
     * @param $fn - The callback that is used to determine the stopping condition.
     *
     * @return - A `Vector` that is a subset of the current `Vector` up until the
     *           callback returns `false`.
     */
    public function takeWhile((function(Tv)[_]: bool) $fn)[ctx $fn]: Vector<Tv>;

    /**
     * Returns a `Vector` containing the values after the `$n`-th element of the
     * current `Vector`.
     *
     * The returned `Vector` will always be a subset (but not necessarily a
     * proper subset) of the current `Vector`. If `$n` is greater than or equal to
     * the length of the current `Vector`, the returned `Vector` will contain no
     * elements. If `$n` is negative, the returned `Vector` will contain all
     * elements of the current `Vector`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     * @param $n - The last element to be skipped; the `$n+1` element will be the
     *             first one in the returned `Vector`.
     *
     * @return - A `Vector` that is a subset of the current `Vector` containing
     *           values after the specified `$n`-th element.
     */
    public function skip(int $n)[]: Vector<Tv>;

    /**
     * Returns a `Vector` containing the values of the current `Vector` starting
     * after and including the first value that produces `false` when passed to
     * the specified callback. That is, skips the continuous prefix of values in
     * the current `Vector` for which the specified callback returns `true`.
     *
     * The returned `Vector` will always be a subset (but not necessarily a
     * proper subset) of the current `Vector`.
     *
     * @param $fn - The callback used to determine the starting element for the
     *              returned `Vector`.
     *
     * @return - A `Vector` that is a subset of the current `Vector` starting
     *           with the value for which the callback first returns `false`.
     */
    public function skipWhile((function(Tv)[_]: bool) $fn)[ctx $fn]: Vector<Tv>;

    /**
     * Returns a subset of the current `Vector` starting from a given key up to,
     * but not including, the element at the provided length from the starting key.
     *
     * `$start` is 0-based. `$len` is 1-based. So `slice(0, 2)` would return the
     * elements at keys 0 and 1.
     *
     * The returned `Vector` will always be a subset (but not necessarily a
     * proper subset) of the current `Vector`. If `$start` is greater than or
     * equal to the length of the current `Vector`, the returned `Vector` will
     * contain no elements.  If `$start` + `$len` is greater than or equal to the
     * length of the current `Vector`, the returned `Vector` will contain the
     * elements from `$start` to the end of the current `Vector`.
     *
     * If either `$start` or `$len` is negative, an exception is thrown.
     *
     * @param $start - The starting key of the current `Vector` at which to begin
     *                 the returned `Vector`.
     * @param $len - The length of the returned `Vector`.
     *
     * @return - A `Vector` that is a subset of the current `Vector` starting
     *           at `$start` up to but not including the element `$start + $len`.
     */
    public function slice(int $start, int $len)[]: Vector<Tv>;

    /**
     * Returns a `Vector` that is the concatenation of the values of the current
     * `Vector` and the values of the provided `Traversable`.
     *
     * The returned `Vector` is created from the values of the current `Vector`,
     * followed by the values of the provided `Traversable`.
     *
     * The returned `Vector` is a new object; the current `Vector` is unchanged.
     * Future changes to the current `Vector` will not affect the returned
     * `Vector`, and future changes to the returned `Vector` will not affect the
     * current `Vector`.
     *
     * @param $traversable - The `Traversable` to concatenate with the current
     *                       `Vector`.
     *
     * @return - A new `Vector` containing the values from `$traversable`
     *           concatenated to the values from the current `Vector`.
     *
     * @guide /hack/generics/constraints
     */
    public function concat<Tu super Tv>(
      Traversable<Tu> $traversable,
    )[]: Vector<Tu>;

    /**
     * Returns the first value in the current `Vector`.
     *
     * @return - The first value in the current `Vector`, or `null` if the
     *           `Vector` is empty.
     */
    public function firstValue()[]: ?Tv;

    /**
     * Returns the first key in the current `Vector`.
     *
     * @return - The first key (an integer) in the current `Vector`, or `null` if
     *           the `Vector` is empty.
     */
    public readonly function firstKey()[]: ?int;

    /**
     * Returns the last value in the current `Vector`.
     *
     * @return - The last value in the current `Vector`, or `null` if the current
     *           `Vector` is empty.
     */
    public function lastValue()[]: ?Tv;

    /**
     * Returns the last key in the current `Vector`.
     *
     * @return - The last key (an integer) in the current `Vector`, or `null` if
     *           the `Vector` is empty.
     */
    public readonly function lastKey()[]: ?int;

    /**
     * Checks if the current `Vector` is empty.
     *
     * @return - `true` if the current `Vector` is empty; `false` otherwise.
     */
    public readonly function isEmpty()[]: bool;

    /**
     * Returns the number of elements in the current `Vector`.
     *
     * @return - The number of elements in the current `Vector`.
     */
    public readonly function count()[]: int;

    /**
     * Returns the value at the specified key in the current `Vector`.
     *
     * If the key is not present, an exception is thrown. If you don't want an
     * exception to be thrown, use `get()` instead.
     *
     * `$v = $vec->at($k)` is semantically equivalent to `$v = $vec[$k]`.
     *
     * @param $k - The key for which to retrieve the value.
     *
     * @return - The value at the specified key; or an exception if the key does
     *           not exist.
     */
    public function at(int $k)[]: Tv;

    /**
     * Returns the value at the specified key in the current `Vector`.
     *
     * If the key is not present, null is returned. If you would rather have an
     * exception thrown when a key is not present, use `at()` instead.
     *
     * @param $k - The key for which to retrieve the value.
     *
     * @return - The value at the specified key; or `null` if the key does not
     *           exist.
     */
    public function get(int $k)[]: ?Tv;

    /**
     * Stores a value into the current `Vector` with the specified key,
     * overwriting the previous value associated with the key.
     *
     * If the key is not present, an exception is thrown. If you want to add
     * a value even if the key is not present, use `add()`.
     *
     * `$vec->set($k,$v)` is semantically equivalent to `$vec[$k] = $v` (except
     * that `set()` returns the current `Vector`).
     *
     * Future changes made to the current `Vector` ARE reflected in the
     * returned `Vector`, and vice-versa.
     *
     * @param $k - The key to which we will set the value.
     * @param $v - The value to set.
     *
     * @return - Returns itself.
     */
    public function set(int $k, Tv $v)[write_props]: Vector<Tv>;

    /**
     * For every element in the provided `Traversable`, stores a value into the
     * current `Vector` associated with each key, overwriting the previous value
     * associated with the key.
     *
     * If a key is not present the current `Vector` that is present in the
     * `Traversable`, an exception is thrown. If you want to add a value even if a
     * key is not present, use `addAll()`.
     *
     * Future changes made to the current `Vector` ARE reflected in the
     * returned `Vector`, and vice-versa.
     *
     * @param $k - The `Traversable` with the new values to set. If `null` is
     *             provided, no changes are made.
     *
     * @return - Returns itself.
     */
    public function setAll(
      ?KeyedTraversable<int, Tv> $it,
    )[write_props]: Vector<Tv>;

    /**
     * Removes all the elements from the current `Vector`.
     *
     * Future changes made to the current `Vector` ARE reflected in the
     * returned `Vector`, and vice-versa.
     *
     * @return - Returns itself.
     */
    <<__NoAutoLikes>>
    public function clear()[write_props]: Vector<Tv>;

    /**
     * Determines if the specified key is in the current `Vector`.
     *
     * @return - `true` if the specified key is present in the current `Vector`;
     *           returns `false` otherwise.
     *
     * @guide /hack/generics/constraints
     */
    public readonly function containsKey(mixed $k)[]: bool;

    /**
     * Appends a value to the end of the current `Vector`, assigning it the next
     * available integer key.
     *
     * If you want to overwrite the value for an existing key, use `set()`.
     *
     * `$vec->add($v)` is semantically equivalent to `$vec[] = $v` (except that
     * `add()` returns the current `Vector`).
     *
     * Future changes made to the current `Vector` ARE reflected in the
     * returned `Vector`, and vice-versa.
     *
     * If `$v` is an object, future changes to the added element ARE reflected in
     * `$v`, and vice versa.
     *
     * @param $v - The value to add to the end of the current `Vector`
     *
     * @return - Returns itself.
     */
    public function add(Tv $value)[write_props]: Vector<Tv>;

    /**
     * For every element in the provided `Traversable`, append a value into this
     * `Vector`, assigning the next available integer key for each.
     *
     * If you want to overwrite the values for existing keys, use `setAll()`.
     *
     * Future changes made to the current `Vector` ARE reflected in the
     * returned `Vector`, and vice-versa.
     *
     * @param $k - The `Traversable` with the new values to set. If `null` or an
     *             empty `Traversable` are provided, no changes are made.
     *
     * @return - Returns itself.
     */
    public function addAll(?Traversable<Tv> $it)[write_props]: Vector<Tv>;

    /**
     * Adds the keys of the specified container to the current `Vector`.
     *
     * For every key in the provided `KeyedContainer`, append that key into
     * the current `Vector`, assigning the next available integer key for each.
     *
     * Future changes made to the current `Vector` ARE reflected in the
     * returned `Vector`, and vice-versa.
     *
     * @param $container - The `KeyedContainer` with the new keys to add.
     *
     * @return - Returns itself.
     */
    <<__NoAutoLikes>>
    public function addAllKeysOf(
      ?KeyedContainer<Tv, mixed> $container,
    )[write_props]: Vector<Tv> where Tv as arraykey;

    /**
     * Removes the key/value pair with the specified key from the current
     * `Vector`.
     *
     * This will cause elements with higher keys to be assigned a new key that is
     * one less than their previous key.  That is, values with keys `$k + 1` to
     * `n - 1` will be given new keys `$k` to `n - 2`, where n is the length of
     * the current `Vector` before the call to `removeKey()`.
     *
     * If `$k` is negative, or `$k` is greater than the largest key in the current
     * `Vector`, no changes are made.
     *
     * Future changes made to the current `Vector` ARE reflected in the
     * returned `Vector`, and vice-versa.
     *
     * @param $k - The key of the key/value pair to remove.
     *
     * @return - Returns itself.
     */
    public function removeKey(int $k)[write_props]: Vector<Tv>;

    /**
     * Remove the last element of the current `Vector` and return it.
     *
     * This function throws an exception if the current `Vector` is empty.
     *
     * The current `Vector` will have `n - 1` elements after this operation, where
     * `n` is the number of elements in the current `Vector` prior to the call to
     * `pop()`.
     *
     * @return - The value of the last element.
     */
    public function pop()[write_props]: Tv;

    /**
     * Resize the current `Vector`.
     *
     * Resize the current `Vector` to contain `$sz` elements. If `$sz` is smaller
     * than the current size of the current `Vector`, elements are removed from
     * the end of the current `Vector`. If `$sz` is greater than the current size
     * of the current `Vector`, the current `Vector` is extended by appending as
     * many copies of `$value` as needed to reach a size of `$sz` elements.
     *
     * `$value` can be `null`.
     *
     * If `$sz` is less than zero, an exception is thrown.
     *
     * @param $sz - The desired size of the current `Vector`.
     * @param $value - The value to use as the filler if we are increasing the
     *                 size of the current `Vector`.
     */
    public function resize(int $sz, Tv $value)[write_props]: void;

    /**
     * Reserves enough memory to accommodate a given number of elements.
     *
     * Reserves enough memory for `$sz` elements. If `$sz` is less than or
     * equal to the current capacity of the current `Vector`, this method does
     * nothing.
     *
     * If `$sz` is less than zero, an exception is thrown.
     *
     * @param $sz - The pre-determined size you want for the current `Vector`.
     */
    public function reserve(int $sz)[]: void;

    /**
     * Returns an iterator that points to beginning of the current `Vector`.
     *
     * @return - A `KeyedIterator` that allows you to traverse the current
     *           `Vector`.
     */
    public function getIterator()[]: KeyedIterator<int, Tv>;

    /**
     * Reverse the elements of the current `Vector` in place.
     */
    public function reverse()[write_props]: void;

    /**
     * Splice the current `Vector` in place.
     *
     * This function provides the functionality of
     * [`array_splice()`](http://php.net/manual/en/function.array-splice.php)
     * for `Vector`s (except that `splice()` does not permit specifying
     * replacement values.  If a third ("replacement values") parameter is
     * specified, an exception is thrown.
     *
     *
     *
     * Note that this function modifies the current `Vector` in place.
     *
     * @param $offset - The (0-based) key at which to begin the splice. If
     *                  negative, then it starts that far from the end of the
     *                  current `Vector`.
     * @param $len - The length of the splice. If `null`, then the current
     *               `Vector` is spliced until its end.
     *
     * @link http://php.net/manual/en/function.array-splice.php
     */
    public function splice(int $offset, ?int $len = null)[write_props]: void;

    /**
     * Returns the index of the first element that matches the search value.
     *
     * If no element matches the search value, this function returns -1.
     *
     * @param $search_value - The value that will be searched for in the current
     *                        `Vector`.
     *
     * @return - The key (index) where that value is found; -1 if it is not found.
     *
     * @guide /hack/generics/constraints
     */
    public readonly function linearSearch(mixed $search_value)[]: int;

    /**
     * Shuffles the values of the current `Vector` randomly in place.
     */
    public function shuffle()[leak_safe]: void;

    /**
     * Returns a `Vector` containing the values from the specified `array`.
     *
     * This function is deprecated. Use `new Vector($arr)` instead.
     *
     * @param $arr - The `array` to convert to a `Vector`.
     *
     * @return - A `Vector` with the values from the provided `array`.
     */
    <<__Deprecated('Use `new Vector($arr)` instead.'), __NoAutoLikes>>
    public static function fromArray(darray<arraykey, Tv> $arr): Vector<Tv>;

    /**
     * Creates a `Vector` from the given `Traversable`, or an empty `Vector` if
     * `null` is passed.
     *
     * This is the static method version of the `Vector::__construct()`
     * constructor.
     *
     * @param $items - Any `Traversable` object from which to create a `Vector`
     *                 (e.g., `array`). If `null`, then an empty `Vector` is
     *                 created.
     *
     * @return - A `Vector` with the values from the `Traversable`; or an empty
     *           `Vector` if the `Traversable` is `null`.
     */
    <<__NoAutoLikes>>
    public static function fromItems(?Traversable<Tv> $items)[]: Vector<Tv>;

    /**
     * Creates a `Vector` from the keys of the specified container.
     *
     * Every key in the provided `KeyedContainer` will appear sequentially in the
     * returned `Vector`, with the next available integer key assigned to each.
     *
     * @param $container - The container with the keys used to create the
     *                     `Vector`.
     *
     * @return - A `Vector` built from the keys of the specified container.
     */
    <<__NoAutoLikes>>
    public static function fromKeysOf<Tk as arraykey>(
      ?KeyedContainer<Tk, mixed> $container,
    )[]: Vector<Tk>;

    /**
     * Returns the `string` version of the current `Vector`, which is `"Vector"`.
     *
     * @return - The string `"Vector"`.
     */
    public function __toString()[]: string;

    /**
     * Returns an `Iterable` view of the current `Vector`.
     *
     * The `Iterable` returned is one that produces the values from the current
     * `Vector`.
     *
     * @return - The `Iterable` view of the current `Vector`.
     */
    public function items()[]: Iterable<Tv>;
    public function toVArray()[]: varray<Tv>;
    public function toDArray()[]: darray<int, Tv>;
  }

} // namespace HH

namespace {

  /**
   * @internal
   *
   * Methods and functions should take and return the KeyedIterator interface.
   */
  class VectorIterator<+Tv> implements HH\KeyedIterator<int, Tv> {
    public function __construct()[];
    public function current()[]: Tv;
    public function key()[]: int;
    public function valid()[]: bool;
    public function next()[write_props]: void;
    public function rewind()[write_props]: void;
  }

} // namespace
