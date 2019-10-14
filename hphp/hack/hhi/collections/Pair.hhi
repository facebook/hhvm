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
 * `Pair` is an immutable, fixed-size collection with exactly two elements
 * (possibly of different types). HHVM provides a native implementation for
 * this class.  The Hack class definition below is not actually used at run
 * time; it is simply provided for the typechecker and for developer reference.
 *
 * Like all objects in PHP, `Pair`s have reference-like semantics. The elements
 * of a `Pair` cannot be mutated (i.e. you can't assign to the elements of a
 * `Pair`) though `Pair`s may contain mutable objects.
 *
 * `Pair`s only support integer keys. If a non-integer key is used, an
 * exception will be thrown.
 *
 * `Pair`s support `$m[$k]` style syntax for getting values by key. `Pair`s
 * also support `isset($m[$k])` and `empty($m[$k])` syntax, and they provide
 * similar semantics as arrays.
 *
 * `Pair`s do not support taking elements by reference. If binding assignment
 * (`=&`) is used with an element of a `Pair`, if an element of a `Pair` is
 * passed by reference, or if a `Pair` is used with foreach by reference, an
 * exception will be thrown.
 *
 * `Pair` keys are always 0 and 1, respectively.
 *
 * You may notice that many methods affecting the instace of `Pair` return an
 * `ImmVector` -- `Pair`s are essentially backed by 2-element `ImmVector`s.
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/classes
 */

final class Pair<+Tv1, +Tv2> implements \ConstVector<mixed> {
  /**
   * @internal
   *
   * Pairs must be constructed with "Pair {$first, $second}".
   */
  <<__Rx>>
  private function __construct();

  /**
   * Returns an `array` containing the values from the current `Pair`.
   *
   * This method is interchangeable with `toValuesArray()`.
   *
   * @return - an `array` containing the values from the current `Pair`.
   */
  <<__Rx, __MaybeMutable, __PHPStdLib>>
  public function toArray(): array;

  /**
   * Returns an `array` containing the values from the current `Pair`.
   *
   * This method is interchangeable with `toArray()`.
   *
   * @return - an `array` containing the values from the current `Pair`.
   */
  <<__Rx, __MaybeMutable>>
  /* HH_FIXME[4110] pair needs to extend ConstVector<Tv1|Tv2> */
  /* HH_FIXME[4341] pair needs to extend ConstVector<Tv1|Tv2> */
  public function toValuesArray<Tu>(): varray<Tu>
    where Tv1 as Tu, Tv2 as Tu;

  /**
   * Returns an `array` whose values are the keys from the current `Pair`.
   *
   * @return - an `array` with the integer keys from the current `Pair`.
   */
  <<__Rx, __MaybeMutable>>
  public function toKeysArray(): varray<int>;

 /**
   * Returns a `Vector` containing the elements of the current `Pair`.
   *
   * The returned `Vector` will, of course, be mutable.
   *
   * @return - a `Vector` with the elements of the current `Pair`.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function toVector(): Vector<mixed>;

 /**
   * Returns an immutable vector (`ImmVector`) containing the elements of the
   * current `Pair`.
   *
   * @return - an `ImmVector` with the elements of the current `Pair`.
   */
  <<__Rx, __MaybeMutable>>
  public function toImmVector(): ImmVector<mixed>;

  /**
   * Returns an integer-keyed `Map` based on the elements of the current `Pair`.
   *
   * The keys are 0 and 1.
   *
   * @return - an integer-keyed `Map` with the values of the current `Pair`.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function toMap(): Map<int, mixed>;

  /**
   * Returns an immutable, integer-keyed map (`ImmMap`) based on the elements of
   * the current `Pair`.
   *
   * The keys are 0 and 1.
   *
   * @return - an `ImmMap` with the values of the current `Pair`.
   */
  <<__Rx, __MaybeMutable>>
  public function toImmMap(): ImmMap<int, mixed>;

  /**
   * Returns a `Set` with the values of the current `Pair`.
   *
   * @return - a `Set` with the current values of the current `Pair`.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  /* HH_FIXME[4110] pair needs to extend ConstVector<Tv1|Tv2> */
  /* HH_FIXME[4341] pair needs to extend ConstVector<Tv1|Tv2> */
  public function toSet(): Set<arraykey> where Tv1 as arraykey, Tv2 as arraykey;

