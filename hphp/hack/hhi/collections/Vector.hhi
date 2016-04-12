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
 * `Vector`s do not support taking elements by reference. If binding assignment
 * (`=&`) is used with an element of a `Vector`, or if an element of a `Vector`
 * is passed by reference, of if a `Vector` is used with `foreach` by
 * reference, an exception will be thrown.
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/classes
 */

final class Vector<Tv> implements MutableVector<Tv> {
  /**
   * Creates a `Vector` from the given `Traversable`, or an empty `Vector`
   * if `null` is passed.
   *
   * @param $it - any `Traversable` object from which to create
   *              the `Vector` (e.g., `array`). If `null`, then an empty
   *              `Vector` is created.
   */
  public function __construct(?Traversable<Tv> $it);

  /**
   * Returns an `array` containing the values from this `Vector`.
   *
   * This method is interchangeable with `toValuesArray()`.
   *
   * @return - an `array` containing the values from this `Vector`.
   */
  public function toArray(): array<Tv>;

  /**
   * Returns an `array` containing the values from the current `Vector`.
   *
   * This method is interchangeable with `toArray()`.
   *
   * @return - an `array` containing the values from the current `Vector`.
   */
  public function toValuesArray(): array<Tv>;

  /**
   * Returns an `array` whose values are the keys from the current `Vector`.
   *
   * @return - an `array` with the integer keys from the current `Vector`.
   */
  public function toKeysArray(): array<int>;

  /**
   * Returns a deep copy of the current `Vector`.
   *
   * @return - a `Vector` that is a copy of the current `Vector`.
   */
  public function toVector(): Vector<Tv>;

  /**
   * Returns a deep, immutable copy (`ImmVector`) of the current `Vector`.
   *
   * @return - a `Vector` that is an immutable copy of the current `Vector`.
   */
  public function toImmVector(): ImmVector<Tv>;

  /**
   * Returns an integer-keyed `Map` based on the values of the current `Vector`.
   *
   * @return - a `Map` that has the integer keys and associated values of the
   *           current `Vector`.
   */
  public function toMap(): Map<int, Tv>;

  /**
   * Returns an immutable, integer-keyed map (`ImmMap`) based on the values of
   * the current `Vector`.
   *
   * @return - an `ImmMap` that has the integer keys and associated values
   *           of the current `Vector`.
   */
  public function toImmMap(): ImmMap<int, Tv>;

  /**
   * Returns a `Set` based on the values of the current `Vector`.
   *
   * @return - a `Set` with the values of the current `Vector`.
   */
  public function toSet(): Set<Tv>;

  /**
   * Returns an immutable set (`ImmSet`) based on the values of the current
   * `Vector`.
   *
   * @return - an `ImmSet` with the values of the current `Vector`.
   */
  public function toImmSet(): ImmSet<Tv>;

  /**
   * Returns a deep, immutable copy (`ImmVector`) of the current `Vector`.
   *
   * This method is interchangeable with `toImmVector()`.
   *
   * @return - an `ImmVector` copy the current `Vector`.
   */
  public function immutable(): ImmVector<Tv>;

  /**
   * Returns a lazy, access elements only when needed view of the current
   * `Vector`.
   *
   * Normally, memory is allocated for all of the elements of the `Vector`.
   * With a lazy view, memory is allocated for an element only when needed or
   * used in a calculation like in `map()` or `filter()`.
   *
   * @return - an integer-keyed `KeyedIterable` representing the lazy view into
   *           the current `Vector`.
   *
   * @guide /hack/collections/examples
   */
  public function lazy(): KeyedIterable<int, Tv>;

  /**
   * Returns a `Vector` containing the values of the current `Vector`.
   * Essentially a copy of the current `Vector`.
   *
   * This method is interchangeable with `toVector()`.
   *
   * @return - a `Vector` containing the values of the current `Vector`.
   */
  public function values(): Vector<Tv>;

  /**
   * Returns a `Vector` containing the keys of the current `Vector`.
   *
   * @return - a `Vector` containing the integer keys of the current `Vector`.
   */
  public function keys(): Vector<int>;

