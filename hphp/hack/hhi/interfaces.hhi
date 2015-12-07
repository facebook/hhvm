<?hh // partial
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
 * This file provides type information for some of PHP's predefined interfaces
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

/**
 * Represents an entity that can be iterated over using `foreach`, without
 * requiring a key.
 *
 * The iteration variable will have a type of `T`.
 *
 * In addition to Hack collections, PHP `array`s and anything that implement
 * `Iterator` are `Traversable`.
 *
 * In general, if you are implementing your own Hack class, you will want to
 * implement `Iterable` instead of `Traversable` since `Traversable` is more
 * of a bridge for PHP `array`s to work well with Hack collections.
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/interfaces
 */
interface Traversable<+Tv> {}

/**
 * Represents an entity that can be iterated over using `foreach`, allowing
 * a key.
 *
 * The iteration variables will have a type of `Tk` for the key and `Tv` for the
 * value.
 *
 * In addition to Hack collections, PHP `array`s and anything that implement
 * `KeyedIterator` are `KeyedTraversable`.
 *
 * In general, if you are implementing your own Hack class, you will want to
 * implement `KeyedIterable` instead of `KeyedTraversable` since
 * `KeyedTraversable` is more of a bridge for PHP `array`s to work well with
 * Hack collections.
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/interfaces
 */
interface KeyedTraversable<+Tk, +Tv> extends Traversable<Tv> {}

/**
 * Represents an entity that can be iterated over using `foreach`, without
 * requiring a key, except it does not include objects that implement
 * `Iterator`.
 *
 * The iteration variable will have a type of `T`.
 *
 * In addition to Hack collections, PHP `array`s are `Container`s.
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/interfaces
 */
interface Container<+Tv> extends Traversable<Tv> {}

/**
 * Represents an entity that can be iterated over using `foreach`, allowing
 * a key, except it does not include objects that implement `KeyedIterator` nor
 * `Set` and `ImmSet`.
 *
 * The iteration variables will have a type of `Tk` for the key and `Tv` for the
 * value.
 *
 * In addition to Hack collections, PHP `array`s are `KeyedContainer`s.
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/interfaces
 */
interface KeyedContainer<+Tk, +Tv> extends Container<Tv>, KeyedTraversable<Tk, Tv> {}

/**
 * Represents an entity that can be indexed using square-bracket syntax.
 *
 * Square bracket syntax is:
 *
 * ```
 * $indexish[$key]
 * ```
 *
 * At this point, this includes entities with keys of `int` and `string`.
 *
 * In addition to Hack collections, PHP `array`s are `Indexish`.
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/interfaces
 * @guide /hack/collections/read-write
 */
interface Indexish<+Tk, +Tv> extends KeyedContainer<Tk, Tv> {}

/**
 * For those entities that are `Traversable`, the `Iterator` interfaces provides
 * the methods of iteration.
 *
 * If a class implements `Iterator`, then it provides the infrastructure to be
 * iterated over using a `foreach` loop.
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/interfaces
 *
 * @link http://php.net/manual/en/class.iterator.php
 */
interface Iterator<+Tv> extends Traversable<Tv> {
  /**
   * Return the current value at the current iterator position.
   *
   * @return - The current value of type `Tv`.
   */
  public function current(): Tv;
  /**
   * Move the iterator position to the next element.
   *
   */
  public function next(): void;
  /**
   * Rewind the iterator position to its beginning.
   *
   * This rewinds back to the first element of the `Iterator`.
   */
  public function rewind(): void;
  /**
   * Checks to see if the current iterator position is valid.
   *
   * This method is called after `rewind()` and `next()` to check if the
   * current iterator position is valid.
   *
   * @return - `true` if the position is valid; `false` otherwise.
   */
  public function valid(): bool;
}

