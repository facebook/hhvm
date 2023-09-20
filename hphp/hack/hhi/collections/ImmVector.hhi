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
   * **Prefer `vec` over `ImmVector`**.
   *
   * `ImmVector` is an immutable `Vector`. HHVM provides a native implementation
   * for this class. The PHP class definition below is not actually used at run
   * time; it is simply provided for the typechecker and for developer reference.
   *
   * An `ImmVector` cannot be mutated. No elements can be added to it or removed
   * from it, nor can elements be overwritten using assignment (i.e. `$c[$k] = $v`
   * is not allowed).
   *
   * ```
   * $v = Vector {1, 2, 3};
   * $fv = $v->toImmVector();
   * ```
   *
   * construct it with a `Traversable`:
   *
   * ```
   * $a = vec[1, 2, 3];
   * $fv = new ImmVector($a);
   * ```
   *
   * or use the literal syntax:
   *
   * ```
   * $fv = ImmVector {1, 2, 3};
   * ```
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/classes
   */
  final class ImmVector<+Tv> implements \ConstVector<Tv> {
    /**
     * Creates an `ImmVector` from the given `Traversable`, or an empty
     * `ImmVector` if `null` is passed.
     *
     * @param $it - Any `Traversable` object from which to create the `ImmVector`
     *              (e.g., `array`). If `null`, then an empty `ImmVector` is
     *              created.
     */
    public function __construct(?Traversable<Tv> $it)[];

    /**
     * Checks if the current `ImmVector` is empty.
     *
     * @return - `true` if the current `ImmVector` is empty; `false` otherwise.
     */
    public readonly function isEmpty()[]: bool;

    /**
     * Returns the number of elements in the current `ImmVector`.
     *
     * @return - The number of elements in the current `ImmVector`.
     */
    public readonly function count()[]: int;

    /**
     * Returns the value at the specified key in the current `ImmVector`.
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
     * Returns the value at the specified key in the current `ImmVector`.
     *
     * If the key is not present, `null` is returned. If you would rather have an
     * exception thrown when a key is not present, then use `at()`.
     *
     * @param $k - The key for which to retrieve the value.
     *
     * @return - The value at the specified key; or `null` if the key does not
     *           exist.
     */
    public function get(int $k)[]: ?Tv;

    /**
     * Determines if the specified key is in the current `ImmVector`.
     *
     * @return - `true` if the specified key is present in the current
     *           `ImmVector`; `false` otherwise.
     *
     * @guide /hack/generics/constraints
     */
    public readonly function containsKey(mixed $k)[]: bool;

    /**
     * Returns an `array` containing the values from the current `ImmVector`.
     *
     * This method is interchangeable with `toArray()`.
     *
     * @return - An `array` containing the values from the current `ImmVector`.
     */
    public function toValuesArray()[]: varray<Tv>;

    /**
     * Returns an `array` whose values are the keys from the current `ImmVector`.
     *
     * @return - An `array` with the integer keys from the current `ImmVector`.
     */
    public readonly function toKeysArray()[]: varray<int>;

    /**
     * Returns an iterator that points to beginning of the current `ImmVector`.
     *
     * @return - A `KeyedIterator` that allows you to traverse the current
     *           `ImmVector`.
     */
    public function getIterator()[]: KeyedIterator<int, Tv>;

    /**
     * Returns the index of the first element that matches the search value.
     *
     * If no element matches the search value, this function returns -1.
     *
     * @param $search_value - The value that will be searched for in the current
     *                        `ImmVector`.
     *
     * @return - The key (index) where that value is found; -1 if it is not found.
     *
     * @guide /hack/generics/constraints
     */
    public readonly function linearSearch(mixed $search_value)[]: int;

    /**
     * Creates an `ImmVector` from the given `Traversable`, or an empty
     * `ImmVector` if `null` is passed.
     *
     * This is the static method version of the `ImmVector::__construct()`
     * constructor.
     *
     * @param $items - Any `Traversable` object from which to create an
     *                 `ImmVector` (e.g., `array`). If `null`, then an empty
     *                 `ImmVector` is created.
     *
     * @return - An `ImmVector` with the values from the `Traversable`; or an
     *           empty `ImmVector` if the `Traversable` is `null`.
     */
    public static function fromItems(?Traversable<Tv> $items)[]: ImmVector<Tv>;

    /**
     * Creates an `ImmVector` from the keys of the specified container.
     *
     * Every key in the provided `KeyedContainer` will appear sequentially in the
     * returned `ImmVector`, with the next available integer key assigned to each.
     *
     * @param $container - The container with the keys used to create the
     *                     current `ImmVector`.
     *
     * @return - An `ImmVector` built from the keys of the specified container.
     */
    public static function fromKeysOf<Tk as arraykey>(
      ?KeyedContainer<Tk, mixed> $container,
    )[]: ImmVector<Tk>;

    /**
     * Returns the `string` version of the current `ImmVector`, which is
     * `"ImmVector"`.
     *
     * @return - The `string` `"ImmVector"`.
     */
    public function __toString()[]: string;

    /**
     * Returns an `Iterable` view of the current `ImmVector`.
     *
     * The `Iterable` returned is one that produces the values from the current
     * `ImmVector`.
     *
     * @return - The `Iterable` view of the current `ImmVector`.
     */
    public function items()[]: Iterable<Tv>;

    /**
     * Returns the current `ImmVector`.
     *
     * Unlike `Vector`'s `toVector()` method, this does not actually return a copy
     * of the current `ImmVector`. Since `ImmVector`s are immutable, there is no
     * reason to pay the cost of creating a copy of the current `ImmVector`.
     *
     * This method is interchangeable with `immutable()`.
     *
     * This method is NOT interchangeable with `values()`. `values()` returns a
     * new `ImmVector` that is a copy of the current `ImmVector`, and thus incurs
     * both the cost of copying the current `ImmVector`, and the memory space
     * consumed by the new `ImmVector`.  This may be significant, for large
     * `ImmVector`s.
     *
     * @return - The current `ImmVector`.
     */
    public function toImmVector()[]: ImmVector<Tv>;

    /**
     * Returns an immutable integer-keyed Map (`ImmMap`) based on the elements of
     * the current `ImmVector`.
     *
     * The keys are `0... count() - 1`.
     *
     * @return - An integer-keyed `ImmMap` with the values of the current
     *           `ImmVector`.
     */
    public function toImmMap()[]: ImmMap<int, Tv>;

    /**
     * Returns an immutable Set (`ImmSet`) with the values of the current
     * `ImmVector`.
     *
     * @return - An `ImmSet` with the current values of the current `ImmVector`.
     */
    public function toImmSet()[]: ImmSet<Tv> where Tv as arraykey;

    /**
     * Returns the current `ImmVector`.
     *
     * Unlike `Vector`'s `toVector()` method, this does not actually return a copy
     * of the current `ImmVector`. Since `ImmVector`s are immutable, there is no
     * reason to pay the cost of creating a copy of the current `ImmVector`.
     *
     * This method is interchangeable with `toImmVector()`.
     *
     * This method is NOT interchangeable with `values()`. `values()` returns a
     * new `ImmVector` that is a copy of the current `ImmVector`, and thus incurs
     * both the cost of copying the current `ImmVector`, and the memory space
     * consumed by the new `ImmVector`.  This may be significant, for large
     * `ImmVector`s.
     *
     * @return - The current `ImmVector`.
     */
    public function immutable()[]: ImmVector<Tv>;

    /**
     * Returns a lazy, access-elements-only-when-needed view of the current
     * `ImmVector`.
     *
     * Normally, memory is allocated for all of the elements of an `ImmVector`.
     * With a lazy view, memory is allocated for an element only when needed or
     * used in a calculation like in `map()` or `filter()`.
     *
     * @return - An integer-keyed `KeyedIterable` representing the lazy view into
     *           the current `ImmVector`.
     *
     * @guide /hack/collections/examples
     */
    public function lazy()[]: KeyedIterable<int, Tv>;

    /**
     * Returns a new `ImmVector` containing the values of the current `ImmVector`;
     * that is, a copy of the current `ImmVector`.
     *
     * This method is NOT interchangeable with `toImmVector()` and `immutable()`.
     * `toImmVector()` and `immutable()` return the current `ImmVector`, and do
     * not incur the cost of copying the current `ImmVector`, or the memory space
     * consumed by the new `ImmVector`.  This may be significant, for large
     * `ImmVector`s.
     *
     * @return - A new `ImmVector` containing the values of the current
     *           `ImmVector`.
     */
    public function values()[]: ImmVector<Tv>;

    /**
     * Returns an `ImmVector` containing the keys, as values, of the current
     * `ImmVector`.
     *
     * @return - An `ImmVector` containing, as values, the integer keys of the
     *           current `ImmVector`.
     */
    public readonly function keys()[]: ImmVector<int>;

    /**
     * Returns an `ImmVector` containing the results of applying an operation to
     * each value in the current `ImmVector`.
     *
     * `map()`'s result contains a value for every value in the current
     * `ImmVector`; unlike `filter()`, where only values that meet a certain
     * criterion are included in the resulting `ImmVector`.
     *
     * @param $fn - The callback containing the operation to apply to the
     *                    current `ImmVector`'s values.
     *
     * @return - An `ImmVector` containing the results of applying a user-specified
     *           operation to each value of the current `ImmVector` in turn.
     *
     * @guide /hack/collections/examples
     */
    public function map<Tu>((function(Tv)[_]: Tu) $fn)[ctx $fn]: ImmVector<Tu>;

    /**
     * Returns an `ImmVector` containing the results of applying an operation to
     * each key/value pair in the current `ImmVector`.
     *
     * `mapWithKey()`'s result contains a value for every key/value pair in the
     * current `ImmVector`; unlike `filterWithKey()`, where only values whose
     * key/value pairs meet a certain criterion are included in the resulting
     * `ImmVector`.
     *
     * @param $fn - The callback containing the operation to apply to the
     *                    current `ImmVector`'s key/value pairs.
     *
     * @return - An `ImmVector` containing the results of applying a
     *           user-specified operation to each key/value pair of the current
     *           `ImmVector` in turn.
     */
    public function mapWithKey<Tu>(
      (function(int, Tv)[_]: Tu) $fn,
    )[ctx $fn]: ImmVector<Tu>;

    /**
     * Returns a `ImmVector` containing the values of the current `ImmVector` that
     * meet a supplied condition.
     *
     * `filter()`'s result contains only values that meet the provided criterion;
     * unlike `map()`, where a value is included for each value in the original
     * `ImmVector`.
     *
     * @param $fn - The callback containing the condition to apply to the
     *                    current `ImmVector` values.
     *
     * @return - An `ImmVector` containing the values after a user-specified
     *           condition is applied.
     *
     * @guide /hack/collections/examples
     */
    public function filter((function(Tv)[_]: bool) $fn)[ctx $fn]: ImmVector<Tv>;

    /**
     * Returns an `ImmVector` containing the values of the current `ImmVector`
     * that meet a supplied condition applied to its keys and values.
     *
     * `filterWithKey()`'s result contains only values whose key/value pairs
     * satisfy the provided criterion; unlike `mapWithKey()`, which contains
     * results derived from every key/value pair in the original `ImmVector`.
     *
     * @param $fn - The callback containing the condition to apply to the
     *                    `ImmVector`'s key/value pairs. For each key/value pair,
     *                    the key is passed as the first parameter to the
     *                    callback, and the value is passed as the second
     *                    parameter.
     *
     * @return - An `ImmVector` containing the values of the current `ImmVector`
     *           for which a user-specified test condition returns true when
     *           applied to the corresponding key/value pairs.
     *
     */
    public function filterWithKey(
      (function(int, Tv)[_]: bool) $fn,
    )[ctx $fn]: ImmVector<Tv>;

    /**
     * Returns an `ImmVector` where each element is a `Pair` that combines the
     * element of the current `ImmVector` and the provided `Traversable`.
     *
     * If the number of elements of the current `ImmVector` are not equal to the
     * number of elements in the `Traversable`, then only the combined elements
     * up to and including the final element of the one with the least number of
     * elements is included.
     *
     * @param $traversable - The `Traversable` to use to combine with the
     *                       elements of the current `ImmVector`.
     *
     * @return - An `ImmVector` that combines the values of the current
     *           `ImmVector` with the provided `Traversable`.
     */
    public function zip<Tu>(
      Traversable<Tu> $traversable,
    )[]: ImmVector<Pair<Tv, Tu>>;

    /**
     * Returns an `ImmVector` containing the first `$n` values of the current
     * `ImmVector`.
     *
     * The returned `ImmVector` will always be a subset (but not necessarily a
     * proper subset) of the current `ImmVector`. If `$n` is greater than the
     * length of the current `ImmVector`, the returned `ImmVector` will contain
     * all elements of the current `ImmVector`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     * @param $n - The last element that will be included in the returned
     *             `ImmVector`.
     *
     * @return - An `ImmVector` that is a subset of the current `ImmVector` up to
     *           `$n` elements.
     */
    public function take(int $n)[]: ImmVector<Tv>;

    /**
     * Returns an `ImmVector` containing the values of the current `ImmVector` up
     * to but not including the first value that produces `false` when passed to
     * the specified callback. That is, takes the continuous prefix of values in
     * the current `ImmVector` for which the specified callback returns `true`.
     *
     * The returned `ImmVector` will always be a subset (but not necessarily a
     * proper subset) of the current `ImmVector`.
     *
     * @param $fn - The callback that is used to determine the stopping condition.
     *
     * @return - An `ImmVector` that is a subset of the current `ImmVector` up
     *           until when the callback returns `false`.
     */
    public function takeWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: ImmVector<Tv>;

    /**
     * Returns an `ImmVector` containing the values after the `$n`-th element of
     * the current `ImmVector`.
     *
     * The returned `ImmVector` will always be a subset (but not necessarily a
     * proper subset) of the current `ImmVector`. If `$n` is greater than or equal
     * to the length of the current `ImmVector`, the returned `ImmVector` will
     * contain no elements. If `$n` is negative, the returned `ImmVector` will
     * contain all elements of the current `ImmVector`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     * @param $n - The last element to be skipped; the `$n+1` element will be the
     *             first one in the returned `ImmVector`.
     *
     * @return - An `ImmVector` that is a subset of the current `ImmVector`
     *           containing values after the specified `$n`-th element.
     */
    public function skip(int $n)[]: ImmVector<Tv>;

    /**
     * Returns an `ImmVector` containing the values of the current `ImmVector`
     * starting after and including the first value that produces `false` when
     * passed to the specified callback. That is, skips the continuous prefix of
     * values in the current `ImmVector` for which the specified callback returns
     * `true`.
     *
     * The returned `ImmVector` will always be a subset (but not necessarily a
     * proper subset) of the current `ImmVector`.
     *
     * @param $fn - The callback used to determine the starting element for the
     *              returned `ImmVector`.
     *
     * @return - An `ImmVector` that is a subset of the current `ImmVector`
     *           starting with the value for which the callback first returns
     *           `false`.
     */
    public function skipWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: ImmVector<Tv>;

    /**
     * Returns a subset of the current `ImmVector` starting from a given key up
     * to, but not including, the element at the provided length from the
     * starting key.
     *
     * `$start` is 0-based. `$len` is 1-based. So `slice(0, 2)` would return the
     * elements at key 0 and 1.
     *
     * The returned `ImmVector` will always be a subset (but not necessarily a
     * proper subset) of the current `ImmVector`. If `$start` is greater than or
     * equal to the length of the current `Vector`, the returned `Vector` will
     * contain no elements.  If `$start` + `$len` is greater than or equal to the
     * length of the current `Vector`, the returned `Vector` will contain the
     * elements from `$start` to the end of the current `Vector`.
     *
     * If either `$start` or `$len` is negative, an exception is thrown.
     *
     * @param $start - The starting key of the current `ImmVector` at which to
     *                 begin the returned `ImmVector`.
     * @param $len - The length of the returned `ImmVector`.
     *
     * @return - An `ImmVector` that is a subset of the current `ImmVector`
     *           starting at `$start` up to but not including the element
     *           `$start + $len`.
     */
    public function slice(int $start, int $len)[]: ImmVector<Tv>;

    /**
     * Returns an `ImmVector` that is the concatenation of the values of the
     * current `ImmVector` and the values of the provided `Traversable`.
     *
     * The returned `ImmVector` is created from the values of the current
     * `ImmVector`, followed by the values of the provided `Traversable`.
     *
     * The returned `ImmVector` is a new object; the current `ImmVector` is
     * unchanged.
     *
     * @param $traversable - The `Traversable` to concatenate to the current
     *                       `ImmVector`.
     *
     * @return - A new `ImmVector` containing the values from `$traversable`
     *           concatenated to the values from the current `ImmVector`.
     *
     * @guide /hack/generics/constraints
     */
    public function concat<Tu super Tv>(
      Traversable<Tu> $traversable,
    )[]: ImmVector<Tu>;

    /**
     * Returns the first value in the current `ImmVector`.
     *
     * @return - The first value in the current `ImmVector`, or `null` if the
     *           current `ImmVector` is empty.
     */
    public function firstValue()[]: ?Tv;

    /**
     * Returns the first key in the current `ImmVector`.
     *
     * @return - The first key (an integer) in the current `ImmVector`, or `null`
     *           if the current `ImmVector` is empty.
     */
    public readonly function firstKey()[]: ?int;

    /**
     * Returns the last value in the current `ImmVector`.
     *
     * @return - The last value in the current `ImmVector`, or `null` if the
     *           current `ImmVector` is empty.
     */
    public function lastValue()[]: ?Tv;

    /**
     * Returns the last key in the current `ImmVector`.
     *
     * @return - The last key (an integer) in the current `ImmVector`, or `null`
     *           if the current `ImmVector` is empty.
     */
    public readonly function lastKey()[]: ?int;
    public function toVArray()[]: varray<Tv>;
    public function toDArray()[]: darray<int, Tv>;
  }

} // namespace HH