  /**
   * Returns a `Vector` containing the values after an operation has been
   * applied to each value in the current `Vector`.
   *
   * Every value in the current `Vector` is affected by a call to `map()`,
   * unlike `filter()` where only values that meet a certain criteria are
   * affected.
   *
   * @param $callback - The callback containing the operation to apply to the
   *                    `Vector` values.
   *
   * @return - a `Vector` containing the values after a user-specified operation
   *           is applied.
   *
   * @guide /hack/collections/examples
   */
  public function map<Tu>((function(Tv): Tu) $callback): Vector<Tu>;

  /**
   * Returns a `Vector` containing the values after an operation has been
   * applied to each key and value in the current `Vector`.
   *
   * Every key and value in the current `Vector` is affected by a call to
   * `mapWithKey()`, unlike `filterWithKey()` where only values that meet a
   * certain criteria are affected.
   *
   * @param $callback - The callback containing the operation to apply to the
   *                    `Vector` keys and values.
   *
   * @return - a `Vector` containing the values after a user-specified operation
   *           on the current `Vector`'s keys and values is applied.
   */
  public function mapWithKey<Tu>((function(int, Tv): Tu) $callback): Vector<Tu>;

  /**
   * Returns a `Vector` containing the values of the current `Vector` that meet
   * a supplied condition.
   *
   * Only values that meet a certain criteria are affected by a call to
   * `filter()`, while all values are affected by a call to `map()`.
   *
   * @param $callback - The callback containing the condition to apply to the
   *                    `Vector` values.
   *
   * @return - a `Vector` containing the values after a user-specified condition
   *           is applied.
   *
   * @guide /hack/collections/examples
   */
  public function filter((function(Tv): bool) $callback): Vector<Tv>;

  /**
   * Returns a `Vector` containing the values of the current `Vector` that meet
   * a supplied condition applied to its keys and values.
   *
   * Only keys and values that meet a certain criteria are affected by a call to
   * `filterWithKey()`, while all values are affected by a call to
   * `mapWithKey()`.
   *
   * @param $callback - The callback containing the condition to apply to the
   *                    `Vector` keys and values.
   *
   * @return - a `Vector` containing the values after a user-specified condition
   *           is applied to the keys and values of the current `Vector`.
   *
   */
  public function filterWithKey((function(int, Tv): bool) $callback):
    Vector<Tv>;

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
   *                       elements of this `Vector`.
   *
   * @return - The `Vector` that combines the values of the current `Vector`
   *           with the provided `Traversable`.
   */
  public function zip<Tu>(Traversable<Tu> $traversable): Vector<Pair<Tv, Tu>>;

  /**
   * Returns a `Vector` containing the first `n` values of the current `Vector`.
   *
   * The returned `Vector` will always be a proper subset of the current
   * `Vector`.
   *
   * `n` is 1-based. So the first element is 1, the second 2, etc.
   *
   * @param $n - The last element that will be included in the returned
   *             `Vector`.
   *
   * @return - A `Vector` that is a proper subset of the current `Vector` up to
   *           `n` elements.
   */
  public function take(int $n): Vector<Tv>;

  /**
   * Returns a `Vector` containing the values of the current `Vector` up to but
   * not including the first value that produces `false` when passed to the
   * specified callback.
   *
   * The returned `Vector` will always be a proper subset of the current
   * `Vector`.
   *
   * @param $fn - The callback that is used to determine the stopping condition.
   *
   * @return - A `Vector` that is a proper subset of the current `Vector` up
   *           until the callback returns `false`.
   */
  public function takeWhile((function(Tv): bool) $fn): Vector<Tv>;

  /**
   * Returns a `Vector` containing the values after the `n`-th element of the
   * current `Vector`.
   *
   * The returned `Vector` will always be a proper subset of the current
   * `Vector`.
   *
   * `n` is 1-based. So the first element is 1, the second 2, etc.
   *
   * @param $n - The last element to be skipped; the `$n+1` element will be the
   *             first one in the returned `Vector`.
   *
   * @return - A `Vector` that is a proper subset of the current `Vector`
   *           containing values after the specified `n`-th element.
   */
  public function skip(int $n): Vector<Tv>;