  /**
   * Returns an immutable set (`ImmSet`) with the values of the current `Pair`.
   *
   * @return - an `ImmSet` with the current values of the current `Pair`.
   */
  <<__Rx, __MaybeMutable>>
  /* HH_FIXME[4110] pair needs to extend ConstVector<Tv1|Tv2> */
  /* HH_FIXME[4341] pair needs to extend ConstVector<Tv1|Tv2> */
  public function toImmSet(): ImmSet<arraykey> where Tv1 as arraykey, Tv2 as arraykey;

  /**
   * Returns a lazy, access elements only when needed view of the current
   * `Pair`.
   *
   * Normally, memory is allocated for all of the elements of the `Pair`.
   * With a lazy view, memory is allocated for an element only when needed or
   * used in a calculation like in `map()` or `filter()`.
   *
   * That said, `Pair`s only have two elements. So the performance impact on
   * a `Pair` will be not be noticeable in the real world.
   *
   * @return - an integer-keyed KeyedIterable representing the lazy view into
   *           the current `Pair`.
   *
   * @guide /hack/collections/examples
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function lazy(): \HH\Rx\KeyedIterable<int, mixed>;

  /**
   * Returns an `ImmVector` containing the values of the current `Pair`.
   *
   * This method is interchangeable with `toImmVector()`.
   *
   * @return - an `ImmVector` containing the values of the current `Pair`.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function values(): ImmVector<mixed>;

  /**
   * Returns an `ImmVector` with the values being the keys of the current
   * `Pair`.
   *
   * This method will return an `ImmVector` with keys 0 and 1, and values 0 and
   * 1, since the keys of a `Pair` are 0 and 1.
   *
   * @return - an `ImmVector` containing the integer keys of the current
   *           `Pair` as values.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function keys(): ImmVector<int>;

  /**
   * Returns an `ImmVector` containing the values after an operation has been
   * applied to each value in the current `Pair`.
   *
   * Every value in the current Pair is affected by a call to `map()`, unlike
   * `filter()` where only values that meet a certain criteria are affected.
   *
   * @param $callback - The callback containing the operation to apply to the
   *                    current `Pair` values.
   *
   * @return - an `ImmVector` containing the values after a user-specified
   *           operation is applied.
   *
   * @guide /hack/collections/examples
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function map<Tu>(<<__AtMostRxAsFunc>>(function(mixed): Tu) $callback): ImmVector<Tu>;

  /**
   * Returns an `ImmVector` containing the values after an operation has been
   * applied to each key and value in the current `Pair`.
   *
   * Every key and value in the current `Pair` is affected by a call to
   * `mapWithKey()`, unlike `filterWithKey()` where only values that meet a
   * certain criteria are affected.
   *
   * @param $callback - The $allback containing the operation to apply to the
   *                    current `Pair` keys and values.
   *
   * @return - an `ImmVector` containing the values after a user-specified
   *           operation on the current `Pair`'s keys and values is applied.
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function mapWithKey<Tu>(<<__AtMostRxAsFunc>>(function(int, mixed): Tu) $callback):
    ImmVector<Tu>;

  /**
   * Returns a `ImmVector` containing the values of the current `Pair` that
   * meet a supplied condition.
   *
   * Only values that meet a certain criteria are affected by a call to
   * `filter()`, while all values are affected by a call to `map()`.
   *
   * @param $callback - The callback containing the condition to apply to the
   *                    current `Pair` values.
   *
   * @return - an `ImmVector` containing the values after a user-specified
   *           condition is applied.
   *
   * @guide /hack/collections/examples
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filter(<<__AtMostRxAsFunc>>(function(mixed): bool) $callback): ImmVector<mixed>;

  /**
   * Returns an `ImmVector` containing the values of the current `Pair` that
   * meet a supplied condition applied to its keys and values.
   *
   * Only keys and values that meet a certain criteria are affected by a call to
   * `filterWithKey()`, while all values are affected by a call to
   * `mapWithKey()`.
   *
   * @param $callback - The callback containing the condition to apply to the
   *                    current `Pair` keys and values.
   *
   * @return - an `ImmVector` containing the values after a user-specified
   *           condition is applied to the keys and values of the current
   *           `Pair`.
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filterWithKey(<<__AtMostRxAsFunc>>(function(int, mixed): bool) $callback):
    ImmVector<mixed>;

  /**
   * Returns an `ImmVector` where each element is a `Pair` that combines each
   * element of the current `Pair` and the provided `Traversable`.
   *
   * If the number of elements of the current `Pair` are not equal to the
   * number of elements in the `Traversable`, then only the combined elements
   * up to and including the final element of the one with the least number of
   * elements is included.
   *
   * @param $traversable - The `Traversable` to use to combine with the
   *                       elements of the current `Pair`.
   *
   * @return - The `ImmVector` that combines the values of the current `Pair`
   *           with the provided `Traversable`.
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function zip<Tu>(<<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\Traversable::class)>> Traversable<Tu> $traversable):
    ImmVector<Pair<mixed, Tu>>;

  /**
   * Returns an `ImmVector` containing the first `n` values of the current
   * `Pair`.
   *
   * `n` is 1-based. So the first element is 1, the second 2. There is no
   * element 3 in a `Pair`, but if you specify an element greater than 2, it
   * will just return all elements in the `Pair`.
   *
   * @param $n - The last element that will be included in the current `Pair`.
   *
   * @return - An `ImmVector` containing the first `n` values of the current
   *           `Pair`.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function take(int $n): ImmVector<mixed>;

  /**
   * Returns an `ImmVector` containing the values of the current `Pair` up to
   * but not including the first value that produces `false` when passed to the
   * specified callback.
   *
   * @param $fn - The callback that is used to determine the stopping condition.
   *
   * @return - An `ImmVector` that contains the values of the current `Pair` up
   *           until the callback returns `false`.
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function takeWhile(<<__AtMostRxAsFunc>>(function(mixed): bool) $fn): ImmVector<mixed>;

  /**
   * Returns an `ImmVector` containing the values after the `n`-th element of
   * the current `Pair`.
   *
   * `n` is 1-based. So the first element is 1, the second 2, etc. There is no
   * element 3 in a `Pair`, but if you specify an element greater than or equal
   * to 2, it will just return empty. If you specify 0, it will return all the
   * elements in the `Pair`.
   *
   * @param $n - The last element to be skipped; the `$n+1` element will be the
   *             first one in the returned `ImmVector`.
   *
   * @return - An `ImmVector` that contains values after the specified `n`-th
   *           element in the current `Pair`.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function skip(int $n): ImmVector<mixed>;

  /**
   * Returns an `ImmVector` containing the values of the current `Pair` starting
   * after and including the first value that produces `true` when passed to
   * the specified callback.
   *
   * @param $fn - The callback used to determine the starting element for the
   *              `ImmVector`.
   *
   * @return - An `ImmVector` that contains the values of the current `Pair`
   *           starting after the callback returns `true`.
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function skipWhile(<<__AtMostRxAsFunc>>(function(mixed): bool) $fn): ImmVector<mixed>;

  /**
   * Returns a subset of the current `Pair` starting from a given key up to,
   * but not including, the element at the provided length from the starting
   * key.
   *
   * `$start` is 0-based. `$len` is 1-based. So `slice(0, 2)` would return the
   * elements at key 0 and 1 (all of the current `Pair` elements).
   *
   * @param $start - The starting key of the current `Pair` to begin the
   *                 returned `ImmVector`.
   * @param $len - The length of the returned `ImmVector`.
   *
   * @return - An `ImmVector` with values from the current `Pair` starting at
   *           `$start` up to but not including the element `$start + $len`.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function slice(int $start, int $len): ImmVector<mixed>;

  /**
   * Returns an `ImmVector` that is the concatenation of the values of the
   * current `Pair` and the values of the provided `Traversable`.
   *
   * The values of the provided `Traversable` is concatenated to the end of the
   * current `Pair` to produce the returned `ImmVector`.
   *
   * @param $traversable - The `Traversable` to concatenate to the current
   *                       `Pair`.
   *
   * @return - The concatenated `ImmVector`.
   *
   * @guide /hack/generics/constraints
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function concat(<<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\Traversable::class)>> Traversable<mixed> $traversable):
    ImmVector<mixed>;

  /**
   * Returns the first value in the current `Pair`.
   *
   * @return - The first value in the current `Pair`.
   */
  <<__Rx, __MaybeMutable>>
  public function firstValue(): Tv1;

