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
 * `Map` is an ordered dictionary-style collection. HHVM provides a native
 * implementation for this class. The PHP class definition below is not
 * actually used at run time; it is simply provided for the typechecker and for
 * developer reference.
 *
 * Like all objects in PHP, `Map`s have reference-like semantics. When a caller
 * passes a `Map` to a callee, the callee can modify the `Map` and the caller
 * will see the changes. `Map`s do not have "copy-on-write" semantics.
 *
 * `Map`s preserve insertion order of key/value pairs. When iterating over a
 * `Map`, the key/value pairs appear in the order they were inserted. Also,
 * `Map`s do not automagically convert integer-like `string` keys (ex. `"123"`)
 * into integer keys.
 *
 * `Map`s only support `int` keys and `string` keys. If a key of a different
 * type is used, an exception will be thrown.
 *
 * `Map`s support `$m[$k]` style syntax for getting and setting values by key.
 * `Map`s also support `isset($m[$k])` and `empty($m[$k])` syntax, and they
 * provide similar semantics as arrays. Adding an element with square bracket
 * syntax `[]` is supported either by providing a key between the brackets or
 * a `Pair` on the right-hand side. e.g.,
 * `$m[$k] = $v` is supported
 * `$m[] = Pair {$k, $v}` is supported
 * `$m[] = $v` is not supported.
 *
 * `Map`s do not support iterating while new keys are being added or elements
 * are being removed. When a new key is added or an element is removed, all
 * iterators that point to the `Map` shall be considered invalid.
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/classes
 */