  /**
   * Returns a `Vector` containing the values of the current `Vector` starting
   * after and including the first value that produces `true` when passed to the
   * specified callback.
   *
   * The returned `Vector` will always be a proper subset of the current
   * `Vector`.
   *
   * @param $fn - The callback used to determine the starting element for the
   *              returned `Vector`.
   *
   * @return - A `Vector` that is a proper subset of the current `Vector`
   *           starting after the callback returns `true`.
   */
  public function skipWhile((function(Tv): bool) $fn): Vector<Tv>;

  /**
   * Returns a subset of the current `Vector` starting from a given key up to,
   * but not including, the element at the provided length from the starting key.
   *
   * `$start` is 0-based. `$len` is 1-based. So `slice(0, 2)` would return the
   * elements at key 0 and 1.
   *
   * The returned `Vector` will always be a proper subset of this `Vector`.
   *
   * @param $start - The starting key of this `Vector` to begin the returned
   *                 `Vector`.
   * @param $len - The length of the returned `Vector`.
   *
   * @return - A `Vector` that is a proper subset of the current `Vector`
   *           starting at `$start` up to but not including the element
   *           `$start + $len`.
   */
  public function slice(int $start, int $len): Vector<Tv>;

  /**
   * Returns a `Vector` that is the concatenation of the values of the current
   * `Vector` and the values of the provided `Traversable`.
   *
   * The values of the provided `Traversable` is concatenated to the end of the
   * current `Vector` to produce the returned `Vector`.
   *
   * @param $traversable - The `Traversable` to concatenate to this `Vector`.
   *
   * @return - The concatenated `Vector`.
   *
   * @guide /hack/generics/constraints
   */
  public function concat<Tu super Tv>(Traversable<Tu> $traversable): Vector<Tu>;

  /**
   * Returns the first value in the current `Vector`.
   *
   * @return - The first value in the current `Vector`, or `null` if the
   *           `Vector` is empty.
   */
  public function firstValue(): ?Tv;

  /**
   * Returns the first key in the current `Vector`.
   *
   * @return - The first key (an integer) in the current `Vector`, or `null` if
   *           the `Vector` is empty.
   */
  public function firstKey(): ?int;

  /**
   * Returns the last value in the current `Vector`.
   *
   * @return - The last value in the current `Vector`, or `null` if the current
   *           `Vector` is empty.
   */
  public function lastValue(): ?Tv;

  /**
   * Returns the last key in the current `Vector`.
   *
   * @return - The last key (an integer) in the current `Vector`, or `null` if
   *           the `Vector` is empty.
   */
  public function lastKey(): ?int;

  /**
   * Checks if the current `Vector` is empty.
   *
   * @return - `true` if the current `Vector` is empty; `false` otherwise.
   */
  public function isEmpty(): bool;

  /**
   * Provides the number of elements in current `Vector`.
   *
   * @return - The number of elements in current `Vector`.
   */
  public function count(): int;

  /**
   * Returns the value at the specified key in the current `Vector`.
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
   * Returns the value at the specified key in the current `Vector`.
   *
   * If the key is not present, null is returned. If you would rather have an
   * exception thrown when a key is not present, then use `at()`.
   *
   * @param $k - the key from which to retrieve the value.
   *
   * @return - The value at the specified key; or `null` if the key does not
   *           exist.
   */
  public function get(int $k): ?Tv;

  /**
   * Stores a value into the current `Vector` with the specified key,
   * overwriting the previous value associated with the key.
   *
   * If the key is not present, an exception is thrown. If you want to add
   * a value even if a key is not present, use `add()`.
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
   * @return - A shallow copy of the current `Vector` with the updated the value
   *           set. The current `Vector` is also updated.
   */
  public function set(int $k, Tv $v): Vector<Tv>;

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
   * @return - A shallow copy of the current `Vector` with the updated the
   *           values set. The current `Vector` is also updated.
   */
  public function setAll(?KeyedTraversable<int, Tv> $it): Vector<Tv>;