  /**
   * Returns the first key in the current `Pair`.
   *
   * The return will always be 0 since a `Pair` only has two keys, 0 and 1.
   *
   * @return - 0
   */
  <<__Rx, __MaybeMutable>>
  public function firstKey(): int;

  /**
   * Returns the last value in the current `Pair`.
   *
   * @return - The last value in the current `Pair`.
   */
  <<__Rx, __MaybeMutable>>
  public function lastValue(): Tv2;

  /**
   * Returns the last key in the current `Pair`.
   *
   * The return will always be 1 since a `Pair` only has two keys, 0 and 1.
   *
   * @return - 0
   */
  <<__Rx, __MaybeMutable>>
  public function lastKey(): int;

  /**
   * Returns the index of the first element that matches the search value.
   *
   * If no element matches the search value, this function returns -1.
   *
   * @param $search_value - The value that will be search for in the current
   *                        `Pair`.
   *
   * @return - The key (index) where that value is found; -1 if it is not found.
   *
   * @guide /hack/generics/constraints
   */
  <<__Rx, __MaybeMutable>>
  public function linearSearch(mixed $search_value): int;

  /**
   * Returns `false`; a `Pair` cannot be empty.
   *
   * @return - `false`
   */
  <<__Rx, __MaybeMutable>>
  public function isEmpty(): bool;

