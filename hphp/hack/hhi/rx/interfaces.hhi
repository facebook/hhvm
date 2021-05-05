<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/**
 * This file provides type information for some of hack's reactive interfaces
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

namespace HH\Rx;

/* See documentation for \HH\Traversable */
<<__Sealed(
  namespace\KeyedTraversable::class,
  namespace\Iterator::class,
  namespace\IteratorAggregate::class,
  \HH\Container::class
)>>
interface Traversable<+Tv> extends \HH\Traversable<Tv> {}

/* See documentation for \HH\KeyedTraversable */
<<__Sealed(
  namespace\KeyedIterable::class,
  namespace\KeyedIterator::class,
  \HH\KeyedContainer::class
)>>
interface KeyedTraversable<+Tk, +Tv>
  extends namespace\Traversable<Tv>, \HH\KeyedTraversable<Tk, Tv> {}

/* See documentation for \HH\Iterator */
interface Iterator<+Tv> extends namespace\Traversable<Tv>, \HH\Iterator<Tv> {
  /**
   * Return the current value at the current iterator position.
   *
   * @return - The current value of type `Tv`.
   */
  public function current()[]: Tv;
  /**
   * Move the iterator position to the next element.
   *
   */
  public function next()[write_props]: void;
  /**
   * Rewind the iterator position to its beginning.
   *
   * This rewinds back to the first element of the `Iterator`.
   */
  public function rewind()[write_props]: void;
  /**
   * Checks to see if the current iterator position is valid.
   *
   * This method is called after `rewind()` and `next()` to check if the
   * current iterator position is valid.
   *
   * @return - `true` if the position is valid; `false` otherwise.
   */
  public function valid()[]: bool;
}

interface KeyedIterator<+Tk, +Tv>
  extends
    namespace\KeyedTraversable<Tk, Tv>,
    namespace\Iterator<Tv>,
    \HH\KeyedIterator<Tk, Tv> {
  /**
   * Return the current key at the current iterator position.
   *
   * @return - The current key of type `Tk`.
   */
  public function key()[]: Tk;
}

/* See documentation for \AsyncIterator */
interface AsyncIterator<+Tv> extends \HH\AsyncIterator<Tv> {
  /**
   * Move the async iterator to the next `Awaitable` position.
   *
   * ```
   * foreach ($async_iter await $async_iter->next() $value)
   * ```
   *
   * The above is the longhand syntax for `await as $value`.
   *
   * @return - The next `Awaitable` in the iterator sequence.
   */
  public function next()[write_props]: Awaitable<?(mixed, Tv)>;
}

/* See documentation for \AsyncKeyedIterator */
interface AsyncKeyedIterator<+Tk, +Tv>
  extends namespace\AsyncIterator<Tv>, \HH\AsyncKeyedIterator<Tk, Tv> {
  /**
   * Move the async iterator to the next `Awaitable` position.
   *
   * ```
   * foreach ($async_iter await $async_iter->next() $key=>$value)
   * ```
   *
   * The above is the longhand syntax for `await as $key=>$value`.
   *
   * @return - The next `Awaitable` in the iterator sequence.
   */
  public function next()[write_props]: Awaitable<?(Tk, Tv)>;
}

/* See documentation for \IteratorAggregate */
interface IteratorAggregate<+Tv>
  extends namespace\Traversable<Tv>, \IteratorAggregate<Tv> {
  /**
   * Returns an iterator to be used to iterate over the object's elements.
   *
   * @return - An `Iterator` for iteration.
   */
  public function getIterator()[]: namespace\Iterator<Tv>;
}