  /**
   * Remove all the elements from the current `Vector`.
   *
   * Future changes made to the current `Vector` ARE reflected in the
   * returned `Vector`, and vice-versa.
   *
   * @return - A shallow, empty copy of the current `Vector`. The current
   *           `Vector` is also empty.
   */
  public function clear(): Vector<Tv>;

  /**
   * Determines if the specified key is in the current `Vector`.
   *
   * @return - `true` if the specified key is present in the current `Vector`;
   *           returns `false` otherwise.
   *
   * @guide /hack/generics/constraints
   */
  public function containsKey<Tu super int>(Tu $k): bool;

  /**
   * Append a copy of a value to the end of the current `Vector`, assigning the
   * next available integer key.
   *
   * If you want to overwrite a value, use `set()`.
   *
   * `$vec->add($v)` is semantically equivalent to `$vec[] = $v` (except that
   * `add()` returns the current `Vector`).
   *
   * Future changes made to the current `Vector` ARE reflected in the
   * returned `Vector`, and vice-versa.
   *
   * @param $v - The value to set to the newly appended key
   *
   * @return - A shallow copy of the current `Vector` with the added the value
   *           set. The current `Vector` is also updated.
   */
  public function add(Tv $value): Vector<Tv>;

  /**
   * For every element in the provided `Traversable`, append a value into this
   * `Vector`, assigning the next available integer key for each.
   *
   * If you want to overwrite values, use `setAll()`.
   *
   * Future changes made to the current `Vector` ARE reflected in the
   * returned `Vector`, and vice-versa.
   *
   * @param $k - The `Traversable` with the new values to set. If `null` is
   *             provided, no changes are made.
   *
   * @return - A shallow copy of the current `Vector` with the added the values
   *           set. The current `Vector` is also updated.
   */
  public function addAll(?Traversable<Tv> $it): Vector<Tv>;

  /**
   * Adds the keys of the specified container to the current `Vector`.
   *
   * If a key of the specified container are not integers, then it adds
   * the next available integer key in this `Vector` and makes that key the
   * value.
   *
   * Future changes made to the current `Vector` ARE reflected in the
   * returned `Vector`, and vice-versa.
   *
   * @param $container - The container with the new keys to add.
   *
   * @return - A shallow copy of the current `Vector` with the new keys added;
   *           the current `Vector` is also updated.
   */
  public function addAllKeysOf<Tv2>(
    ?KeyedContainer<Tv,Tv2> $container,
  ): Vector<Tv>;

  /**
   * Removes the specified key from the current `Vector`.
   *
   * This will cause elements with higher keys to be renumbered by `n - 1`,
   * where n is the last key in the current `Vector`.
   *
   * Future changes made to the current `Vector` ARE reflected in the
   * returned `Vector`, and vice-versa.
   *
   * @param $k - The key to remove.
   *
   * @return - A shallow copy of the current `Vector` with the key removed; the
   *           current `Vector` is also updated.
   */
  public function removeKey(int $k): Vector<Tv>;

  /**
   * Remove the last element of the current `Vector` and return it.
   *
   * This function throws an exception if this `Vector` is empty.
   *
   * This `Vector` will have `n - 1` elements after this operation.
   *
   * @return - The value of the last element.
   */
  public function pop(): Tv;

  /**
   * Resize the current `Vector`.
   *
   * Resize the current `Vector` to contain `sz` elements. If `sz` is smaller
   * than the current size of the current `Vector`, elements are removed from
   * the end of the current `Vector`. If `sz` is greater than the current size
   * of the current `Vector`, the current `Vector` is extended by appending as
   * many copies of `value` as needed to reach a size of `sz` elements.
   *
   * @param $sz - The desired size of the current `Vector`.
   * @param $value - The value to use as the filler if we are increasing the
   *                 size of the current `Vector`.
   */
  public function resize(int $sz, Tv $value): void;

