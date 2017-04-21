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

/**
 * This file provides type information for some of HHVM's builtin classes.
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

/**
 * `ImmMap` is an immutable `Map`. HHVM provides a native implementation for
 * this class. The PHP class definition below is not actually used at run time;
 * it is simply provided for the typechecker and for developer reference.
 *
 * A `ImmMap` cannot be mutated. No elements can be added or removed from it,
 * nor can elements be overwritten using assignment (i.e. `$c[$k] = $v` is
 * not allowed).
 *
 * Construct it with a `Traversable`:
 *
 * ```
 * $a = array('a' => 1, 'b' => 2);
 * $fm = new ImmMap($a);
 * ```
 *
 * or use the literal syntax
 *
 * ```
 * $fm = ImmMap {'a' => 1, 'b' => 2};
 * ```
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/classes
 */

final class ImmMap<Tk, +Tv> implements ConstMap<Tk, Tv>, Indexish<Tk, Tv> {
  /**
   * Creates an `ImmMap` from the given `KeyedTraversable`, or an empty
   * `ImmMap` if `null` is passed.
   *
   * @param $it - any `Traversable` object from which to create an `ImmMap`
   *             (e.g., `array`). If `null`, then an empty `ImmMap` is created.
   */
  public function __construct(?KeyedTraversable<Tk, Tv> $it);

  /**
   * Returns an `array` containing the key/value pairs from the current
   * `ImmMap`.
   *
   * @return - an `array` containing the key and value pairs from the current
   *           `ImmMap`.
   */
  public function toArray(): array<Tk, Tv>;

  /**
   * Returns an `array` containing the values from the current `ImmMap`.
   *
   * @return - an integer-indexed `array` containing the values from the
   *           current `ImmMap`.
   */
  public function toValuesArray(): array<Tv>;

  /**
   * Returns an `array` whose values are the keys of the current `ImmMap`.
   *
   * @return - an integer-indexed `array` where the values are the keys from
   *           the current `ImmMap`.
   */
  public function toKeysArray(): array<Tk>;

  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  /**
   * Returns a `Vector` with the values of the current `ImmMap`.
   *
   * @return - a `Vector` that contains the values of the current `ImmMap`.
   */
  public function toVector(): Vector<Tv>;

  /**
   * Returns an immutable vector (`ImmVector`) with the values of the current
   * `ImmMap`.
   *
   * @return - an `ImmVector` that contains the values of the current `ImmMap`.
   */
  public function toImmVector(): ImmVector<Tv>;

  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  /**
   * Returns a mutable copy (`Map`) of this `ImmMap`.
   *
   * @return - a mutable `Map` that is a copy of the current `ImmMap`.
   */
  public function toMap(): Map<Tk, Tv>;

  /**
   * Returns an immutable copy (`ImmMap`) of the current `ImmMap`.
   *
   * @return - an `ImmMap` that is a copy of the current `ImmMap`.
   */
  public function toImmMap(): ImmMap<Tk, Tv>;

  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  /**
   * Returns a `Set` based on the values of the current `ImmMap`.
   *
   * @return - a `Set` with the current values of the current `ImmMap`.
   */
  public function toSet(): Set<Tv>;

  /**
   * Returns an immutable set (`ImmSet`) based on the values of the current
   * `ImmMap`.
   *
   * @return - an `ImmSet` with the current values of the current `ImmMap`.
   */
  public function toImmSet(): ImmSet<Tv>;

  /**
   * Returns an immutable copy (`ImmMap`) of the current `ImmMap`.
   *
   * This method is interchangeable with `toImmMap()`.
   *
   * @return - an `ImmMap` representing a copy of the current `ImmMap`.
   */
  public function immutable(): ImmMap<Tk, Tv>;

  /**
   * Returns a lazy, access elements only when needed view of the current
   * `ImmMap`.
   *
   * Normally, memory is allocated for all of the elements of an `ImmMap`. With
   * a lazy view, memory is allocated for an element only when needed or used
   * in a calculation like in `map()` or `filter()`.
   *
   * @return - a `KeyedIterable` representing the lazy view into the current
   *           `ImmMap`.
   *
   * @guide /hack/collections/examples
   */
  public function lazy(): KeyedIterable<Tk, Tv>;

