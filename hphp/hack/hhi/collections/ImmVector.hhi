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
 * `ImmVector` is an immutable `Vector`. HHVM provides a native implementation
 * for this class. The PHP class definition below is not actually used at run
 * time; it is simply provided for the typechecker and for developer reference.
 *
 * A `ImmVector` cannot be mutated. No elements can be added or removed from it,
 * nor can elements be overwritten using assignment (i.e. `$c[$k] = $v` is not
 * allowed).
 *
 * ```
 * $v = Vector {1, 2, 3};
 * $fv = $v->toImmVector();
 * ```
 *
 * construct it with a `Traversable`:
 *
 * ```
 * $a = array(1, 2, 3);
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

final class ImmVector<+Tv> implements ConstVector<Tv> {
  /**
   * Creates an `ImmVector` from the given `Traversable`, or an empty
   * `ImmVector` if `null` is passed.
   *
   * @param $it - any `Traversable` object from which to create the `ImmVector`
   *              (e.g., `array`). If `null`, then an empty `ImmVector` is
   *              created.
   */
  public function __construct(?Traversable<Tv> $it);

  /**
   * Checks if the current `ImmVector` is empty.
   *
   * @return - `true` if the current `ImmVector` is empty; `false` otherwise.
   */
  public function isEmpty(): bool;

  /**
   * Provides the number of elements in the current `ImmVector`.
   *
   * @return - The number of elements in the current `ImmVector`.
   */
  public function count(): int;

  /**
   * Returns the value at the specified key in the current `ImmVector`.
   *
   * If the key is not present, an exception is thrown. If you don't want an
   * exception to be thrown, use `get()` instead.
   *
   * `$v = $vec->at($k)` is semantically equivalent to `$v = $vec[$k]`.
   *
   * @param $k - the key from which to retrieve the value.
   *
   * @return - The value at the specified key; or an exception if the key does
   *           not exist.
   */
  public function at(int $k): Tv;

  /**
   * Returns the value at the specified key in the current `ImmVector`.
   *
   * If the key is not present, `null` is returned. If you would rather have an
   * exception thrown when a key is not present, then use `at()`.
   *
   * @param $k - the key from which to retrieve the value.
   *
   * @return - The value at the specified key; or `null` if the key does not
   *           exist.
   */
  public function get(int $k): ?Tv;

  /**
   * Determines if the specified key is in the current `ImmVector`.
   *
   * @return - `true` if the specified key is present in the current
   *           `ImmVector`; `false` otherwise.
   *
   * @guide /hack/generics/constraints
   */
  public function containsKey<Tu super int>(Tu $k): bool;

  /**
   * Returns an `array` containing the values from the current `ImmVector`.
   *
   * This method is interchangeable with `toValuesArray()`.
   *
   * @return - an `array` containing the values from the current `ImmVector`.
   */
  public function toArray(): array<Tv>;

  /**
   * Returns an `array` containing the values from the current `ImmVector`.
   *
   * This method is interchangeable with `toArray()`.
   *
   * @return - an `array` containing the values from the current `ImmVector`.
   */
  public function toValuesArray(): array<Tv>;

  /**
   * Returns an `array` whose values are the keys from the current `ImmVector`.
   *
   * @return - an `array` with the integer keys from the current `ImmVector`.
   */
  public function toKeysArray(): array<Tv>;

  /**
   * Returns an iterator that points to beginning of the current `ImmVector`.
   *
   * @return - A `KeyedIterator` that allows you to traverse the current
   *           `ImmVector`.
   */
  public function getIterator(): KeyedIterator<int, Tv>;

  /**
   * Returns the index of the first element that matches the search value.
   *
   * If no element matches the search value, this function returns -1.
   *
   * @param $search_value - The value that will be search for in the current
   *                        `ImmVector`.
   *
   * @return - The key (index) where that value is found; -1 if it is not found.
   *
   * @guide /hack/generics/constraints
   */
  public function linearSearch<Tu super Tv>(Tu $search_value): int;

  /**
   * Creates an `ImmVector` from the given `Traversable`, or an empty `ImmVector`
   * if `null` is passed.
   *
   * This is the static method version of the `ImmVector::__construct()`
   * constructor.
   *
   * @param $items - any `Traversable` object from which to create an
   *                 `ImmVector` (e.g., `array`). If `null`, then an empty
   *                 `ImmVector` is created.
   *
   * @return - An `ImmVector` with the values from the `Traversable`; or an
   *           empty `ImmVector` if the `Traversable` is `null`.
   */
  public static function fromItems(?Traversable<Tv> $items): ImmVector<Tv>;

  /**
   * Creates an `ImmVector` from the keys of the specified container.
   *
   * If a key of the specified container are not integers, then it adds
   * the next available integer key in the current `ImmVector` and makes that
   * key the value.
   *
   * @param $container - The container with the keys used to create the
   *                     current `ImmVector`.
   *
   * @return - An `ImmVector` built from the keys of the specified container.
   */
  public static function fromKeysOf<Tk>(
    ?KeyedContainer<Tk, mixed> $container,
  ): ImmVector<Tk>;

  /**
   * Returns the `string` version of the current `ImmVector`, which is
   * `"ImmVector"`.
   *
   * @return - The `string` `"ImmVector"`.
   */
  public function __toString(): string;

  /**
   * Returns an `Iterable` view of the current `ImmVector`.
   *
   * The `Iterable` returned is one that produces the values from the current
   * `ImmVector`.
   *
   * @return - The `Iterable` view of the current `ImmVector`.
   */
  public function items(): Iterable<Tv>;

  /**
   * Returns a copy of the current `ImmVector`.
   *
   * @return - an `ImmVector` that is a copy of the current `ImmVector`.
   */
  public function toImmVector(): ImmVector<Tv>;

  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
 /**
   * Returns a `Vector` containing the elemnts of the current `ImmVector`.
   *
   * The returned `Vector` will, of course, be mutable.
   *
   * @return - a `Vector` with the elements of the current `ImmVector`.
   */
  public function toVector(): Vector<Tv>;

  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  /**
   * Returns an integer-keyed `Map` based on the elements of the current
   * `ImmVector`.
   *
   * The keys are `0... count() - 1`.
   *
   * @return - an integer-keyed `Map` with the values of the current
   *           `ImmVector`.
   */
  public function toMap(): Map<int, Tv>;

  /**
   * Returns an immutable integer-keyed Map (`ImmMap`) based on the elements of
   * the current `ImmVector`.
   *
   * The keys are `0... count() - 1`.
   *
   * @return - an `ImmMap` with the values of the current `ImmVector`.
   */
  public function toImmMap(): ImmMap<int, Tv>;

  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  /**
   * Returns a `Set` with the values of the current `ImmVector`.
   *
   * @return - a `Set` with the values of the current `ImmVector`.
   */
  public function toSet(): Set<Tv>;

  /**
   * Returns an immutable Set (`ImmSet`) with the values of the current
   * `ImmVector`.
   *
   * @return - an `ImmSet` with the current values of the current `ImmVector`.
   */
  public function toImmSet(): ImmSet<Tv>;

  /**
   * Returns an immutable copy (`ImmVector`) of the current `ImmVector`.
   *
   * This method is interchangeable with `toImmVector()` and `values()`.
   *
   * @return - an `ImmVector` copy of the current `ImmVector`.
   */
  public function immutable(): ImmVector<Tv>;

  /**
   * Returns a lazy, access elements only when needed view of the current
   * `ImmVector`.
   *
   * Normally, memory is allocated for all of the elements of an `ImmVector`.
   * With a lazy view, memory is allocated for an element only when needed or
   * used in a calculation like in `map()` or `filter()`.
   *
   * @return - an integer-keyed `KeyedIterable` representing the lazy view into
   *           the current `ImmVector`.
   *
   * @guide /hack/collections/examples
   */
  public function lazy(): KeyedIterable<int, Tv>;

  /**
   * Returns an `ImmVector` containing the values of the current `ImmVector`.
   * Essentially a copy of the current `ImmVector`.
   *
   * This method is interchangeable with `toImmVector()` and `immutable()`.
   *
   * @return - an `ImmVector` containing the values of the current `ImmVector`.
   */
  public function values(): ImmVector<Tv>;

  /**
   * Returns an `ImmVector` containing the keys, as values, of the current
   * `ImmVector`.
   *
   * @return - an `ImmVector` containing, as values, the integer keys of the
   *           current `ImmVector`.
   */
  public function keys(): ImmVector<int>;

  /**
   * Returns an `ImmVector` containing the values after an operation has been
   * applied to each value in the current `ImmVector`.
   *
   * Every value in the current `ImmVector` is affected by a call to `map()`,
   * unlike `filter()` where only values that meet a certain criteria are
   * affected.
   *
   * @param $callback - The callback containing the operation to apply to the
   *                    current `ImmVector` values.
   *
   * @return - a `ImmVector` containing the values after a user-specified
   *           operation is applied.
   *
   * @guide /hack/collections/examples
   */
  public function map<Tu>((function(Tv): Tu) $callback): ImmVector<Tu>;

  /**
   * Returns an `ImmVector` containing the values after an operation has been
   * applied to each key and value in the current `ImmVector`.
   *
   * Every key and value in the current `ImmVector` is affected by a call to
   * `mapWithKey()`, unlike `filterWithKey()` where only values that meet a
   * certain criteria are affected.
   *
   * @param $callback - The callback containing the operation to apply to the
   *                    current `ImmVector` keys and values.
   *
   * @return - an `ImmVector` containing the values after a user-specified
   *           operation on the current `ImmVector`'s keys and values is applied.
   */
  public function mapWithKey<Tu>((function(int, Tv): Tu) $callback):
    ImmVector<Tu>;

  /**
   * Returns a `ImmVector` containing the values of the current `ImmVector` that
   * meet a supplied condition.
   *
   * Only values that meet a certain criteria are affected by a call to
   * `filter()`, while all values are affected by a call to `map()`.
   *
   * @param $callback - The callback containing the condition to apply to the
   *                    current `ImmVector` values.
   *
   * @return - a `ImmVector` containing the values after a user-specified
   *           condition is applied.
   *
   * @guide /hack/collections/examples
   */
  public function filter((function(Tv): bool) $callback): ImmVector<Tv>;

  /**
   * Returns an `ImmVector` containing the values of the current `ImmVector`
   * that meet a supplied condition applied to its keys and values.
   *
   * Only keys and values that meet a certain criteria are affected by a call to
   * `filterWithKey()`, while all values are affected by a call to
   * `mapWithKey()`.
   *
   * @param $callback - The callback containing the condition to apply to the
   *                    current `ImmVector` keys and values.
   *
   * @return - an `ImmVector` containing the values after a user-specified
   *           condition is applied to the keys and values of the current
   *           `ImmVector`.
   *
   */
  public function filterWithKey((function(int, Tv): bool) $callback):
    ImmVector<Tv>;

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
   * @return - The `ImmVector` that combines the values of this `ImmVector`
   *           with the provided `Traversable`.
   */
  public function zip<Tu>(Traversable<Tu> $traversable):
    ImmVector<Pair<Tv, Tu>>;

  /**
   * Returns an `ImmVector` containing the first `n` values of the current
   * `ImmVector`.
   *
   * The returned `ImmVector` will always be a proper subset of this
   * `ImmVector`.
   *
   * `n` is 1-based. So the first element is 1, the second 2, etc.
   *
   * @param $n - The last element that will be included in the `ImmVector`.
   *
   * @return - An `ImmVector` that is a proper subset of the curretn
   *           `ImmVector` up to `n` elements.
   */
  public function take(int $n): ImmVector<Tv>;

  /**
   * Returns an `ImmVector` containing the values of the current `ImmVector` up
   * to but not including the first value that produces `false` when passed to
   * the specified callback.
   *
   * The returned `ImmVector` will always be a proper subset of the current
   * `ImmVector`.
   *
   * @param $fn - The callback that is used to determine the stopping condition.
   *
   * @return - An `ImmVector` that is a proper subset of the current
   *           `ImmVector` up until when the callback returns `false`.
   */
  public function takeWhile((function(Tv): bool) $fn): ImmVector<Tv>;

  /**
   * Returns an `ImmVector` containing the values after the `n`-th element of
   * the current `ImmVector`.
   *
   * The returned `ImmVector` will always be a proper subset of the current
   * `ImmVector`.
   *
   * `n` is 1-based. So the first element is 1, the second 2, etc.
   *
   * @param $n - The last element to be skipped; the `$n+1` element will be the
   *             first one in the returned `ImmVector`.
   *
   * @return - An `ImmVector` that is a proper subset of the current `ImmVector`
   *           containing values after the specified `n`-th element.
   */
  public function skip(int $n): ImmVector<Tv>;

  /**
   * Returns an `ImmVector` containing the values of the current `ImmVector`
   * starting after and including the first value that produces `true` when
   * passed to the specified callback.
   *
   * The returned `ImmVector` will always be a proper subset of the current
   * `ImmVector`.
   *
   * @param $fn - The callback used to determine the starting element for the
   *              `ImmVector`.
   *
   * @return - An `ImmVector` that is a proper subset of the current
   *           `ImmVector` starting after the callback returns `true`.
   */
  public function skipWhile((function(Tv): bool) $fn): ImmVector<Tv>;

  /**
   * Returns a subset of the current `ImmVector` starting from a given key up
   * to, but not including, the element at the provided length from the
   * starting key.
   *
   * `$start` is 0-based. `$len` is 1-based. So `slice(0, 2)` would return the
   * elements at key 0 and 1.
   *
   * The returned `ImmVector` will always be a proper subset of the current
   * `ImmVector`.
   *
   * @param $start - The starting key of the current `ImmVector` to begin the
   *                 returned `ImmVector`.
   * @param $len - The length of the returned `ImmVector`.
   *
   * @return - An `ImmVector` that is a proper subset of the current
   *           `ImmVector` starting at `$start` up to but not including the
   *           element `$start + $len`.
   */
  public function slice(int $start, int $len): ImmVector<Tv>;

  /**
   * Returns an `ImmVector` that is the concatenation of the values of the
   * current `ImmVector` and the values of the provided `Traversable`.
   *
   * The values of the provided `Traversable` is concatenated to the end of the
   * current `ImmVector` to produce the returned `ImmVector`.
   *
   * @param $traversable - The `Traversable` to concatenate to the current
   *                       `ImmVector`.
   *
   * @return - The concatenated `ImmVector`.
   *
   * @guide /hack/generics/constraints
   */
  public function concat<Tu super Tv>(
    Traversable<Tu> $traversable
  ): ImmVector<Tv>;

  /**
   * Returns the first value in the current `ImmVector`.
   *
   * @return - The first value in the current `ImmVector`, or `null` if the
   *           current `ImmVector` is empty.
   */
  public function firstValue(): ?Tv;

  /**
   * Returns the first key in the current `ImmVector`.
   *
   * @return - The first key (an integer) in the current `ImmVector`, or `null`
   *           if the current `ImmVector` is empty.
   */
  public function firstKey(): ?int;

  /**
   * Returns the last value in the current `ImmVector`.
   *
   * @return - The last value in the current `ImmVector`, or `null` if the
   *           current `ImmVector` is empty.
   */
  public function lastValue(): ?Tv;

  /**
   * Returns the last key in the current `ImmVector`.
   *
   * @return - The last key (an integer) in the current `ImmVector`, or `null`
   *           if the current `ImmVector` is empty.
   */
  public function lastKey(): ?int;
}
