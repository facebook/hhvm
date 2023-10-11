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
 * This file provides type information for some of PHP's predefined interfaces
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

namespace HH {

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
  <<__Sealed(
    Container::class,
    \Imagick::class,
    Iterator::class,
    \IteratorAggregate::class,
    KeyedTraversable::class,
    \ResourceBundle::class,
    \SplHeap::class,
  )>>
  interface Traversable<<<__NoAutoBound>> +Tv> {

  }

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
  <<__Sealed(
    \ArrayIterator::class,
    \AsyncMysqlRowBlock::class,
    \DOMNamedNodeMap::class,
    \ImagickPixelIterator::class,
    \IntlBreakIterator::class,
    KeyedContainer::class,
    KeyedIterable::class,
    KeyedIterator::class,
    \MysqlRow::class,
  )>>
  interface KeyedTraversable<+Tk, <<__NoAutoBound>> +Tv> extends Traversable<Tv> {}

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
  <<__Sealed(KeyedContainer::class)>>
  interface Container<<<__NoAutoBound>> +Tv> extends Traversable<Tv> {

  }

  /**
   * `KeyedContainer` allows you to do `(foreach $value as $k => $v, $value[$key])` and is an
   * interface used by both Hack arrays (vec/dict/keyset) and Hack collections (Vector/Map/Set).
   *
   * If you need to iterate over an array / collection with keys, use KeyedContainer; otherwise, use Container.
   * Without iterating, you can access keys directly: `$keyed_container[$key]`.
   *
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   * @guide /hack/collections/read-write
   */
  <<__Sealed(
    \ConstVector::class,
    \ConstMap::class,
    \ConstSet::class,
    AnyArray::class,
  )>>
  interface KeyedContainer<<<__NoAutoBound>> +Tk as arraykey, <<__NoAutoBound>> +Tv>
    extends Container<Tv>, KeyedTraversable<Tk, Tv> {}

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
     * Returns the current key that the iterator points to.
     */
    public function key(): mixed;
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
   *   foreach ($async_iter await as $value) { ... }
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
  interface KeyedIterator<+Tk, +Tv>
    extends KeyedTraversable<Tk, Tv>, Iterator<Tv> {
    /**
     * Return the current key at the current iterator position.
     *
     * @return - The current key of type `Tk`.
     */
    public function key(): Tk;
  }

} // namespace HH

namespace {

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

} // namespace

namespace HH {

  /**
   * Represents any entity that can be iterated over using something like
   * `foreach`. The entity does not necessarily have to have a key, just values.
   *
   * `Iterable` does not include `array`s.
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   */
  interface Iterable<+Tv> extends \IteratorAggregate<Tv> {
    /**
     * Returns an iterator that points to beginning of the current `Iterable`.
     *
     * @return - An `Iterator` that allows you to traverse the current `Iterable`.
     */
    public function getIterator(): Iterator<Tv>;
    /**
     * Returns an `array` with the values from the current `Iterable`.
     *
     * The keys in the current `Iterable` are discarded and replaced with integer
     * indices, starting with 0.
     *
     * @return - an `array` containing the values from the current `Iterable`.
     */
    public function toValuesArray(): varray<Tv>;
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
    /**
     * Returns an immutable set (`ImmSet`) converted from the current `Iterable`.
     *
     * Any keys in the current `Iterable` are discarded.
     *
     * @return - an `ImmSet` converted from the current `Iterable`.
     */
    public function toImmSet(): ImmSet<Tv> where Tv as arraykey;
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
    public function filter((function(Tv)[_]: bool) $fn)[ctx $fn]: Iterable<Tv>;
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
    public function zip<Tu>(
      Traversable<Tu> $traversable,
    ): Iterable<Pair<Tv, Tu>>;
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
    public function takeWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: Iterable<Tv>;
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
    public function skipWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: Iterable<Tv>;
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
      Traversable<Tu> $traversable,
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
  interface KeyedIterable<Tk, +Tv>
    extends KeyedTraversable<Tk, Tv>, Iterable<Tv> {
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
    public function toKeysArray(): varray<Tk>;
    /**
     * Returns an immutable map (`ImmMap`) based on the keys and values of the
     * current `KeyedIterable`.
     *
     * @return - an `ImmMap` that has the keys and associated values of the
     *           current `KeyedIterable`.
     */
    public function toImmMap(): ImmMap<Tk, Tv> where Tk as arraykey;
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
    )[ctx $fn]: KeyedIterable<Tk, Tv>;
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
    )[ctx $fn]: KeyedIterable<Tk, Tv>;
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
      Traversable<Tu> $traversable,
    ): KeyedIterable<Tk, Pair<Tv, Tu>>;
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
    public function takeWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: KeyedIterable<Tk, Tv>;
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
    public function skipWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: KeyedIterable<Tk, Tv>;
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
      Traversable<Tu> $traversable,
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

} // namespace HH