final class Map<Tk as arraykey, Tv> implements \MutableMap<Tk, Tv> {
  /**
   * Creates a `Map` from the given `KeyedTraversable`, or an empty `Map` if
   * `null` is passed.
   *
   * @param $it - any `Traversable` object from which to create a `Map`
   *              (e.g., `array`). If `null`, then an empty `Map` is created.
   */
  <<__Pure, __AtMostRxAsArgs>>
  public function __construct(<<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\KeyedTraversable::class)>> ?KeyedTraversable<Tk, Tv> $it);

  /**
   * Returns an `array` containing the values from the current `Map`.
   *
   * @return - an integer-indexed `array` containing the values from the
   *           current `Map`.
   */
  <<__Pure, __MaybeMutable>>
  public function toValuesArray(): varray<Tv>;

  /**
   * Returns an `array` whose values are the keys of the current `Map`.
   *
   * @return - an integer-indexed `array` containing the keys from the current
   *           `Map`.
   */
  <<__Pure, __MaybeMutable>>
  public function toKeysArray(): varray<Tk>;

  /**
   * Returns a `Vector` with the values of the current `Map`.
   *
   * @return - a `Vector` that contains the values of the current `Map`.
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function toVector(): Vector<Tv>;

  /**
   * Returns an immutable vector (`ImmVector`) with the values of the current
   * `Map`.
   *
   * @return - an `ImmVector` that is an immutable copy of the current `Map`.
   */
  <<__Pure, __MaybeMutable>>
  public function toImmVector(): ImmVector<Tv>;

  /**
   * Returns a deep copy of the current `Map`.
   *
   * @return - a `Map` that is a deep copy of the current `Map`.
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function toMap(): Map<Tk, Tv>;

  /**
   * Returns a deep, immutable copy (`ImmMap`) of the current `Map`.
   *
   * @return - an `ImmMap` that is a copy of this `Map`.
   */
  <<__Pure, __MaybeMutable>>
  public function toImmMap(): ImmMap<Tk, Tv>;

  /**
   * Returns a `Set` based on the values of the current `Map`.
   *
   * @return - a `Set` with the current values of the current `Map`.
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function toSet(): Set<Tv> where Tv as arraykey;

  /**
   * Returns an immutable set (`ImmSet`) based on the values of the current
   * `Map`.
   *
   * @return - an `ImmSet` with the current values of the current `Map`.
   */
  <<__Pure, __MaybeMutable>>
  public function toImmSet(): ImmSet<Tv> where Tv as arraykey;

  /**
   * Returns a deep, immutable copy (`ImmMap`) of this `Map`.
   *
   * This method is interchangeable with `toImmMap()`.
   *
   * @return - an `ImmMap` that is a deep copy of this `Map`.
   */
  <<__Pure, __MaybeMutable>>
  public function immutable(): ImmMap<Tk, Tv>;

  /**
   * Returns a lazy, access elements only when needed view of the current
   * `Map`.
   *
   * Normally, memory is allocated for all of the elements of the `Map`. With
   * a lazy view, memory is allocated for an element only when needed or used
   * in a calculation like in `map()` or `filter()`.
   *
   * @return - a `KeyedIterable` representing the lazy view into the current
   *           `Map`.
   *
   * @guide /hack/collections/examples
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function lazy(): \HH\Rx\KeyedIterable<Tk, Tv>;

  /**
   * Returns a `Vector` containing the values of the current `Map`.
   *
   * This method is interchangeable with `toVector()`.
   *
   * @return - a `Vector` containing the values of the current `Map`.
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function values(): Vector<Tv>;

  /**
   * Returns a `Vector` containing the keys of the current `Map`.
   *
   * @return - a `Vector` containing the keys of the current `Map`.
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function keys(): Vector<Tk>;

  /**
   * Returns a `Map` after an operation has been applied to each value in the
   * current `Map`.
   *
   * Every value in the current `Map` is affected by a call to `map()`, unlike
   * `filter()` where only values that meet a certain criteria are affected.
   *
   * The keys will remain unchanged from the current `Map` to the returned
   * `Map`.
   *
   * @param $callback - The callback containing the operation to apply to the
   *                    current `Map` values.
   *
   * @return - a `Map` containing key/value pairs after a user-specified
   *           operation is applied.
   *
   * @guide /hack/collections/examples
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function map<Tu>(<<__AtMostRxAsFunc>>(function(Tv): Tu) $callback): Map<Tk, Tu>;

  /**
   * Returns a `Map` after an operation has been applied to each key and
   * value in the current `Map`.
   *
   * Every key and value in the current `Map` is affected by a call to
   * `mapWithKey()`, unlike `filterWithKey()` where only values that meet a
   * certain criteria are affected.
   *
   * The keys will remain unchanged from the current `Map` to the returned
   * `Map`. The keys are only used to help in the mapping operation.
   *
   * @param $callback - The callback containing the operation to apply to the
   *                    current `Map` keys and values.
   *
   * @return - a `Map` containing the values after a user-specified operation
   *           on the current `Map`'s keys and values is applied.
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function mapWithKey<Tu>(<<__AtMostRxAsFunc>>(function(Tk, Tv): Tu) $callback): Map<Tk, Tu>;

  /**
   * Returns a `Map` containing the values of the current `Map` that meet
   * a supplied condition.
   *
   * Only values that meet a certain criteria are affected by a call to
   * `filter()`, while all values are affected by a call to `map()`.
   *
   * The keys associated with the current `Map` remain unchanged in the returned
   * `Map`.
   *
   * @param $callback - The callback containing the condition to apply to the
   *                    current `Map` values.
   *
   * @return - a `Map` containing the values after a user-specified condition
   *           is applied.
   *
   * @guide /hack/collections/examples
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filter(<<__AtMostRxAsFunc>>(function(Tv): bool) $callback): Map<Tk, Tv>;

  /**
   * Returns a `Map` containing the values of the current `Map` that meet
   * a supplied condition applied to its keys and values.
   *
   * Only keys and values that meet a certain criteria are affected by a call to
   * `filterWithKey()`, while all values are affected by a call to
   * `mapWithKey()`.
   *
   * The keys associated with the current `Map` remain unchanged in the
   * returned `Map`; the keys will be used in the filtering process only.
   *
   * @param $callback - The callback containing the condition to apply to the
   *                    current `Map` keys and values.
   *
   * @return - a `Map` containing the values after a user-specified condition
   *           is applied to the keys and values of the current `Map`.
   *
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filterWithKey(<<__AtMostRxAsFunc>>(function(Tk, Tv): bool) $callback):
    Map<Tk, Tv>;

  /**
   * Returns a `Map` where each value is a `Pair` that combines the value
   * of the current `Map` and the provided `Traversable`.
   *
   * If the number of values of the current `Map` are not equal to the number
   * of elements in the `Traversable`, then only the combined elements up to
   * and including the final element of the one with the least number of
   * elements is included.
   *
   * The keys associated with the current `Map` remain unchanged in the
   * returned `Map`.
   *
   * @param $traversable - The `Traversable` to use to combine with the
   *                       elements of the current `Map`.
   *
   * @return - The `Map` that combines the values of the current `Map` with
   *           the provided `Traversable`.
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function zip<Tu>(<<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\Traversable::class)>> Traversable<Tu> $traversable): Map<Tk, Pair<Tv, Tu>>;

  /**
   * Returns a `Map` containing the first `n` key/values of the current `Map`.
   *
   * The returned `Map` will always be a proper subset of the current `Map`.
   *
   * `n` is 1-based. So the first element is 1, the second 2, etc.
   *
   * @param $n - The last element that will be included in the `Map`.
   *
   * @return - A `Map` that is a proper subset of this `Map` up to `n` elements.
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function take(int $n): Map<Tk, Tv>;

  /**
   * Returns a `Map` containing the keys and values of the current `Map` up to
   * but not including the first value that produces `false` when passed to the
   * specified callback.
   *
   * The returned `Map` will always be a proper subset of the current `Map`.
   *
   * @param $fn - The callback that is used to determine the stopping condition.
   *
   * @return - A `Map` that is a proper subset of the current `Map` up until
   *           the callback returns `false`.
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function takeWhile(<<__AtMostRxAsFunc>>(function(Tv): bool) $fn): Map<Tk, Tv>;

  /**
   * Returns a `Map` containing the values after the `n`-th element of the
   * current `Map`.
   *
   * The returned `Map` will always be a proper subset of the current `Map`.
   *
   * `n` is 1-based. So the first element is 1, the second 2, etc.
   *
   * @param $n - The last element to be skipped; the `$n+1` element will be the
   *             first one in the returned `Map`.
   *
   * @return - A `Map` that is a proper subset of the current `Map` containing
   *           values after the specified `n`-th element.
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function skip(int $n): Map<Tk, Tv>;

  /**
   * Returns a `Map` containing the values of the current `Map` starting after
   * and including the first value that produces `true` when passed to the
   * specified callback.
   *
   * The returned `Map` will always be a proper subset of this `Map`.
   *
   * @param $fn - The callback used to determine the starting element for the
   *              current `Map`.
   *
   * @return - A `Map` that is a proper subset of the current `Map` starting
   *           after the callback returns `true`.
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function skipWhile(<<__AtMostRxAsFunc>>(function(Tv): bool) $fn): Map<Tk, Tv>;

  /**
   * Returns a subset of the current `Map` starting from a given key location
   * up to, but not including, the element at the provided length from the
   * starting key location.
   *
   * `$start` is 0-based. `$len` is 1-based. So `slice(0, 2)` would return the
   * keys and values at key location 0 and 1.
   *
   * The returned `Map` will always be a proper subset of the current `Map`.
   *
   * @param $start - The starting key location of the current `Map` for the
   *                 returned `Map`.
   * @param $len - The length of the returned `Map`.
   *
   * @return - A `Map` that is a proper subset of the current `Map` starting at
   *           `$start` up to but not including the element `$start + $len`.
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function slice(int $start, int $len): Map<Tk, Tv>;

  /**
   * Returns a `Vector` that is the concatenation of the values of the current
   * `Map` and the values of the provided `Traversable`.
   *
   * The provided `Traversable` is concatenated to the end of the current `Map`
   * to produce the returned `Vector`.
   *
   * @param $traversable - The `Traversable` to concatenate to the current
   *                       `Map`.
   *
   * @return - The integer-indexed concatenated `Vector`.
   *
   * @guide /hack/generics/constraints
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function concat<Tu super Tv>(<<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\Traversable::class)>> Traversable<Tu> $traversable): Vector<Tu>;

  /**
   * Returns the first value in the current `Map`.
   *
   * @return - The first value in the current `Map`,  or `null` if the `Map` is
   *           empty.
   */
  <<__Pure, __MaybeMutable>>
  public function firstValue(): ?Tv;

  /**
   * Returns the first key in the current `Map`.
   *
   * @return - The first key in the current `Map`, or `null` if the `Map` is
   *           empty.
   */
  <<__Pure, __MaybeMutable>>
  public function firstKey(): ?Tk;

  /**
   * Returns the last value in the current `Map`.
   *
   * @return - The last value in the current `Map`, or `null` if the `Map` is
   *           empty.
   */
  <<__Pure, __MaybeMutable>>
  public function lastValue(): ?Tv;

  /**
   * Returns the last key in the current `Map`.
   *
   * @return - The last key in the current `Map`, or `null` if the `Map` is
   *           empty.
   */
  <<__Pure, __MaybeMutable>>
  public function lastKey(): ?Tk;

  /**
   * Checks if the current `Map` is empty.
   *
   * @return - `true` if the current `Map` is empty; `false` otherwise.
   */
  <<__Pure, __MaybeMutable>>
  public function isEmpty(): bool;

  /**
   * Provides the number of elements in the current `Map`.
   *
   * @return - The number of elements in the current `Map`.
   */
  <<__Pure, __MaybeMutable>>
  public function count(): int;

  /**
   * Returns the value at the specified key in the current `Map`.
   *
   * If the key is not present, an exception is thrown. If you don't want an
   * exception to be thrown, use `get()` instead.
   *
   * `$v = $map->at($k)` is equivalent to `$v = $map[$k]`.
   *
   * @param $k - the key from which to retrieve the value.
   *
   * @return - The value at the specified key; or an exception if the key does
   *           not exist.
   */
  <<__Pure, __MaybeMutable>>
  public function at(Tk $k): Tv;

  /**
   * Returns the value at the specified key in the current `Map`.
   *
   * If the key is not present, `null` is returned. If you would rather have an
   * exception thrown when a key is not present, then use `at()`.
   *
   * @param $k - the key from which to retrieve the value.
   *
   * @return - The value at the specified key; or `null` if the key does not
   *           exist.
   */
  <<__Pure, __MaybeMutable>>
  public function get(Tk $k): ?Tv;

  /**
   * Stores a value into the current `Map` with the specified key, overwriting
   * the previous value associated with the key.
   *
   * This method is equivalent to `Map::add()`. If the key to set does not exist,
   * it is created. This is inconsistent with, for example, `Vector::set()`
   * where if the key is not found, an exception is thrown.
   *
   * `$map->set($k,$v)` is equivalent to `$map[$k] = $v` (except that `set()`
   * returns the current `Map`).
   *
   * Future changes made to the current `Map` ARE reflected in the returned
   * `Map`, and vice-versa.
   *
   * @param $k - The key to which we will set the value.
   * @param $v - The value to set.
   *
   * @return - Returns itself.
   */
  <<__Pure, __Mutable, __ReturnsVoidToRx>>
  public function set(Tk $k, Tv $v): Map<Tk, Tv>;

  /**
   * For every element in the provided `Traversable`, stores a value into the
   * current `Map` associated with each key, overwriting the previous value
   * associated with the key.
   *
   * This method is equivalent to `Map::addAll()`. If a key to set does not
   * exist in the Map that does exist in the `Traversable`, it is created. This
   * is inconsistent with, for example, the method `Vector::setAll()` where if
   * a key is not found, an exception is thrown.
   *
   * Future changes made to the current `Map` ARE reflected in the returned
   * `Map`, and vice-versa.
   *
   * @param $k - The `Traversable` with the new values to set. If `null` is
   *             provided, no changes are made.
   *
   * @return - Returns itself.
   */
  <<__Pure, __Mutable, __AtMostRxAsArgs, __ReturnsVoidToRx>>
  public function setAll(<<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\KeyedTraversable::class)>> ?KeyedTraversable<Tk, Tv> $it): Map<Tk, Tv>;

  /**
   * Remove all the elements from the current `Map`.
   *
   * Future changes made to the current `Map` ARE reflected in the returned
   * `Map`, and vice-versa.
   *
   * @return - Returns itself.
   */
  <<__Pure, __Mutable, __ReturnsVoidToRx>>
  public function clear(): Map<Tk, Tv>;

  /**
   * Determines if the specified key is in the current `Map`.
   *
   * This function is interchangeable with `containsKey()`.
   *
   * @param $k - The key to check.
   *
   * @return - `true` if the specified key is present in the current `Map`;
   *           returns `false` otherwise.
   *
   * @guide /hack/generics/constraints
   */
  <<__Pure, __MaybeMutable>>
  public function contains(mixed $k): bool;

  /**
   * Determines if the specified key is in the current `Map`.
   *
   * This function is interchangeable with `contains()`.
   *
   * @param $k - The key to check.
   *
   * @return - `true` if the specified key is present in the current `Map`;
   *           returns `false` otherwise.
   *
   * @guide /hack/generics/constraints
   */
  <<__Pure, __MaybeMutable>>
  public function containsKey(mixed $k): bool;

  /**
   * Add a key/value pair to the end of the current `Map`.
   *
   * This method is equivalent to `Map::set()`. If the key in the  `Pair`
   * exists in the `Map`,  the value associated with it is overwritten.
   *
   * `$map->add($p)` is equivalent to both `$map[$k] = $v` and
   * `$map[] = Pair {$k, $v}` (except that `add()` returns the `Map`).
   *
   * Future changes made to the current `Map` ARE reflected in the returned
   * `Map`, and vice-versa.
   *
   * @param $p - The key/value Pair to add to the current `Map`.
   *
   * @return - Returns itself.
   */
  <<__Pure, __Mutable, __ReturnsVoidToRx>>
  public function add(Pair<Tk, Tv> $p): Map<Tk, Tv>;

  /**
   * For every element in the provided `Traversable`, add a key/value pair into
   * the current `Map`.
   *
   * This method is equivalent to `Map::setAll()`. If a key in the `Traversable`
   * exists in the `Map`, then the value associated with that key in the `Map`
   * is overwritten.
   *
   * Future changes made to the current `Map` ARE reflected in the returned
   * `Map`, and vice-versa.
   *
   * @param $k - The `Traversable` with the new key/value `Pair` to set. If
   *             `null` is provided, no changes are made.
   *
   * @return - Returns itself.
   */
  <<__Pure, __Mutable, __AtMostRxAsArgs, __ReturnsVoidToRx>>
  public function addAll(<<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\Traversable::class)>> ?Traversable<Pair<Tk, Tv>> $it): Map<Tk, Tv>;

  /**
   * Reserves enough memory to accommodate a given number of elements.
   *
   * Reserves enough memory for `sz` elements. If `sz` is less than or
   * equal to the current capacity of this `Map`, this method does nothing.
   *
   * @param $sz - The pre-determined size you want for the current `Map`.
   */
  <<__Pure, __Mutable>>
  public function reserve(int $sz): void;

  /**
   * Removes the specified key (and associated value) from the current `Map`.
   *
   * This method is interchangeable with `removeKey()`.
   *
   * Future changes made to the current `Map` ARE reflected in the returned
   * `Map`, and vice-versa.
   *
   * @param $k - The key to remove.
   *
   * @return - Returns itself.
   */
  <<__Pure, __Mutable, __ReturnsVoidToRx>>
  public function remove(Tk $k): Map<Tk, Tv>;

  /**
   * Removes the specified key (and associated value) from the current `Map`.
   *
   * This method is interchangeable with `remove()`.
   *
   * Future changes made to the current `Map` ARE reflected in the returned
   * `Map`, and vice-versa.
   *
   * @param $k - The key to remove.
   *
   * @return - Returns itself.
   */
  <<__Pure, __Mutable, __ReturnsVoidToRx>>
  public function removeKey(Tk $k): Map<Tk, Tv>;

  /**
   * Returns a new `Map` with the keys that are in the current `Map`, but not
   * in the provided `KeyedTraversable`.
   *
   * @param $traversable - The `KeyedTraversable` on which to compare the keys.
   *
   * @return - A `Map` containing the keys (and associated values) of the
   *           current `Map` that are not in the `KeyedTraversable`.
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function differenceByKey(<<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\KeyedTraversable::class)>> KeyedTraversable<Tk, Tv> $traversable):
    Map<Tk, Tv>;

  /**
   * Returns an iterator that points to beginning of the current `Map`.
   *
   * @return - A `KeyedIterator` that allows you to traverse the current `Map`.
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function getIterator(): \HH\Rx\KeyedIterator<Tk, Tv>;

  /**
   * Returns a `Map` containing the key/value pairs from the specified `array`.
   *
   * This function is deprecated. Use `new `Map`($arr)` instead.
   *
   * @param $arr - The `array` to convert to a `Map`.
   *
   * @return - A `Map` with the key/value pairs from the provided `array`.
   */
  <<__Deprecated('Use `new Map($arr)` instead.')>>
  public static function fromArray(darray<Tk, Tv> $arr): Map<Tk, Tv>;

  /**
   * Creates a `Map` from the given `Traversable`, or an empty `Map` if
   * `null` is passed.
   *
   * This is the static method version of the `Map::__construct()` constructor.
   *
   * @param $items - any `Traversable` object from which to create a `Map`
   *                 (e.g., `array`). If `null`, then an empty `Map` is created.
   *
   * @return - A `Map` with the key/value pairs from the `Traversable`; or an
   *           empty `Map` if the `Traversable` is `null`.
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn>>
  public static function fromItems(<<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\Traversable::class)>> ?Traversable<Pair<Tk, Tv>> $items)
    : Map<Tk, Tv>;

  /**
   * Returns the `string` version of the current `Map`, which is `"Map"`.
   *
   * @return - The `string` `"Map"`.
   */
  <<__Pure, __MaybeMutable>>
  public function __toString(): string;

  /**
   * Returns an `Iterable` view of the current `Map`.
   *
   * The `Iterable` returned is one that produces the key/values from the
   * current `Map`.
   *
   * @return - The `Iterable` view of the current `Map`.
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function items(): \HH\Rx\Iterable<Pair<Tk, Tv>>;
  <<__Pure, __MaybeMutable>> /* HH_FIXME[0002] */
  public function toVArray(): varray<Tv>;
  <<__Pure, __MaybeMutable>> /* HH_FIXME[0001] */
  public function toDArray(): darray<Tk, Tv>;
}

} // namespace HH

namespace {

/**
 * @internal
 *
 * Methods and functions should take and return the KeyedIterator interface.
 */
class MapIterator<Tk, +Tv> implements HH\Rx\KeyedIterator<Tk, Tv> {
  <<__Pure>>
  public function __construct();
  <<__Pure, __MaybeMutable>>
  public function current(): Tv;
  <<__Pure, __MaybeMutable>>
  public function key(): Tk;
  <<__Pure, __MaybeMutable>>
  public function valid(): bool;
  <<__Pure, __Mutable>>
  public function next(): void;
  <<__Pure, __Mutable>>
  public function rewind(): void;
}

} // namespace