interface Iterable<+Tv>
  extends namespace\IteratorAggregate<Tv>, \HH\Iterable<Tv> {
  /**
   * Returns an iterator that points to beginning of the current `Iterable`.
   *
   * @return - An `Iterator` that allows you to traverse the current `Iterable`.
   */
  public function getIterator()[]: namespace\Iterator<Tv>;
  /**
   * Returns an `array` with the values from the current `Iterable`.
   *
   * The keys in the current `Iterable` are discarded and replaced with integer
   * indices, starting with 0.
   *
   * @return - an `array` containing the values from the current `Iterable`.
   */
  public function toValuesArray()[]: varray<Tv>;
  /**
   * Returns an immutable vector (`ImmVector`) converted from the current
   * `Iterable`.
   *
   * Any keys in the current `Iterable` are discarded and replaced with integer
   * indices, starting with 0.
   *
   * @return - an `ImmVector` converted from the current `Iterable`.
   */
  public function toImmVector()[]: ImmVector<Tv>;
  /**
   * Returns an immutable set (`ImmSet`) converted from the current `Iterable`.
   *
   * Any keys in the current `Iterable` are discarded.
   *
   * @return - an `ImmSet` converted from the current `Iterable`.
   */
  public function toImmSet()[]: ImmSet<Tv> where Tv as arraykey;
  /**
   * Returns a lazy, access elements only when needed view of the current
   * `Iterable`.
   *
   * Normally, memory is allocated for all of the elements of the `Iterable`.
   * With a lazy view, memory is allocated for an element only when needed or
   * used in a calculation like in `map()` or `filter()`.
   *
   * @return - an `Iterable` representing the lazy view into the current
   *           `Iterable`.
   *
   * @guide /hack/collections/examples
   */
  public function lazy()[]: namespace\Iterable<Tv>;
  /**
   * Returns an `Iterable` containing the current `Iterable`'s values.
   *
   * Any keys are discarded.
   *
   * @return An `Iterable` with the values of the current `Iterable`.
   */
  public function values()[]: namespace\Iterable<Tv>;
  /**
   * Returns an `Iterable` containing the values after an operation has been
   * applied to each value in the current `Iterable`.
   *
   * Every value in the current `Iterable` is affected by a call to `map()`,
   * unlike `filter()` where only values that meet a certain criteria are
   * affected.
   *
   * @param $fn - The callback containing the operation to apply to the
   *              `Iterable` values.
   *
   * @return - an `Iterable` containing the values after a user-specified
   *           operation is applied.
   *
   * @guide /hack/collections/examples
   */
  public function map<Tu>(
    (function(Tv)[_]: Tu) $fn,
  )[ctx $fn]: namespace\Iterable<Tu>;
  /**
   * Returns an `Iterable` containing the values of the current `Iterable` that
   * meet a supplied condition.
   *
   * Only values that meet a certain criteria are affected by a call to
   * `filter()`, while all values are affected by a call to `map()`.
   *
   * @param $fn - The callback containing the condition to apply to the
   *              `Itearble` values.
   *
   * @return - an `Iterable` containing the values after a user-specified
   *           condition is applied.
   *
   * @guide /hack/collections/examples
   */
  public function filter(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: namespace\Iterable<Tv>;
  /**s
   *  Returns an `Iterable` where each element is a `Pair` that combines the
   *  element of the current `Iterable` and the provided `Traversable`.
   *
   *  If the number of elements of the `Iterable` are not equal to the number of
   *  elements in the `Traversable`, then only the combined elements up to and
   *  including the final element of the one with the least number of elements
   *  is included.
   *
   *  @param $traversable - The `Traversable` to use to combine with the
   *                        elements of the current `Iterable`.
   *
   *  @return - The `Iterable` that combines the values of the current
   *            `Itearable` with the provided `Traversable`.
   */
  public function zip<Tu>(
    \HH\Traversable<Tu> $traversable,
  )[]: namespace\Iterable<Pair<Tv, Tu>>;
  /**
   * Returns an `Iterable` containing the first `n` values of the current
   * `Iterable`.
   *
   * The returned `Iterable` will always be a proper subset of the current
   * `Iterable`.
   *
   * `$n` is 1-based. So the first element is 1, the second 2, etc.
   *
   * @param $n - The last element that will be included in the returned
   *             `Iterable`.
   *
   * @return - An `Iterable that is a proper subset of the current `Iterable`
   *           up to `n` elements.
   */
  public function take(int $n)[]: namespace\Iterable<Tv>;
  /**
   * Returns an `Iterable` containing the values of the current `Iterable` up
   * to but not including the first value that produces `false` when passed to
   * the specified callback.
   *
   * The returned `Iterable` will always be a proper subset of the current
   * `Iterable`.
   *
   * @param $fn - The callback that is used to determine the stopping
   *              condition.
   *
   * @return - An `Iterable` that is a proper subset of the current `Iterable`
   *           up until the callback returns `false`.
   */
  public function takeWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: namespace\Iterable<Tv>;
  /**
   * Returns an `Iterable` containing the values after the `n`-th element of the
   * current `Iterable`.
   *
   * The returned `Iterable` will always be a proper subset of the current
   * `Iterable`.
   *
   * `$n` is 1-based. So the first element is 1, the second 2, etc.
   *
   * @param $n - The last element to be skipped; the `$n+1` element will be
   *             the first one in the returned `Iterable`.
   *
   * @return - An `Iterable` that is a proper subset of the current `Iterable`
   *           containing values after the specified `n`-th element.
   */
  public function skip(int $n)[]: namespace\Iterable<Tv>;
  /**
   * Returns an `Iterable` containing the values of the current `Iterable`
   * starting after and including the first value that produces `true` when
   * passed to the specified callback.
   *
   * The returned `Iterable` will always be a proper subset of the current
   * `Iterable`.
   *
   * @param $fn - The callback used to determine the starting element for the
   *              returned `Iterable`.
   *
   * @return - An `Iterable` that is a proper subset of the current `Iterable`
   *           starting after the callback returns `true`.
   */
  public function skipWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: namespace\Iterable<Tv>;
  /**
   * Returns a subset of the current `Iterable` starting from a given key up
   * to, but not including, the element at the provided length from the
   * starting key.
   *
   * `$start` is 0-based. `$len` is 1-based. So `slice(0, 2)` would return the
   * elements at key 0 and 1.
   *
   * The returned `Iterable` will always be a proper subset of the current
   * `Iterable`.
   *
   * @param $start - The starting key of the current `Iterable` to begin the
   *                 returned `Iterable`.
   * @param $len - The length of the returned `Iterable`.
   *
   * @return - An `Iterable` that is a proper subset of the current `Iterable`
   *           starting at `$start` up to but not including the element
   *           `$start + $len`.
   */
  public function slice(int $start, int $len)[]: namespace\Iterable<Tv>;
  /**
   * Returns an `Iterable` that is the concatenation of the values of the
   * current `Iterable` and the values of the provided `Traversable`.
   *
   * The values of the provided `Traversable` is concatenated to the end of the
   * current `Iterable` to produce the returned `Iterable`.
   *
   * @param $traversable - The `Traversable` to concatenate to the current
   *                       `Iterable`.
   *
   * @return - The concatenated `Iterable`.
   *
   * @guide /hack/generics/constraints
   */
  public function concat<Tu super Tv>(
    \HH\Traversable<Tu> $traversable,
  )[]: namespace\Iterable<Tu>;
  /**
   * Returns the first value in the current `Iterable`.
   *
   * @return - The first value in the current `Iterable`, or `null` if the
   *           current `Iterable` is empty.
   */
  public function firstValue()[]: ?Tv;
  /**
   * Returns the last value in the current `Iterable`.
   *
   * @return - The last value in the current `Iterable`, or `null` if the
   *           current `Iterable` is empty.
   */
  public function lastValue()[]: ?Tv;
}

interface KeyedIterable<Tk, +Tv>
  extends
    namespace\KeyedTraversable<Tk, Tv>,
    namespace\Iterable<Tv>,
    \HH\KeyedIterable<Tk, Tv> {
  /**
   * Returns an iterator that points to beginning of the current
   * `KeyedIterable`.
   *
   * @return - A `KeyedIterator` that allows you to traverse the current
   *           `KeyedIterable`.
   */
  public function getIterator()[]: namespace\KeyedIterator<Tk, Tv>;
  /**
   * Returns an `array` with the keys from the current `KeyedIterable`.
   *
   * @return - an `array` containing the values from the current
   *           `KeyedIterable`.
   */
  public function toKeysArray()[]: varray;
 /**
   * Returns an immutable map (`ImmMap`) based on the keys and values of the
   * current `KeyedIterable`.
   *
   * @return - an `ImmMap` that has the keys and associated values of the
   *           current `KeyedIterable`.
   */
  public function toImmMap()[]: ImmMap<Tk, Tv> where Tk as arraykey;
  /**
   * Returns a lazy, access elements only when needed view of the current
   * `KeyedIterable`.
   *
   * Normally, memory is allocated for all of the elements of the
   * `KeyedIterable`. With a lazy view, memory is allocated for an element only
   * when needed or used in a calculation like in `map()` or `filter()`.
   *
   * @return - a `KeyedIterable` representing the lazy view into the current
   *           `KeyedIterable`.
   *
   * @guide /hack/collections/examples
   */
  public function lazy()[]: namespace\KeyedIterable<Tk, Tv>;
  /**
   * Returns an `Iterable` containing the current `KeyedIterable`'s values.
   *
   * Any keys are discarded.
   *
   * @return An `Iterable` with the values of the current `KeyedIterable`.
   */
  public function values()[]: namespace\Iterable<Tv>;
  /**
   * Returns an `Iterable` containing the current `KeyedIterable`'s keys.
   *
   * Any values are discarded.
   *
   * @return An `Iterable` with the keys of the current `KeyedIterable`.
   */
  public function keys()[]: namespace\Iterable<Tk>;
  /**
   * Returns a `KeyedIterable` containing the values after an operation has been
   * applied to each value in the current `KeyedIterable`.
   *
   * Every value in the current `KeyedIterable` is affected by a call to
   * `map()`, unlike `filter()` where only values that meet a certain criteria
   * are affected.
   *
   * @param $fn - The callback containing the operation to apply to the
   *              `KeyedIterable` values.
   *
   * @return - a `KeyedIterable` containing the values after a user-specified
   *           operation is applied.
   *
   * @guide /hack/collections/examples
   */
  public function map<Tu>(
    (function(Tv)[_]: Tu) $fn,
  )[ctx $fn]: namespace\KeyedIterable<Tk, Tu>;
  /**
   * Returns a `KeyedIterable` containing the values after an operation has
   * been applied to each key and value in the current `KeyedIterable`.
   *
   * Every key and value in the current `KeyedIterable` is affected by a call to
   * `mapWithKey()`, unlike `filterWithKey()` where only values that meet a
   * certain criteria are affected.
   *
   * @param $fn - The callback containing the operation to apply to the
   *              `KeyedIterable` keys and values.
   *
   * @return - a `KeyedIterable` containing the values after a user-specified
   *           operation on the current `KeyedIterable`'s keys and values is
   *           applied.
   */
  public function mapWithKey<Tu>(
    (function(Tk, Tv)[_]: Tu) $fn,
  )[ctx $fn]: namespace\KeyedIterable<Tk, Tu>;
  /**
   * Returns a `KeyedIterable` containing the values of the current
   * `KeyedIterable` that meet a supplied condition.
   *
   * Only values that meet a certain criteria are affected by a call to
   * `filter()`, while all values are affected by a call to `map()`.
   *
   * @param $fn - The callback containing the condition to apply to the
   *              `KeyedItearble` values.
   *
   * @return - a `KeyedIterable` containing the values after a user-specified
   *           condition is applied.
   *
   * @guide /hack/collections/examples
   */
  public function filter(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: namespace\KeyedIterable<Tk, Tv>;
  /**
   * Returns a `KeyedIterable` containing the values of the current
   * `KeyedIterable` that meet a supplied condition applied to its keys and
   * values.
   *
   * Only keys and values that meet a certain criteria are affected by a call to
   * `filterWithKey()`, while all values are affected by a call to
   * `mapWithKey()`.
   *
   * @param $fn - The callback containing the condition to apply to the
   *              `KeyedIterable` keys and values.
   *
   * @return - a `KeyedIterable` containing the values after a user-specified
   *           condition is applied to the keys and values of the current
   *           `KeyedIterable`.
   *
   */
  public function filterWithKey(
    (function(Tk, Tv)[_]: bool) $fn,
  )[ctx $fn]: namespace\KeyedIterable<Tk, Tv>;
  /**
   *  Returns a `KeyedIterable` where each element is a `Pair` that combines the
   *  element of the current `KeyedIterable` and the provided `Traversable`.
   *
   *  If the number of elements of the `KeyedIterable` are not equal to the
   *  number of elements in the `Traversable`, then only the combined elements
   *  up to and including the final element of the one with the least number of
   *  elements is included.
   *
   *  @param $traversable - The `Traversable` to use to combine with the
   *                        elements of the current `KeyedIterable`.
   *
   *  @return - The `KeyedIterable` that combines the values of the current
   *            `KeyedItearable` with the provided `Traversable`.
   */
  public function zip<Tu>(
    \HH\Traversable<Tu> $traversable,
  )[]: namespace\KeyedIterable<Tk, Pair<Tv, Tu>>;
  /**
   * Returns a `KeyedIterable` containing the first `n` values of the current
   * `KeyedIterable`.
   *
   * The returned `KeyedIterable` will always be a proper subset of the current
   * `Iterable`.
   *
   * `$n` is 1-based. So the first element is 1, the second 2, etc.
   *
   * @param $n - The last element that will be included in the returned
   *             `KeyedIterable`.
   *
   * @return - A `KeyedIterable that is a proper subset of the current
   *           `KeyedIterable` up to `n` elements.
   */
  public function take(int $n)[]: namespace\KeyedIterable<Tk, Tv>;
  /**
   * Returns a `KeyedIterable` containing the values of the current
   * `KeyedIterable` up to but not including the first value that produces
   * `false` when passed to the specified callback.
   *
   * The returned `KeyedIterable` will always be a proper subset of the current
   * `KeyedIterable`.
   *
   * @param $fn - The callback that is used to determine the stopping
   *              condition.
   *
   * @return - A `KeyedIterable` that is a proper subset of the current
   *           `KeyedIterable` up until the callback returns `false`.
   */
  public function takeWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: namespace\KeyedIterable<Tk, Tv>;
  /**
   * Returns a `KeyedIterable` containing the values after the `n`-th element
   * of the current `KeyedIterable`.
   *
   * The returned `KeyedIterable` will always be a proper subset of the current
   * `KeyedIterable`.
   *
   * `$n` is 1-based. So the first element is 1, the second 2, etc.
   *
   * @param $n - The last element to be skipped; the `$n+1` element will be
   *             the first one in the returned `KeyedIterable`.
   *
   * @return - A `KeyedIterable` that is a proper subset of the current
   *           `KeyedIterable`  containing values after the specified `n`-th
   *           element.
   */
  public function skip(int $n)[]: namespace\KeyedIterable<Tk, Tv>;
  /**
   * Returns a `KeyedIterable` containing the values of the current
   * `KeyedIterable` starting after and including the first value that produces
   * `true` when passed to the specified callback.
   *
   * The returned `KeyedIterable` will always be a proper subset of the current
   * `KeyedIterable`.
   *
   * @param $fn - The callback used to determine the starting element for the
   *              returned `KeyedIterable`.
   *
   * @return - A `KeyedIterable` that is a proper subset of the current
   *           `KeyedIterable` starting after the callback returns `true`.
   */
  public function skipWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: namespace\KeyedIterable<Tk, Tv>;
  /**
   * Returns a subset of the current `KeyedIterable` starting from a given key
   * up to, but not including, the element at the provided length from the
   * starting key.
   *
   * `$start` is 0-based. `$len` is 1-based. So `slice(0, 2)` would return the
   * elements at key 0 and 1.
   *
   * The returned `KeyedIterable` will always be a proper subset of the current
   * `KeyedIterable`.
   *
   * @param $start - The starting key of the current `KeyedIterable` to begin
   *                 the returned `KeyedIterable`.
   * @param $len - The length of the returned `KeyedIterable`.
   *
   * @return - A `KeyedIterable` that is a proper subset of the current
   *           `KeyedIterable` starting at `$start` up to but not including the
   *           element `$start + $len`.
   */
  public function slice(int $start, int $len)[]: namespace\KeyedIterable<Tk, Tv>;
  /**
   * Returns an `Iterable` that is the concatenation of the values of the
   * current `KeyedIterable` and the values of the provided `Traversable`.
   *
   * The values of the provided `Traversable` is concatenated to the end of the
   * current `KeyedIterable` to produce the returned `Iterable`.
   *
   * @param $traversable - The `Traversable` to concatenate to the current
   *                       `KeyedIterable`.
   *
   * @return - The concatenated `Iterable`.
   *
   * @guide /hack/generics/constraints
   */
  public function concat<Tu super Tv>(
    \HH\Traversable<Tu> $traversable,
  )[]: namespace\Iterable<Tu>;
  /**
   * Returns the first value in the current `KeyedIterable`.
   *
   * @return - The first value in the current `KeyedIterable`, or `null` if the
   *           current `KeyedIterable` is empty.
   */
  public function firstValue()[]: ?Tv;
  /**
   * Returns the first key in the current `KeyedIterable`.
   *
   * @return - The first key in the current `KeyedIterable`, or `null` if the
   *           current `KeyedIterable` is empty.
   */
  public function firstKey()[]: ?Tk;
  /**
   * Returns the last value in the current `KeyedIterable`.
   *
   * @return - The last value in the current `KeyedIterable`, or `null` if the
   *           current `KeyedIterable` is empty.
   */
  public function lastValue()[]: ?Tv;
  /**
   * Returns the last key in the current `KeyedIterable`.
   *
   * @return - The last key in the current `KeyedIterable`, or `null` if the
   *           current `KeyedIterable` is empty.
   */
  public function lastKey()[]: ?Tk;
}

interface Countable extends \Countable {
  <<__SupportDynamicType>>
  public function count()[]: int;
}