/**
 * Allows for the iteration over the values provided by an `async` function.
 *
 * If an `async` function returns an `AsyncIterator<T>`, then you can iterate
 * over the `T` values returned from that function.
 *
 * ```
 * async function countdown(int $start): AsyncIterator<int> { ... }
 *
 * async function use_countdown(): Awaitable<void> {
 *   $async_iter = countdown(100);
 *   foreach ($async_gen await as $value) { ... }
 * }
 * ```
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/guidelines
 */
interface AsyncIterator<+Tv> {
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
  public function next(): Awaitable<?(mixed, Tv)>;
}

/**
 * Allows for the iteration over the keys and values provided by an `async`
 * function.
 *
 * If an `async` function returns an `AsyncIterator<Tk, Tv>`, then you can
 * iterate over the `Tk` and `Tv` values returned from that function.
 *
 * ```
 * async function countdown(int $start): AsyncIterator<int, string> { ... }
 *
 * async function use_countdown(): Awaitable<void> {
 *   $async_iter = countdown(100);
 *   foreach ($async_gen await as $num => $str) { ... }
 * }
 * ```
 *
 * @guide /hack/async/introduction
 * @guide /hack/async/guidelines
 */
interface AsyncKeyedIterator<+Tk, +Tv> extends AsyncIterator<Tv> {
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
  public function next(): Awaitable<?(Tk, Tv)>;
}

/**
 * For those entities that are `KeyedTraversable`, the `KeyedIterator`
 * interfaces provides the methods of iteration, included being able to get
 * the key.
 *
 * If a class implements `KeyedIterator`, then it provides the infrastructure
 * to be iterated over using a `foreach` loop.
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/interfaces
 */
interface KeyedIterator<+Tk, +Tv> extends KeyedTraversable<Tk,Tv>, Iterator<Tv> {
  /**
   * Return the current key at the current iterator position.
   *
   * @return - The current key of type `Tk`.
   */
  public function key(): Tk;
}

/**
 * Represents objects that can produce an `Iterator` object to iterate over
 * their contents using `foreach`.
 *
 * Normally, this interface won't be used in type annotations; rather `Iterable`
 * or `Traversable` will be the better interface.
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/interfaces
 *
 * @link http://php.net/manual/en/class.iteratoraggregate.php
 */
interface IteratorAggregate<+Tv> extends Traversable<Tv> {
  /**
   * Returns an iterator to be used to iterate over the object's elements.
   *
   * @return - An `Iterator` for iteration.
   */
  public function getIterator(): Iterator<Tv>;
}

/**
 * Represents any entity that can be iterated over using something like
 * `foreach`. The entity does not necessarily have to have a key, just values.
 *
 * `Iterable` does not include `array`s.
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/interfaces
 */