  /**
   * Returns an ImmVector containing the values of the current `ImmMap`.
   *
   * This method is interchangeable with toImmVector().
   *
   * @return - an ImmVector containing the values of the current `ImmMap`.
   */
  public function values(): ImmVector<Tv>;

  /**
   * Returns an ImmVector containing, as values, the keys of the current `ImmMap`.
   *
   * @return - an ImmVector containing, as values, the keys of the current
   *           `ImmMap`.
   */
  public function keys(): ImmVector<Tk>;

  /**
   * Returns an `ImmMap` after an operation has been applied to each value in
   * the current `ImmMap`.
   *
   * Every value in the current `ImmMap` is affected by a call to `map()`,
   * unlike `filter()` where only values that meet a certain criteria are
   * affected.
   *
   * The keys will remain unchanged from this `ImmMap` to the returned `ImmMap`.
   *
   * @param $callback - The callback containing the operation to apply to the
   *                    current `ImmMap` values.
   *
   * @return - an `ImmMap` containing key/value pairs after a user-specified
   *           operation is applied.
   *
   * @guide /hack/collections/examples
   */
  public function map<Tu>((function(Tv): Tu) $callback): ImmMap<Tk, Tu>;

  /**
   * Returns an `ImmMap` after an operation has been applied to each key and
   * value in current `ImmMap`.
   *
   * Every key and value in the current `ImmMap` is affected by a call to
   * `mapWithKey()`, unlike `filterWithKey()` where only values that meet a
   * certain criteria are affected.
   *
   * The keys will remain unchanged from the current `ImmMap` to the returned
   * `ImmMap`. The keys are only used to help in the operation.
   *
   * @param $callback - The callback containing the operation to apply to the
   *                    current `ImmMap` keys and values.
   *
   * @return - an `ImmMap` containing the values after a user-specified
   *           operation on the current `ImmMap`'s keys and values is applied.
   */
  public function mapWithKey<Tu>((function(Tk, Tv): Tu) $callback):
    ImmMap<Tk, Tu>;

  /**
   * Returns an `ImmMap` containing the values of the current `ImmMap` that
   * meet a supplied condition.
   *
   * Only values that meet a certain criteria are affected by a call to
   * `filter()`, while all values are affected by a call to `map()`.
   *
   * The keys associated with the current `ImmMap` remain unchanged in the
   * returned `Map`.
   *
   * @param $callback - The callback containing the condition to apply to the
   *                    current `ImmMap` values.
   *
   * @return - an `ImmMap` containing the values after a user-specified
   *           condition is applied.
   *
   * @guide /hack/collections/examples
   */
  public function filter((function(Tv): bool) $callback): ImmMap<Tk, Tv>;

  /**
   * Returns an `ImmMap` containing the values of the current `ImmMap` that
   * meet a supplied condition applied to its keys and values.
   *
   * Only keys and values that meet a certain criteria are affected by a call to
   * `filterWithKey()`, while all values are affected by a call to
   * `mapWithKey()`.
   *
   * The keys associated with the current `ImmMap` remain unchanged in the
   * returned `ImmMap`; the keys will be used in the filtering process only.
   *
   * @param $callback - The callback containing the condition to apply to the
   *                    current `ImmMap` keys and values.
   *
   * @return - an `ImmMap` containing the values after a user-specified
   *           condition is applied to the keys and values of the current
   *           `ImmMap`.
   *
   */
  public function filterWithKey((function(Tk, Tv): bool) $callback):
    ImmMap<Tk, Tv>;

  /**
   * Returns an `ImmMap` where each value is a `Pair` that combines the value
   * of the current `ImmMap` and the provided `Traversable`.
   *
   * If the number of values of the current `ImmMap` are not equal to the
   * number of elements in the `Traversable`, then only the combined elements
   * up to and including the final element of the one with the least number of
   * elements is included.
   *
   * The keys associated with the current `ImmMap` remain unchanged in the
   * returned `ImmMap`.
   *
   * @param $traversable - The `Traversable` to use to combine with the
   *                       elements of the current `ImmMap`.
   *
   * @return - The `ImmMap` that combines the values of the current `ImmMap`
   *           with the provided `Traversable`.
   */
  public function zip<Tu>(Traversable<Tu> $traversable):
    ImmMap<Tk, Pair<Tv, Tu>>;