  /**
   * Returns 2; a `Pair` always has two values.
   *
   * @return - 2
   */
  <<__Rx, __MaybeMutable>>
  public function count(): int;

  /**
   * Returns an `Iterable` view of the current `Pair`.
   *
   * The `Iterable` returned is one that produces the values from the current
   * `Pair`.
   *
   * @return - The `Iterable` view of the current `Pair`.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function items(): \HH\Rx\Iterable<mixed>;

  /**
   * Returns the value at the specified key in the current `Pair`.
   *
   * If the key is not present, an exception is thrown. This essentially means
   * if you specify a key other than 0 or 1, you will get an exception. If you
   * don't want an exception to be thrown, use `get()` instead.
   *
   * $v = $p->at($k)" is semantically equivalent to `$v = $p[$k]`.
   *
   * @param $k - the key from which to retrieve the value.
   *
   * @return - The value at the specified key; or an exception if the key does
   *           not exist.
   */
  <<__Rx, __MaybeMutable>>
  public function at(int $k): mixed;

  /**
   * Returns the value at the specified key in the current `Pair`.
   *
   * If the key is not present, `null` is returned. If you would rather have an
   * exception thrown when a key is not present, then use `at()`.
   *
   * @param $k - the key from which to retrieve the value.
   *
   * @return - The value at the specified key; or `null` if the key does not
   *           exist.
   */
  <<__Rx, __MaybeMutable>>
  public function get(int $k): mixed;

  /**
   * Checks whether a provided key exists in the current `Pair`.
   *
   * This will only return `true` for provided keys of 0 and 1 since those are
   * the only two keys that can exist in a `Pair`.
   *
   * @param $k - the key to check existance for in the `Pair`.
   *
   * @return - `true` if the provided key exists in the `Pair`; `false`
   *           otherwise. This will only return `true` if the provided key is
   *           0 or 1.
   */
  <<__Rx, __MaybeMutable>>
  public function containsKey(mixed $k): bool;

  /**
   * Returns an iterator that points to beginning of the current `Pair`.
   *
   * @return - A `KeyedIterator` that allows you to traverse the current `Pair`.
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function getIterator(): \HH\Rx\KeyedIterator<int, mixed>;

  /**
   * Returns the `string` version of the current `Pair`, which is `"Pair"`.
   *
   * @return - The `string` "Pair".
   */
  <<__Rx, __MaybeMutable>>
  public function __toString(): string;

  <<__Rx, __MaybeMutable>> /* HH_FIXME[0002] */
  public function toVArray(): varray;
  <<__Rx, __MaybeMutable>> /* HH_FIXME[0001] */
  public function toDArray(): darray<int, mixed>;
}

} // namespace HH

namespace {

/**
 * @internal
 *
 * Methods and functions should take and return the KeyedIterator interface.
 */
class PairIterator implements HH\Rx\KeyedIterator<int, mixed> {
  <<__Rx>>
  public function __construct();
  <<__Rx, __MaybeMutable>>
  public function current(): mixed;
  <<__Rx, __MaybeMutable>>
  public function key(): int;
  <<__Rx, __MaybeMutable>>
  public function valid(): bool;
  <<__Rx, __Mutable>>
  public function next(): void;
  <<__Rx, __Mutable>>
  public function rewind(): void;
}

} // namespace