interface Iterable<+Tv> extends IteratorAggregate<Tv> {
  /**
   * Returns an iterator that points to beginning of the current `Iterable`.
   *
   * @return - An `Iterator` that allows you to traverse the current `Iterable`.
   */
  public function getIterator(): Iterator<Tv>;
  /**
   * Returns an `array` converted from the current `Iterable`.
   *
   * @return - an array converted from the current `Iterable`.
   */
  public function toArray(): array;
  /**
   * Returns an `array` with the values from the current `Iterable`.
   *
   * The keys in the current `Iterable` are discarded and replaced with integer
   * indices, starting with 0.
   *
   * @return - an `array` containing the values from the current `Iterable`.
   */
  public function toValuesArray(): array;
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  /**
   * Returns a `Vector` converted from the current `Iterable`.
   *
   * Any keys in the current `Iterable` are discarded and replaced with integer
   * indices, starting with 0.
   *
   * @return - a `Vector` converted from the current `Iterable`.
   */
  public function toVector(): Vector<Tv>;
  /**
   * Returns an immutable vector (`ImmVector`) converted from the current
   * `Iterable`.
   *
   * Any keys in the current `Iterable` are discarded and replaced with integer
   * indices, starting with 0.
   *
   * @return - an `ImmVector` converted from the current `Iterable`.
   */
  public function toImmVector(): ImmVector<Tv>;
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe.
   * See #6853603. */
  /**
   * Returns a `Set` converted from the current `Iterable`.
   *
   * Any keys in the current `Iterable` are discarded.
   *
   * @return - a `Set` converted from the current `Iterable`.
   */
  public function toSet(): Set<Tv>;
  /**
   * Returns an immutable set (`ImmSet`) converted from the current `Iterable`.
   *
   * Any keys in the current `Iterable` are discarded.
   *
   * @return - an `ImmSet` converted from the current `Iterable`.
   */
  public function toImmSet(): ImmSet<Tv>;
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
  public function lazy(): Iterable<Tv>;
  /**
   * Returns an `Iterable` containing the current `Iterable`'s values.
   *
   * Any keys are discarded.
   *
   * @return An `Iterable` with the values of the current `Iterable`.
   */
  public function values(): Iterable<Tv>;
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
  public function map<Tu>((function(Tv): Tu) $fn): Iterable<Tu>;
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
  public function filter((function(Tv): bool) $fn): Iterable<Tv>;
  /**
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
  public function zip<Tu>(Traversable<Tu> $traversable): Iterable<Pair<Tv, Tu>>;
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
  public function take(int $n): Iterable<Tv>;
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
  public function takeWhile((function(Tv): bool) $fn): Iterable<Tv>;
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
  public function skip(int $n): Iterable<Tv>;
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
  public function skipWhile((function(Tv): bool) $fn): Iterable<Tv>;
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
  public function slice(int $start, int $len): Iterable<Tv>;
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
    Traversable<Tu> $traversable
  ): Iterable<Tu>;
  /**
   * Returns the first value in the current `Iterable`.
   *
   * @return - The first value in the current `Iterable`, or `null` if the
   *           current `Iterable` is empty.
   */
  public function firstValue(): ?Tv;
  /**
   * Returns the last value in the current `Iterable`.
   *
   * @return - The last value in the current `Iterable`, or `null` if the
   *           current `Iterable` is empty.
   */
  public function lastValue(): ?Tv;
}

/**
 * Represents any entity that can be iterated over using something like
 * `foreach`. The entity is required to have a key in addition to values.
 *
 * `KeyedIterable` does not include `array`s.
 *
 * @guide /hack/collections/introduction
 * @guide /hack/collections/interfaces
 */