  /**
   * Returns an `ImmMap` containing the first `n` key/values of the current
   * `ImmMap`.
   *
   * The returned `ImmMap` will always be a proper subset of the current
   * `ImmMap`.
   *
   * `n` is 1-based. So the first element is 1, the second 2, etc.
   *
   * @param $n - The last element that will be included in the returned
   *             `ImmMap`.
   *
   * @return - An `ImmMap` that is a proper subset of the current `ImmMap` up
   *           to `n` elements.
   */
  public function take(int $n): ImmMap<Tk, Tv>;

  /**
   * Returns an `ImmMap` containing the keys and values of the current `ImmMap`
   * up to but not including the first value that produces `false` when passed
   * to the specified callback.
   *
   * The returned `ImmMap` will always be a proper subset of the current
   * `ImmMap`.
   *
   * @param $fn - The callback that is used to determine the stopping condition.
   *
   * @return - An `ImmMap` that is a proper subset of the current `ImmMap` up
   *           until when the callback returns `false`.
   */
  public function takeWhile((function(Tv): bool) $fn): ImmMap<Tk, Tv>;

  /**
   * Returns an `ImmMap` containing the values after the `n`-th element of the
   * current `ImmMap`.
   *
   * The returned `ImmMap` will always be a proper subset of the current
   * `ImmMap`.
   *
   * `n` is 1-based. So the first element is 1, the second 2, etc.
   *
   * @param $n - The last element to be skipped; the `$n+1` element will be the
   *             first one in the returned `ImmMap`.
   *
   * @return - An `ImmMap` that is a proper subset of the current `ImmMap`
   *           containing values after the specified `n`-th element.
   */
  public function skip(int $n): ImmMap<Tk, Tv>;

  /**
   * Returns an `ImmMap` containing the values of the current `ImmMap` starting
   * after and including the first value that produces `true` when passed to
   * the specified callback.
   *
   * The returned `ImmMap` will always be a proper subset of the current
   * `ImmMap`.
   *
   * @param $fn - The callback used to determine the starting element for the
   *              `ImmMap`.
   *
   * @return - An `ImmMap` that is a proper subset of the current `ImmMap`
   *           starting after the callback returns `true`.
   */
  public function skipWhile((function(Tv): bool) $fn): ImmMap<Tk, Tv>;

  /**
   * Returns a subset of the current `ImmMap` starting from a given key
   * location up to, but not including, the element at the provided length from
   * the starting key location.
   *
   * `$start` is 0-based. `$len` is 1-based. So `slice(0, 2)` would return the
   * keys and values at key location 0 and 1.
   *
   * The returned `ImmMap` will always be a proper subset of the current
   * `ImmMap`.
   *
   * @param $start - The starting key location of the current `ImmMap` for the
   *                 returned `ImmMap`.
   * @param $len - The length of the returned `ImmMap`.
   *
   * @return - An `ImmMap` that is a proper subset of the current `ImmMap`
   *           starting at `$start` up to but not including the element
   *           `$start + $len`.
   */
  public function slice(int $start, int $len): ImmMap<Tk, Tv>;

  /**
   * Returns an ImmVector that is the concatenation of the values of the
   * current `ImmMap` and the values of the provided `Traversable`.
   *
   * The provided `Traversable` is concatenated to the end of the current
   * `ImmMap` to produce the returned `ImmVector`.
   *
   * @param $traversable - The `Traversable` to concatenate to this `ImmMap`.
   *
   * @return - The integer-indexed concatenated `ImmVector`.
   *
   * @guide /hack/generics/constraints
   */
  public function concat<Tu super Tv>(Traversable<Tu> $traversable):
    ImmVector<Tu>;

  /**
   * Returns the first value in the current `ImmMap`.
   *
   * @return - The first value in the current `ImmMap`, or `null` if the current
   *           `ImmMap` is empty.
   */
  public function firstValue(): ?Tv;

  /**
   * Returns the first key in the current `ImmMap`.
   *
   * @return - The first key in the current `ImmMap`, or `null` if the current
   *           `ImmMap` is empty.
   */
  public function firstKey(): ?Tk;

  /**
   * Returns the last value in the current `ImmMap`.
   *
   * @return - The last value in the current `ImmMap`, or `null` if the current
   *           `ImmMap` is empty.
   */
  public function lastValue(): ?Tv;

