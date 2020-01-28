<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/**
 * This file provides type information for some of HHVM's builtin classes.
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

namespace HH {

/**
 * `ImmSet` is an immutable, ordered set-style collection. HHVM provides a
 * native implementation for this class. The PHP class definition below is not
 * actually used at run time; it is simply provided for the typechecker and
 * for developer reference.
 *
 * An `ImmSet` cannot be mutated. No elements can be added or removed from it,
 * nor can elements be overwritten using assignment (i.e. `$s[$k] = $v` is
 * not allowed).
 *
 * Construct it with a `Traversable`:
 *
 * ```
 * $a = array(1, 2);
 * $s = new ImmSet($a);
 * ```
 *
 * or use the literal syntax:
 *
 * ```
 * $s = ImmSet {1, 2};
 * ```
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/classes
 */

final class ImmSet<+Tv as arraykey> implements \ConstSet<Tv> {
  /**
   * Creates an `ImmSet` from the given `Traversable`, or an empty `ImmSet` if
   * `null` is passed.
   *
   * @param $it - any `Traversable` object from which to create the `ImmSet`
   *              (e.g., `array`). If `null`, then an empty `ImmSet` is created.
   */
  <<__Rx, __AtMostRxAsArgs>>
  public function __construct(<<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\Traversable::class)>> ?Traversable<Tv> $it);

  /**
   * Checks if the current `ImmSet` is empty.
   *
   * @return - `true` if the current `ImmSet` is empty; `false` otherwise.
   */
  <<__Rx, __MaybeMutable>>
  public function isEmpty(): bool;

  /**
   * Provides the number of elements in the current `ImmSet`.
   *
   * @return - The number of elements in the current `ImmSet`.
   */
  <<__Rx, __MaybeMutable>>
  public function count(): int;

  /**
   * Determines if the specified value is in the current `ImmSet`.
   *
   * @param $v - The value to check.
   * @return - `true` if the specified value is present in the current `ImmSet`;
   *           `false` otherwise.
   */
  <<__Rx, __MaybeMutable>>
  public function contains(arraykey $k): bool;

  /**
   * Returns an `array` containing the values from the current `ImmSet`.
   *
   * For the returned `array`, each key is the same as its associated value.
   *
   * @return - an `array` containing the values from the current `ImmSet`,
   *           where each key of the `array` are the same as each value.
   */
  <<__Rx, __MaybeMutable, __PHPStdLib>>
  public function toArray(): array<Tv, Tv>;

  /**
   * Returns an `array` containing the values from the current `ImmSet`.
   *
   * `Set`s don't have keys. So this method just returns the values.
   *
   * This method is interchangeable with `toValuesArray()`.
   *
   * @return - an integer-indexed `array` containing the values from the
   *           current `ImmSet`.
   */
  <<__Rx, __MaybeMutable>>
  public function toKeysArray(): varray<Tv>;

  /**
   * Returns an `array` containing the values from the current `ImmSet`.
   *
   * This method is interchangeable with `toKeysArray()`.
   *
   * @return - an integer-indexed `array` containing the values from the
   *           current `ImmSet`.
   */
  <<__Rx, __MaybeMutable>>
  public function toValuesArray(): varray<Tv>;

  /**
   * Returns an iterator that points to beginning of the current `ImmSet`.
   *
   * The keys and values when iterating through the `KeyedIterator` will be
   * identical since `ImmSet`s have no keys, the values are used as keys.
   *
   * @return - A `KeyedIterator` that allows you to traverse the current
   *           `ImmSet`.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function getIterator(): \HH\Rx\KeyedIterator<arraykey, Tv>;

  /**
   * Creates an `ImmSet` from the given `Traversable`, or an empty `ImmSet` if
   * `null` is passed.
   *
   * This is the static method version of the `ImmSet::__construct()`
   * constructor.
   *
   * @param $items - any `Traversable` object from which to create an `ImmSet`
   *                 (e.g., `array`). If `null`, then an empty `ImmSet` is
   *                 created.
   *
   * @return - An `ImmSet` with the values from the `Traversable`; or an empty
   *           `ImmSet` if the `Traversable` is `null`.
   */
  <<__Rx, __AtMostRxAsArgs>>
  public static function fromItems(<<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\Traversable::class)>> ?Traversable<Tv> $items): ImmSet<Tv>;

  /**
   * Returns an `ImmSet` containing all the values from the specified
   * `array`(s).
   *
   * @param ...$argv - The `array`(s) to convert to an `ImmSet`.
   *
   * @return - An `ImmSet` with the values from the passed `array`(s).
   */
  <<__Rx>>
  public static function fromArrays(...$argv): ImmSet<Tv>;

  /**
   * Creates an `ImmSet` from the keys of the specified container.
   *
   * The keys of the container will be the values of the `ImmSet`.
   *
   * @param $container - The container with the keys used to create the
   *                     `ImmSet`.
   *
   * @return - An `ImmSet` built from the keys of the specified container.
   */
  <<__Rx>>
  public static function fromKeysOf<Tk as arraykey>(
    ?KeyedContainer<Tk,mixed> $container
  ): ImmSet<Tk>;

  /**
   * Returns the `string` version of this `ImmSet`, which is `"ImmSet"`.
   *
   * @return - The `string` `"ImmSet"`.
   */
  <<__Rx, __MaybeMutable>>
  public function __toString(): string;

  /**
   * Returns an immutable vector (`ImmVector`) with the values of the current
   * `ImmSet`.
   *
   * @return - an `ImmVector` (integer-indexed) with the values of the current
   *           `ImmSet`.
   */
  <<__Rx, __MaybeMutable>>
  public function toImmVector(): ImmVector<Tv>;

  /**
   * Returns an immutable map (`ImmMap`) based on the values of the current
   * `ImmSet`.
   *
   * Each key of the `Map` will be the same as its value.
   *
   * @return - an `ImmMap` that that contains the values of the current
   *           `ImmSet`, with each key of the `ImmMap` being the same as its
   *           value.
   */
  <<__Rx, __MaybeMutable>>
  public function toImmMap(): ImmMap<arraykey, Tv>;

  /**
   * Returns an immutable copy (`ImmSet`) of the current `ImmSet`.
   *
   * This function is interchangeable with `immutable()`.
   *
   * @return - an `ImmSet` that is a copy of the current `ImmSet`.
   */
  <<__Rx, __MaybeMutable>>
  public function toImmSet(): ImmSet<Tv>;

  /**
   * Returns an immutable copy (`ImmSet`) of the current `ImmSet`.
   *
   * This method is interchangeable with `toImmSet()`.
   *
   * @return - an `ImmSet` that is a copy of the current `ImmSet`.
   */
  <<__Rx, __MaybeMutable>>
  public function immutable(): ImmSet<Tv>;

  /**
   * Returns an Iterable view of the current `ImmSet`.
   *
   * The `Iterable` returned is one that produces the values from the current
   * `ImmSet`.
   *
   * @return - The `Iterable` view of the current `ImmSet`.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function items(): \HH\Rx\Iterable<Tv>;

  /**
   * Returns an `ImmVector` containing the values of the current `ImmSet`.
   *
   * This method is interchangeable with `toImmVector()` and `keys()`.
   *
   * @return - an `ImmVector` containing the values of the current `ImmSet`.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function values(): ImmVector<Tv>;

  /**
   * Returns an `ImmVector` containing the values of this `ImmSet`.
   *
   * `ImmSet`s don't have keys, so this will return the values.
   *
   * This method is interchangeable with `toImmVector()` and `values()`.
   *
   * @return - an `ImmVector` containing the values of the current `ImmSet`.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function keys(): ImmVector<arraykey>;

  /**
   * Returns a lazy, access elements only when needed view of the current
   * `ImmSet`.
   *
   * Normally, memory is allocated for all of the elements of an `ImmSet`. With
   * a lazy view, memory is allocated for an element only when needed or used
   * in a calculation like in `map()` or `filter()`.
   *
   * @return - an `KeyedIterable` representing the lazy view into the current
   *           `ImmSet`, where the keys are the same as the values.
   *
   * @guide /hack/collections/examples
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function lazy(): \HH\Rx\KeyedIterable<arraykey, Tv>;

  /**
   * Returns an `ImmSet` containing the values after an operation has been
   * applied to each value in the current `ImmSet`.
   *
   * Every value in the current `ImmSet` is affected by a call to `map()`,
   * unlike `filter()` where only values that meet a certain criteria are
   * affected.
   *
   * @param $callback - The callback containing the operation to apply to the
   *                    current `ImmSet` values.
   *
   * @return - a `ImmSet` containing the values after a user-specified operation
   *           is applied.
   *
   * @guide /hack/collections/examples
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function map<Tu as arraykey>(<<__AtMostRxAsFunc>>(function(Tv): Tu) $callback): ImmSet<Tu>;

  /**
   * Returns an `ImmSet` containing the values after an operation has been
   * applied to each "key" and value in the current `ImmSet`.
   *
   * Since `ImmSet`s don't have keys, the callback uses the values as the keys
   * as well.
   *
   * Every value in the current `ImmSet` is affected by a call to
   * `mapWithKey()`, unlike `filterWithKey()` where only values that meet a
   * certain criteria are affected.
   *
   * @param $callback - The callback containing the operation to apply to the
   *                    current `ImmSet` keys and values.
   *
   * @return - an `ImmSet` containing the values after a user-specified
   *           operation on the current `ImmSet`'s values is applied.
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function mapWithKey<Tu as arraykey>(<<__AtMostRxAsFunc>>(function(arraykey, Tv): Tu) $callback): ImmSet<Tu>;

  /**
   * Returns an `ImmSet` containing the values of the current `ImmSet` that
   * meet a supplied condition applied to each value.
   *
   * Only values that meet a certain criteria are affected by a call to
   * `filter()`, while all values are affected by a call to `map()`.
   *
   * @param $callback - The callback containing the condition to apply to the
   *                    current `ImmSet` values.
   *
   * @return - an `ImmSet` containing the values after a user-specified
   *           condition is applied.
   *
   * @guide /hack/collections/examples
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filter(<<__AtMostRxAsFunc>>(function(Tv): bool) $callback): ImmSet<Tv>;

  /**
   * Returns an `ImmSet` containing the values of the current `ImmSet` that
   * meet a supplied condition applied to its "keys" and values.
   *
   * Since `ImmSet`s don't have keys, the callback uses the values as the keys
   * as well.
   *
   * Only values that meet a certain criteria are affected by a call to
   * `filterWithKey()`, while all values are affected by a call to
   * `mapWithKey()`.
   *
   * @param $callback - The callback containing the condition to apply to the
   *                    current `ImmSet` keys and values.
   *
   * @return - an `ImmSet` containing the values after a user-specified
   *           condition is applied to the values of the current `ImmSet`.
   *
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filterWithKey(<<__AtMostRxAsFunc>>(function(arraykey, Tv): bool) $callback):
    ImmSet<Tv>;

  /**
   * Throws an exception unless the current `ImmSet` or the `Traversable` is
   * empty.
   *
   * Since `ImmSet`s only support integers or strings as values, we cannot
   * have a `Pair` as an `ImmSet` value. So in order to avoid an
   * `InvalidArgumentException`, either the current `ImmSet` or the
   * `Traversable` must be empty so that we actually return an empty `ImmSet`.
   *
   * @param $traversable - The `Traversable` to use to combine with the
   *                       elements of the current `ImmSet`.
   *
   * @return - The `ImmSet` that combines the values of the current `ImmSet`
   *           with the provided `Traversable`; one of these must be empty or
   *           an exception is thrown.
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function zip<Tu>(
    <<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\Traversable::class)>> Traversable<Tu> $traversable
  ): ImmSet<nothing>;

  /**
   * Returns an `ImmSet` containing the first n values of the current `ImmSet`.
   *
   * The returned `ImmSet` will always be a proper subset of the current
   * `ImmSet`.
   *
   * `n` is 1-based. So the first element is 1, the second 2, etc.
   *
   * @param $n - The last element that will be included in the returned
   *             `ImmSet`.
   *
   * @return - An `ImmSet` that is a proper subset of the current `ImmSet` up
   *           to `n` elements.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function take(int $n): ImmSet<Tv>;

  /**
   * Returns an `ImmSet` containing the values of the current `ImmSet` up to
   * but not including the first value that produces `false` when passed to the
   * specified callback.
   *
   * The returned `ImmSet` will always be a proper subset of the current
   * `ImmSet`.
   *
   * @param $fn - The callback that is used to determine the stopping condition.
   *
   * @return - An `ImmSet` that is a proper subset of the current `ImmSet` up
   *           until the callback returns `false`.
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function takeWhile(<<__AtMostRxAsFunc>>(function(Tv): bool) $fn): ImmSet<Tv>;

  /**
   * Returns an `ImmSet` containing the values after the `n`-th element of the
   * current `ImmSet`.
   *
   * The returned `ImmSet` will always be a proper subset of the current
   * `ImmSet`.
   *
   * `n` is 1-based. So the first element is 1, the second 2, etc.
   *
   * @param $n - The last element to be skipped; the `$n+1` element will be the
   *             first one in the returned `ImmSet`.
   *
   * @return - An `ImmSet` that is a proper subset of the current `ImmSet`
   *           containing values after the specified `n`-th element.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function skip(int $n): ImmSet<Tv>;

  /**
   * Returns an `ImmSet` containing the values of the current `ImmSet` starting
   * after and including the first value that produces `true` when passed to
   * the specified callback.
   *
   * The returned `ImmSet` will always be a proper subset of the current
   * `ImmSet`.
   *
   * @param $fn - The callback used to determine the starting element for the
   *              `ImmSet`.
   *
   * @return - An `ImmSet` that is a proper subset of the current `ImmSet`
   *           starting after the callback returns `true`.
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function skipWhile(<<__AtMostRxAsFunc>>(function(Tv): bool) $fn): ImmSet<Tv>;

  /**
   * Returns a subset of the current `ImmSet` starting from a given key up to,
   * but not including, the element at the provided length from the starting
   * key.
   *
   * `$start` is 0-based. `$len` is 1-based. So `slice(0, 2)` would return the
   * elements at key 0 and 1.
   *
   * The returned `ImmSet` will always be a proper subset of the current
   * `ImmSet`.
   *
   * @param $start - The starting value in the current `ImmSet` for the
   *                 returned `ImmSet`.
   * @param $len - The length of the returned `ImmSet`.
   *
   * @return - An `ImmSet` that is a proper subset of the current `ImmSet`
   *           starting at `$start` up to but not including the element
   *           `$start + $len`.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function slice(int $start, int $len): ImmSet<Tv>;

  /**
   * Returns an `ImmVector` that is the concatenation of the values of the
   * current `ImmSet` and the values of the provided `Traversable`.
   *
   * The values of the provided `Traversable` is concatenated to the end of the
   * current `ImmSet` to produce the returned `ImmVector`.
   *
   * @param $traversable - The `Traversable` to concatenate to the current
   *                       `ImmSet`.
   *
   * @return - The concatenated `ImmVector`.
   *
   * @guide /hack/generics/constraints
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function concat<Tu super Tv>(
    <<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\Traversable::class)>> Traversable<Tu> $traversable
  ): ImmVector<Tu>;

  /**
   * Returns the first value in the current `ImmSet`.
   *
   * This method is interchangeable with `firstKey()`.
   *
   * @return - The first value in the current `ImmSet`, or `null` if the
   *           current `ImmSet` is empty.
   */
  <<__Rx, __MaybeMutable>>
  public function firstValue(): ?Tv;

  /**
   * Returns the first "key" in the current `ImmSet`.
   *
   * Since `ImmSet`s do not have keys, it returns the first value.
   *
   * This method is interchangeable with `firstValue()`.
   *
   * @return - The first value in the current `ImmSet`, or `null` if the
   *           current `ImmSet` is empty.
   */
  <<__Rx, __MaybeMutable>>
  public function firstKey(): ?arraykey;

  /**
   * Returns the last value in the current `ImmSet`.
   *
   * This method is interchangeable with `lastKey()`.
   *
   * @return - The last value in the current `ImmSet`, or `null` if the current
   *           `ImmSet` is empty.
   */
  <<__Rx, __MaybeMutable>>
  public function lastValue(): ?Tv;

  /**
   * Returns the last "key" in the current `ImmSet`.
   *
   * Since `ImmSet`s do not have keys, it returns the last value.
   *
   * This method is interchangeable with `lastValue()`.
   *
   * @return - The last value in the current `ImmSet`, or `null` if the current
   *           `ImmSet` is empty.
   */
  <<__Rx, __MaybeMutable>>
  public function lastKey(): ?arraykey;

  <<__Rx, __MaybeMutable>> /* HH_FIXME[0002] */
  public function toVArray(): varray<Tv>;
  <<__Rx, __MaybeMutable>> /* HH_FIXME[0001] */
  public function toDArray(): darray<Tv, Tv>;
}

} // namespace HH