namespace {

  interface Serializable {
    public function serialize(): string;
    public function unserialize(string $serialized): void;
  }

  interface Countable {
    public function count(): int;
  }

  interface RecursiveIterator<+Tv> extends Iterator<Tv> {
    // see RecursiveDirectoryIterator
    public function getChildren(): ?HH\FIXME\POISON_MARKER<this>;
    public function hasChildren(): bool;
  }

  interface SeekableIterator<+Tv> extends Iterator<Tv> {
    public function seek(int $position): void;
  }

  interface OuterIterator<+Tv> extends Iterator<Tv> {
    public function getInnerIterator(): Iterator<Tv>;
  }

  interface ArrayAccess<Tk, Tv> {
    public function offsetExists(Tk $key): bool;
    public function offsetGet(Tk $key): Tv;
    public function offsetSet(Tk $key, Tv $val): void;
    public function offsetUnset(Tk $key): void;
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
  <<__Sealed(StringishObject::class)>>
  interface Stringish extends XHPChild {}

  /**
   * StringishObject represents values of Stringish that are specifically objects
   */
  interface StringishObject extends Stringish {
    public function __toString(): string;
  }

  interface IZonedStringishObject extends StringishObject {
    public function __toString()[zoned]: string;
  }

  interface ILeakSafeStringishObject extends IZonedStringishObject {
    public function __toString()[leak_safe]: string;
  }

  interface IPureStringishObject extends ILeakSafeStringishObject {
    public function __toString()[]: string;
  }

} // namespace

namespace HH {

  /**
   * Classes that implement `IMemoizeParam` may be used as parameters on
   * `<<__Memoize>>` functions.
   *
   * @guide /hack/attributes/introduction
   * @guide /hack/attributes/special
   */
  interface IMemoizeParam {
    abstract const ctx CMemoParam = [defaults];
    /**
    * Serialize this object to a string that can be used as a
    * dictionary key to differentiate instances of this class.
    */
    public function getInstanceKey(): string;
  }

  /*
   * Unsafe marker interface for an IMemoizeParam used to override
   * typechecker check that IMemoizeParam objects are only used as parameters
   * on functions that have access to globals.
   * Singleton classes are allowed to be passed into pure memoized functions because there can't
   * be a data leak associated with them.
   * TODO: enforce SingletonMemoizeParam in some way in runtime/typechecker
  */
  interface UNSAFESingletonMemoizeParam extends IMemoizeParam {
    public function getInstanceKey(): string;
  }

} // namespace HH

namespace {

  /**
   * Objects that implement `IDisposable` may be used in `using` statements.
   */
  interface IDisposable {
    /**
     * This method is invoked exactly once at the end of the scope of the
     * `using` statement, unless the program terminates with a fatal error.
     */
    public function __dispose(): void;
  }

  /**
   * Objects that implement `IAsyncDisposable` may be used in `using`
   * statements with `await`.
   */
  interface IAsyncDisposable {
    /**
     * This method is invoked exactly once at the end of the scope of the
     * `using` statement, unless the program terminates with a fatal error.
     */
    public function __disposeAsync(): Awaitable<void>;
  }

} // namespace