  /**
   * Returns the last key in the current `ImmMap`.
   *
   * @return - The last key in the current `ImmMap`, or `null` if the current
   *           `ImmMap` is empty.
   */
  public function lastKey(): ?Tk;

  /**
   * Checks if the current `ImmMap` is empty.
   *
   * @return - `true` if the current `ImmMap` is empty; `false` otherwise.
   */

  public function isEmpty(): bool;

  /**
   * Provides the number of elements in the current `ImmMap`.
   *
   * @return - The number of elements in current `ImmMap`.
   */
  public function count(): int;

  /**
   * Returns the value at the specified key in the current `ImmMap`.
   *
   * If the key is not present, an exception is thrown. If you don't want an
   * exception to be thrown, use `get()` instead.
   *
   * `$v = $map->at($k)` is semantically equivalent to `$v = $map[$k]`.
   *
   * @param $k - the key from which to retrieve the value.
   *
   * @return - The value at the specified key; or an exception if the key does
   *           not exist.
   */
  public function at(Tk $k): Tv;

  /**
   * Returns the value at the specified key in the current `ImmMap`.
   *
   * If the key is not present, null is returned. If you would rather have an
   * exception thrown when a key is not present, then use `at()`.
   *
   * @param $k - the key from which to retrieve the value.
   *
   * @return - The value at the specified key; or `null` if the key does not
   *           exist.
   */
  public function get(Tk $k): ?Tv;

  /**
   * Determines if the specified key is in the current `ImmMap`.
   *
   * This function is interchangeable with `containsKey()`.
   *
   * @param $k - The key to check.
   *
   * @return - `true` if the specified key is present in the current `ImmMap`;
   *           `false` otherwise.
   *
   * @guide /hack/generics/constraints
   */
  public function contains<Tu super Tk>(Tu $k): bool;

  /**
   * Determines if the specified key is in the current `ImmMap`.
   *
   * This function is interchangeable with `contains()`.
   *
   * @param $k - The key to check.
   *
   * @return - `true` if the specified key is present in the current `ImmMap`;
   *           `false` otherwise.
   *
   * @guide /hack/generics/constraints
   */
  public function containsKey<Tu super Tk>(Tu $k): bool;

  /**
   * Returns a new `ImmMap` with the keys that are in the current `ImmMap`, but
   * not in the provided `KeyedTraversable`.
   *
   * @param $traversable - The `KeyedTraversable` on which to compare the keys.
   *
   * @return - An `ImmMap` containing the keys (and associated values) of the
   *           current `ImmMap` that are not in the `KeyedTraversable`.
   */
  public function differenceByKey<Tu super Tk, Tw>(
    KeyedTraversable<Tu, Tw> $traversable
  ): ImmMap<Tk, Tv>;

  /**
   * Returns an iterator that points to beginning of the current `ImmMap`.
   *
   * @return - A `KeyedIterator` that allows you to traverse the current
   *           `ImmMap`.
   */
  public function getIterator(): KeyedIterator<Tk, Tv>;

  /**
   * Creates an `ImmMap` from the given `Traversable`, or an empty `ImmMap`
   * if `null` is passed.
   *
   * This is the static method version of the `ImmMap::__construct()`
   * constructor.
   *
   * @param $items - any Traversable object from which to create an `ImmMap`
   *                 (e.g., `array`). If `null`, then an empty `ImmMap` is
   *                 created.
   *
   * @return - An `ImmMap` with the key/value pairs from the `Traversable`; or
   *           an empty `ImmMap` if the `Traversable` is `null`.
   */
  public static function fromItems(?Traversable<Pair<Tk, Tv>> $items):
    ImmMap<Tk, Tv>;

  /**
   * Returns the `string` version of the current `ImmMap`, which is `"ImmMap"`.
   *
   * @return - The `string` `"ImmMap"`.
   */
  public function __toString(): string;

  /**
   * Returns an `Iterable` view of the current `ImmMap`.
   *
   * The `Iterable` returned is one that produces the key/values from the
   * current `ImmMap`.
   *
   * @return - The `Iterable` view of the current `ImmMap`.
   */
  public function items(): Iterable<Pair<Tk, Tv>>;
}