  /**
   * Reserves enough memory to accommodate a given number of elements.
   *
   * Reserves enough memory for `sz` elements. If `sz` is less than or
   * equal to the current capacity of the current `Vector`, this method does
   * nothing.
   *
   * @param $sz - The pre-determined size you want for the current `Vector`.
   */
  public function reserve(int $sz): void;

  /**
   * Returns an iterator that points to beginning of the current `Vector`.
   *
   * @return - A `KeyedIterator` that allows you to traverse the current
   *           `Vector`.
   */
  public function getIterator(): KeyedIterator<int, Tv>;

  /**
   * Reverse the elements of the current `Vector` in place.
   */
  public function reverse(): void;

  /**
   * Splice the current `Vector` in place.
   *
   * This function provides the functionality of
   * [`array_splice()`](http://php.net/manual/en/function.array-splice.php)
   * for `Vector`s.
   *
   * Note that this function modifies the current `Vector` in place.
   *
   * @param $offset - The key where to begin slicing, 0-based.
   * @param $len - The length of the splice. If `null`, then the current
   *               `Vector` is spliced until its end.
   *
   * @link http://php.net/manual/en/function.array-splice.php
   */
  public function splice(int $offset, ?int $len = null): void;

  /**
   * Returns the index of the first element that matches the search value.
   *
   * If no element matches the search value, this function returns -1.
   *
   * @param $search_value - The value that will be search for in the current
   *                        `Vector`.
   *
   * @return - The key (index) where that value is found; -1 if it is not found.
   *
   * @guide /hack/generics/constraints
   */
  public function linearSearch<Tu super Tv>(Tu $search_value): int;

  /**
   * Shuffles the values of the current `Vector` randomly in place.
   */
  public function shuffle(): void;

  /**
   * Returns a `Vector` containing the values from the specified `array`.
   *
   * This function is deprecated. Use `new Vector($arr)` instead.
   *
   * @param $arr - The `array` to convert to a `Vector`.
   *
   * @return - A `Vector` with the values from this `array`.
   */
  <<__Deprecated('Use `new Vector($arr)` instead.')>>
  public static function fromArray<T>(array<T, Tv> $arr): Vector<Tv>;

  /**
   * Creates a `Vector` from the given `Traversable`, or an empty `Vector` if
   * `null` is passed.
   *
   * This is the static method version of the `Vector::__construct()`
   * constructor.
   *
   * @param $items - any `Traversable` object from which to create a `Vector`
   *                 (e.g., `array`). If `null`, then an empty `Vector` is
   *                 created.
   *
   * @return - A `Vector` with the values from the `Traversable`; or an empty
   *           `Vector` if the `Traversable` is `null`.
   */
  public static function fromItems(?Traversable<Tv> $items): Vector<Tv>;

  /**
   * Creates a `Vector` from the keys of the specified container.
   *
   * If a key of the specified container are not integers, then it adds
   * the next available integer key in this `Vector` and makes that key the
   * value.
   *
   * @param $container - The container with the keys used to create the
   *                     `Vector`.
   *
   * @return - A `Vector` built from the keys of the specified container.
   */
  public static function fromKeysOf<Tk,Tv2>(
    ?KeyedContainer<Tk,Tv2> $container
  ): Vector<Tk>;

  /**
   * Returns the `string` version of this `Vector`, which is `"Vector"`.
   *
   * @return - The string `"Vector"`.
   */
  public function __toString(): string;

  /**
   * Returns an `Iterable` view of the current `Vector`.
   *
   * The `Iterable` returned is one that produces the values from the current
   * `Vector`.
   *
   * @return - The `Iterable` view of this `Vector`.
   */
  public function items(): Iterable<Tv>;
}

/**
 * @internal
 *
 * Methods and functions should take and return the KeyedIterator interface.
 */
class VectorIterator<+Tv> implements KeyedIterator<int, Tv> {
  public function __construct();
  public function rewind(): void;
  public function current(): Tv;
  public function key(): int;
  public function next(): void;
  public function valid(): bool;
}