interface KeyedIterable<Tk, +Tv> extends KeyedTraversable<Tk, Tv>, Iterable<Tv> {
  /**
   * Returns an iterator that points to beginning of the current
   * `KeyedIterable`.
   *
   * @return - A `KeyedIterator` that allows you to traverse the current
   *           `KeyedIterable`.
   */
  public function getIterator(): KeyedIterator<Tk, Tv>;
  /**
   * Returns an `array` with the keys from the current `KeyedIterable`.
   *
   * @return - an `array` containing the values from the current
   *           `KeyedIterable`.
   */
  public function toKeysArray(): array;
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  /**
   * Returns a `Map` based on the keys and values of the current
   * `KeyedIterable`.
   *
   * @return - a `Map` that has the keys and associated values of the current
   *           `KeyedIterable`.
   */
  public function toMap(): Map<Tk, Tv>;
  /**
   * Returns an immutable map (`ImmMap`) based on the keys and values of the
   * current `KeyedIterable`.
   *
   * @return - an `ImmMap` that has the keys and associated values of the
   *           current `KeyedIterable`.
   */
  public function toImmMap(): ImmMap<Tk, Tv>;
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
  public function lazy(): KeyedIterable<Tk, Tv>;
  /**
   * Returns an `Iterable` containing the current `KeyedIterable`'s values.
   *
   * Any keys are discarded.
   *
   * @return An `Iterable` with the values of the current `KeyedIterable`.
   */
  public function values(): Iterable<Tv>;
  /**
   * Returns an `Iterable` containing the current `KeyedIterable`'s keys.
   *
   * Any values are discarded.
   *
   * @return An `Iterable` with the keys of the current `KeyedIterable`.
   */
  public function keys(): Iterable<Tk>;
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
  public function map<Tu>((function(Tv): Tu) $fn): KeyedIterable<Tk, Tu>;
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
  public function mapWithKey<Tu>((function(Tk, Tv): Tu) $fn):
    KeyedIterable<Tk, Tu>;
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
  public function filter((function(Tv): bool) $fn): KeyedIterable<Tk, Tv>;
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
  public function filterWithKey((function(Tk, Tv): bool) $fn):
    KeyedIterable<Tk, Tv>;
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
  public function zip<Tu>(Traversable<Tu> $traversable):
    KeyedIterable<Tk, Pair<Tv, Tu>>;
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
  public function take(int $n): KeyedIterable<Tk, Tv>;
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
  public function takeWhile((function(Tv): bool) $fn): KeyedIterable<Tk, Tv>;
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
  public function skip(int $n): KeyedIterable<Tk, Tv>;
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
  public function skipWhile((function(Tv): bool) $fn): KeyedIterable<Tk, Tv>;
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
  public function slice(int $start, int $len): KeyedIterable<Tk, Tv>;
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
    Traversable<Tu> $traversable
  ): Iterable<Tu>;
  /**
   * Returns the first value in the current `KeyedIterable`.
   *
   * @return - The first value in the current `KeyedIterable`, or `null` if the
   *           current `KeyedIterable` is empty.
   */
  public function firstValue(): ?Tv;
  /**
   * Returns the first key in the current `KeyedIterable`.
   *
   * @return - The first key in the current `KeyedIterable`, or `null` if the
   *           current `KeyedIterable` is empty.
   */
  public function firstKey(): ?Tk;
  /**
   * Returns the last value in the current `KeyedIterable`.
   *
   * @return - The last value in the current `KeyedIterable`, or `null` if the
   *           current `KeyedIterable` is empty.
   */
  public function lastValue(): ?Tv;
  /**
   * Returns the last key in the current `KeyedIterable`.
   *
   * @return - The last key in the current `KeyedIterable`, or `null` if the
   *           current `KeyedIterable` is empty.
   */
  public function lastKey(): ?Tk;
}

interface Serializable {
  public function serialize(): string;
  public function unserialize($serialized): void;
}

interface Countable {
  public function count(): int;
}

interface RecursiveIterator<Tv> extends Iterator<Tv> {
  public function getChildren(): this;
  public function hasChildren(): bool;
}

interface SeekableIterator<Tv> extends Iterator<Tv> {
  public function seek(int $position): void;
}

interface OuterIterator<Tv> extends Iterator<Tv> {
  public function getInnerIterator(): Iterator<Tv>;
}

interface ArrayAccess<Tk, Tv> {
  public function offsetExists(Tk $key): bool;
  public function offsetGet(Tk $key): Tv;
  public function offsetSet(Tk $key, Tv $val): void;
  public function offsetUnset(Tk $key): void;
}

interface Awaitable<+T> {
  public function getWaitHandle(): WaitHandle<T>;
}

/**
 * @see http://www.php.net/manual/en/jsonserializable.jsonserialize.php
 */
interface JsonSerializable {
  /**
   * Return data which can be serialized with json_encode.
   */
  public function jsonSerialize(): mixed;
}

/**
 * XHPChild is the base type of values that can be children of XHP elements.
 * Most primitive types implement XHPChild: string, int, float, and array.
 *
 * Classes that implement XHPChild must do so by implementing the XHPChildClass
 * subinterface.
 */
interface XHPChild {}

/**
 * Stringish is a type that matches strings as well as string-convertible
 * objects: that is, objects that provide the __toString method
 */
interface Stringish {
  public function __toString(): string;
}

/**
  * Classes that implement IMemoizeParam may be used as parameters on
  * <<__Memoize>> functions
  *
 * @guide /hack/attributes/introduction
 * @guide /hack/attributes/special
  */
interface IMemoizeParam {
   /**
   * Serialize this object to a string that can be used as a
   * dictionary key to differentiate instances of this class.
   */
  public function getInstanceKey(): string;
}
