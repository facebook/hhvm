<?hh
<<file: __EnableUnstableFeatures('readonly')>>

namespace {

  /**
   * **Prefer Hack arrays (`vec`, `dict`, `shape`, `keyset`) over collection types (`Vector`, `Map`, etc.)**.
   *
   * The base interface implemented for a collection type so that base information
   * such as count and its items are available. Every concrete class indirectly
   * implements this interface.
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   *
   */
  <<__Sealed(
    Collection::class,
    ConstMap::class,
    ConstSet::class,
    ConstVector::class,
  )>>
  interface ConstCollection<+Te> extends Countable, IPureStringishObject {

    /**
     * Is the collection empty?
     *
     * @return - Returns `true` if the collection is empty; `false`
     *           otherswise.
     */
    public readonly function isEmpty()[]: bool;
    /**
     * Get the number of items in the collection. Cannot be negative.
     *
     * @return - Returns the number of items in the collection
     */
    public readonly function count()[]: int;
    /**
     * Get access to the items in the collection. Can be empty.
     *
     * @return - Returns an `Iterable` with access to all of the items in
     *   the collection.
     */
    public function items()[]: HH\Iterable<Te>;
    public function toVArray(
    )[]: varray<mixed>;
    public function toDArray(
    )[]: darray<arraykey, mixed>;
  }

  /**
   * The interface implemented for a mutable collection type so that values can
   * be added to it.
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   *
   */
  <<__Sealed(Collection::class)>>
  interface OutputCollection<-Te> {
    /**
     * Add a value to the collection and return the collection itself.
     *
     * It returns the current collection, meaning changes made to the current
     * collection will be reflected in the returned collection.
     *
     * @param $e - The value to add.
     *
     * @return - The updated collection itself.
     */
    public function add(Te $e)[write_props]: this;
    /**
     * For every element in the provided `Traversable`, append a value into the
     * current collection.
     *
     * It returns the current collection, meaning changes made to the current
     * collection will be reflected in the returned collection.
     *
     * @param $traversable - The `Traversable` with the new values to set. If
     *                       `null` is provided, no changes are made.
     *
     * @return - Returns itself.
     */
    public function addAll(?Traversable<Te> $traversable)[write_props]: this;
  }

} // namespace

namespace HH {

  /**
   * `Collection` is the primary collection interface for mutable collections.
   *
   * Assuming you want the ability to clear out your collection, you would
   * implement this (or a child of this) interface. Otherwise, you can implement
   * `OutputCollection` only. If your collection to be immutable, implement
   * `ConstCollection` only instead.
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   */
  <<__Sealed(\MutableMap::class, \MutableSet::class, \MutableVector::class)>>
  interface Collection<Te> extends \ConstCollection<Te>, \OutputCollection<Te> {
    /**
     * Removes all items from the collection.
     */
    public function clear()[write_props]: this;
  }

} // namespace HH

namespace {

  /**
   * The interface for all `Set`s to enable access its values.
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   */
  <<__Sealed(ConstMapAccess::class, SetAccess::class, ConstSet::class)>>
  interface ConstSetAccess<+Tm as arraykey> {
    /**
     * Checks whether a value is in the current `Set`.
     *
     * @return - `true` if the value is in the current `Set`; `false` otherwise.
     */
    public readonly function contains(arraykey $m)[]: bool;
  }

  /**
   *
   * The interface for mutable `Set`s to enable removal of its values.
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   */
  <<__Sealed(MapAccess::class, MutableSet::class)>>
  interface SetAccess<Tm as arraykey> extends ConstSetAccess<Tm> {
    /**
     * Removes the provided value from the current `Set`.
     *
     * If the value is not in the current `Set`, the `Set` is unchanged.
     *
     * It the current `Set`, meaning changes  made to the current `Set` will be
     * reflected in the returned `Set`.
     *
     * @param $m - The value to remove.
     *
     * @return - Returns itself.
     */
    public function remove(Tm $m)[write_props]: this;
  }

  /**
   * The interface for all keyed collections to enable access its values.
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   */
  <<__Sealed(ConstMapAccess::class, IndexAccess::class, ConstVector::class)>>
  interface ConstIndexAccess<Tk, +Tv> {
    /**
     * Returns the value at the specified key in the current collection.
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
    public function at(Tk $k)[]: Tv;
    /**
     * Returns the value at the specified key in the current collection.
     *
     * If the key is not present, `null` is returned. If you would rather have an
     * exception thrown when a key is not present, then use `at()`.
     *
     * @param $k - the key from which to retrieve the value.
     *
     * @return - The value at the specified key; or `null` if the key does not
     *           exist.
     */
    public function get(Tk $k)[]: ?Tv;
    /**
     * Determines if the specified key is in the current collection.
     *
     * @return - `true` if the specified key is present in the current collection;
     *           `false` otherwise.
     *
     * @guide /hack/generics/constraints
     */
    public readonly function containsKey(mixed $k)[]: bool;
  }

  /**
   * The interface for mutable, keyed collections to enable setting and removing
   * keys.
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   */
  <<__Sealed(MapAccess::class, MutableVector::class)>>
  interface IndexAccess<Tk, Tv> extends ConstIndexAccess<Tk, Tv> {
    /**
     * Stores a value into the current collection with the specified key,
     * overwriting the previous value associated with the key.
     *
     * If the key is not present, an exception is thrown. If you want to add
     * a value even if a key is not present, use `add()`.
     *
     * `$coll->set($k,$v)` is semantically equivalent to `$coll[$k] = $v`
     * (except that `set()` returns the current collection).
     *
     * It returns the current collection, meaning changes made to the current
     * collection will be reflected in the returned collection.
     *
     * @param $k - The key to which we will set the value.
     * @param $v - The value to set.
     *
     * @return - Returns itself.
     */
    public function set(Tk $k, Tv $v)[write_props]: this;
    /**
     * For every element in the provided `Traversable`, stores a value into the
     * current collection associated with each key, overwriting the previous value
     * associated with the key.
     *
     * If a key is not present the current Vector that is present in the
     * `Traversable`, an exception is thrown. If you want to add a value even if a
     * key is not present, use `addAll()`.
     *
     * It the current collection, meaning changes made to the current collection
     * will be reflected in the returned collection.
     *
     * @param $traversable - The `Traversable` with the new values to set. If
     *                       `null` is provided, no changes are made.
     *
     * @return - Returns itself.
     */
    public function setAll(
      ?KeyedTraversable<Tk, Tv> $traversable,
    )[write_props]: this;
    /**
     * Removes the specified key (and associated value) from the current
     * collection.
     *
     * If the key is not in the current collection, the current collection is
     * unchanged.
     *
     * It the current collection, meaning changes made to the current collection
     * will be reflected in the returned collection.
     *
     * @param $k - The key to remove.
     *
     * @return - Returns itself.
     */
    public function removeKey(Tk $k)[write_props]: this;
  }

  /**
   * The interface for enabling access to the `Map`s values.
   *
   * This interface provides no new methods as all current access for `Map`s are
   * defined in its parent interfaces. But you could theoretically use this
   * interface for parameter and return type annotations.
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   */
  <<__Sealed(ConstMap::class, MapAccess::class)>>
  interface ConstMapAccess<Tk as arraykey, +Tv>
    extends ConstSetAccess<Tk>, ConstIndexAccess<Tk, Tv> {
  }

  /**
   * The interface for setting and removing `Map` keys (and associated values).
   *
   * This interface provides no new methods as all current access for `Map`s are
   * defined in its parent interfaces. But you could theoretically use this
   * interface for parameter and return type annotations.
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   */
  <<__Sealed(MutableMap::class)>>
  interface MapAccess<Tk as arraykey, Tv>
    extends ConstMapAccess<Tk, Tv>, SetAccess<Tk>, IndexAccess<Tk, Tv> {
  }

  /**
   * Represents a read-only (immutable) sequence of values, indexed by integers
   * (i.e., a vector).
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   */
  <<__Sealed(ImmVector::class, MutableVector::class, Pair::class)>>
  interface ConstVector<+Tv>
    extends
      KeyedContainer<int, Tv>,
      ConstCollection<Tv>,
      ConstIndexAccess<int, Tv>,
      HH\KeyedIterable<int, Tv> {
    /**
     * Returns a `ConstVector` containing the values of the current
     * `ConstVector`. Essentially a copy of the current `ConstVector`.
     *
     * This method is interchangeable with `toVector()`.
     *
     * @return - a `ConstVector` containing the values of the current
     *           `ConstVector`.
     */
    public function values()[]: ConstVector<Tv>;
    /**
     * Returns a `ConstVector` containing the keys of the current `ConstVector`.
     *
     * @return - a `ConstVector` containing the integer keys of the current
     *           `ConstVector`.
     */
    public function keys()[]: ConstVector<int>;
    /**
     * Returns a `ConstVector` containing the values after an operation has been
     * applied to each value in the current `ConstVector`.
     *
     * Every value in the current `ConstVector` is affected by a call to `map()`,
     * unlike `filter()` where only values that meet a certain criteria are
     * affected.
     *
     * @param $fn - The callback containing the operation to apply to the
     *              `ConstVector` values.
     *
     * @return - a `ConstVector` containing the values after a user-specified
     *           operation is applied.
     *
     * @guide /hack/collections/examples
     */
    public function map<Tu>(
      (function(Tv)[_]: Tu) $fn,
    )[ctx $fn]: ConstVector<Tu>;
    /**
     * Returns a `ConstVector` containing the values after an operation has been
     * applied to each key and value in the current `ConstVector`.
     *
     * Every key and value in the current `ConstVector` is affected by a call to
     * `mapWithKey()`, unlike `filterWithKey()` where only values that meet a
     * certain criteria are affected.
     *
     * @param $fn - The callback containing the operation to apply to the
     *              `ConstVector` keys and values.
     *
     * @return - a `ConstVector` containing the values after a user-specified
     *           operation on the current Vector's keys and values is applied.
     */
    public function mapWithKey<Tu>(
      (function(int, Tv)[_]: Tu) $fn,
    )[ctx $fn]: ConstVector<Tu>;
    /**
     * Returns a `ConstVector` containing the values of the current `ConstVector`
     * that meet a supplied condition.
     *
     * Only values that meet a certain criteria are affected by a call to
     * `filter()`, while all values are affected by a call to `map()`.
     *
     * @param $fn - The $fn containing the condition to apply to the
     *              `ConstVector` values.
     *
     * @return - a `ConstVector` containing the values after a user-specified
     *           condition is applied.
     *
     * @guide /hack/collections/examples
     */
    public function filter(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: ConstVector<Tv>;
    /**
     * Returns a `ConstVector` containing the values of the current `ConstVector`
     * that meet a supplied condition applied to its keys and values.
     *
     * Only keys and values that meet a certain criteria are affected by a call to
     * `filterWithKey()`, while all values are affected by a call to
     * `mapWithKey()`.
     *
     * @param $fn - The callback containing the condition to apply to the
     *              `ConstVector` keys and values.
     *
     * @return - a `ConstVector` containing the values after a user-specified
     *           condition is applied to the keys and values of the current
     *           `ConstVector`.
     */
    public function filterWithKey(
      (function(int, Tv)[_]: bool) $fn,
    )[ctx $fn]: ConstVector<Tv>;
    /**
     * Returns a `ConstVector` where each element is a `Pair` that combines the
     * element of the current `ConstVector` and the provided `Traversable`.
     *
     * If the number of elements of the `ConstVector` are not equal to the
     * number of elements in the `Traversable`, then only the combined elements
     * up to and including the final element of the one with the least number of
     * elements is included.
     *
     * @param $traversable - The `Traversable` to use to combine with the
     *                       elements of this `ConstVector`.
     *
     * @return - The `ConstVector` that combines the values of the current
     *           `ConstVector` with the provided `Traversable`.
     */
    public function zip<Tu>(
      Traversable<Tu> $traversable,
    )[]: ConstVector<Pair<Tv, Tu>>;
    /**
     * Returns a `ConstVector` containing the first `n` values of the current
     * `ConstVector`.
     *
     * The returned `ConstVector` will always be a proper subset of the current
     * `ConstVector`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     * @param $n - The last element that will be included in the returned
     *             `ConstVector`.
     *
     * @return - A `ConstVector` that is a proper subset of the current
     *           `ConstVector` up to `n` elements.
     */
    public function take(int $n)[]: ConstVector<Tv>;
    /**
     * Returns a `ConstVector` containing the values of the current `ConstVector`
     * up to but not including the first value that produces `false` when passed
     * to the specified callback.
     *
     * The returned `ConstVector` will always be a proper subset of the current
     * `ConstVector`.
     *
     * @param $fn - The callback that is used to determine the stopping
     *              condition.
     *
     * @return - A `ConstVector` that is a proper subset of the current
     *           `ConstVector` up until the callback returns `false`.
     */
    public function takeWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: ConstVector<Tv>;
    /**
     * Returns a `ConstVector` containing the values after the `n`-th element of
     * the current `ConstVector`.
     *
     * The returned `ConstVector` will always be a proper subset of the current
     * `ConstVector`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     * @param $n - The last element to be skipped; the $n+1 element will be the
     *             first one in the returned `ConstVector`.
     *
     * @return - A `ConstVector` that is a proper subset of the current
     *           `ConstVector` containing values after the specified `n`-th
     *           element.
     */
    public function skip(int $n)[]: ConstVector<Tv>;
    /**
     * Returns a `ConstVector` containing the values of the current `ConstVector`
     * starting after and including the first value that produces `true` when
     * passed to the specified callback.
     *
     * The returned `ConstVector` will always be a proper subset of the current
     * `ConstVector`.
     *
     * @param $fn - The callback used to determine the starting element for the
     *              returned `ConstVector`.
     *
     * @return - A `ConstVector` that is a proper subset of the current
     *           `ConstVector` starting after the callback returns `true`.
     */
    public function skipWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: ConstVector<Tv>;
    /**
     * Returns a subset of the current `ConstVector` starting from a given key up
     * to, but not including, the element at the provided length from the starting
     * key.
     *
     * `$start` is 0-based. $len is 1-based. So `slice(0, 2)` would return the
     * elements at key 0 and 1.
     *
     * The returned `ConstVector` will always be a proper subset of this
     * `ConstVector`.
     *
     * @param $start - The starting key of this Vector to begin the returned
     *                 `ConstVector`.
     * @param $len - The length of the returned `ConstVector`.
     *
     * @return - A `ConstVector` that is a proper subset of the current
     *           `ConstVector` starting at `$start` up to but not including the
     *           element `$start + $len`.
     */
    public function slice(int $start, int $len)[]: ConstVector<Tv>;
    /**
     * Returns a `ConstVector` that is the concatenation of the values of the
     * current `ConstVector` and the values of the provided `Traversable`.
     *
     * The values of the provided `Traversable` is concatenated to the end of the
     * current `ConstVector` to produce the returned `ConstVector`.
     *
     * @param $traversable - The `Traversable` to concatenate to the current
     *                       `ConstVector`.
     *
     * @return - The concatenated `ConstVector`.
     *
     * @guide /hack/generics/constraints
     */
    public function concat<Tu super Tv>(
      Traversable<Tu> $traversable,
    )[]: ConstVector<Tu>;
    /**
     * Returns the first value in the current `ConstVector`.
     *
     * @return - The first value in the current `ConstVector`, or `null` if the
     *           current `ConstVector` is empty.
     */
    public function firstValue()[]: ?Tv;
    /**
     * Returns the first key in the current `ConstVector`.
     *
     * @return - The first key in the current `ConstVector`, or `null` if the
     *           current `ConstVector` is empty.
     */
    public readonly function firstKey()[]: ?int;
    /**
     * Returns the last value in the current `ConstVector`.
     *
     * @return - The last value in the current `ConstVector`, or `null` if the
     *           current `ConstVector` is empty.
     */
    public function lastValue()[]: ?Tv;
    /**
     * Returns the last key in the current `ConstVector`.
     *
     * @return - The last key in the current `ConstVector`, or `null` if the
     *           current `ConstVector` is empty.
     */
    public readonly function lastKey()[]: ?int;
    /**
     * Returns the index of the first element that matches the search value.
     *
     * If no element matches the search value, this function returns -1.
     *
     * @param $search_value - The value that will be search for in the current
     *                        `ConstVector`.
     *
     * @return - The key (index) where that value is found; -1 if it is not found.
     *
     * @guide /hack/generics/constraints
     */
    public readonly function linearSearch(mixed $search_value)[]: int;
    public function toVArray()[]: varray<Tv>;
    public function toDArray()[]: darray<int, Tv>;
  }

  /**
   * Represents a write-enabled (mutable) sequence of values, indexed by integers
   * (i.e., a vector).
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   */
  <<__Sealed(Vector::class)>>
  interface MutableVector<Tv>
    extends ConstVector<Tv>, Collection<Tv>, IndexAccess<int, Tv> {
    /**
     * Returns a `MutableVector` containing the values of the current
     * `MutableVector`. Essentially a copy of the current `MutableVector`.
     *
     * This method is interchangeable with `toVector()`.
     *
     * @return - a `MutableVector` containing the values of the current
     *           `MutableVector`.
     */
    public function values()[]: MutableVector<Tv>;
    /**
     * Returns a `MutableVector` containing the keys of the current
     * `MutableVector`.
     *
     * @return - a `MutableVector` containing the integer keys of the current
     *           `MutableVector`.
     */
    public readonly function keys()[]: MutableVector<int>;
    /**
     * Returns a `MutableVector` containing the values after an operation has been
     * applied to each value in the current `MutableVector`.
     *
     * Every value in the current `MutableVector` is affected by a call to
     * `map()`, unlike `filter()` where only values that meet a certain criteria
     * are affected.
     *
     * @param $fn - The callback containing the operation to apply to the
     *              `MutableVector` values.
     *
     * @return - a `MutableVector` containing the values after a user-specified
     *           operation is applied.
     *
     * @guide /hack/collections/examples
     */
    public function map<Tu>(
      (function(Tv)[_]: Tu) $fn,
    )[ctx $fn]: MutableVector<Tu>;
    /**
     * Returns a `MutableVector` containing the values after an operation has been
     * applied to each key and value in the current `MutableVector`.
     *
     * Every key and value in the current `MutableVector` is affected by a call to
     * `mapWithKey()`, unlike `filterWithKey()` where only values that meet a
     * certain criteria are affected.
     *
     * @param $fn - The callback containing the operation to apply to the
     *              `MutableVector` keys and values.
     *
     * @return - a `MutableVector` containing the values after a user-specified
     *           operation on the current Vector's keys and values is applied.
     */
    public function mapWithKey<Tu>(
      (function(int, Tv)[_]: Tu) $fn,
    )[ctx $fn]: MutableVector<Tu>;
    /**
     * Returns a `MutableVector` containing the values of the current
     * `MutableVector` that meet a supplied condition.
     *
     * Only values that meet a certain criteria are affected by a call to
     * `filter()`, while all values are affected by a call to `map()`.
     *
     * @param $fn - The $fn containing the condition to apply to the
     *              `MutableVector` values.
     *
     * @return - a `MutableVector` containing the values after a user-specified
     *           condition is applied.
     *
     * @guide /hack/collections/examples
     */
    public function filter(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: MutableVector<Tv>;
    /**
     * Returns a `MutableVector` containing the values of the current
     * `MutableVector` that meet a supplied condition applied to its keys and
     * values.
     *
     * Only keys and values that meet a certain criteria are affected by a call to
     * `filterWithKey()`, while all values are affected by a call to
     * `mapWithKey()`.
     *
     * @param $fn - The callback containing the condition to apply to the
     *              `MutableVector` keys and values.
     *
     * @return - a `MutableVector` containing the values after a user-specified
     *           condition is applied to the keys and values of the current
     *           `MutableVector`.
     *
     */
    public function filterWithKey(
      (function(int, Tv)[_]: bool) $fn,
    )[ctx $fn]: MutableVector<Tv>;
    /**
     * Returns a `MutableVector` where each element is a `Pair` that combines the
     * element of the current `MutableVector` and the provided `Traversable`.
     *
     * If the number of elements of the `MutableVector` are not equal to the
     * number of elements in the `Traversable`, then only the combined elements
     * up to and including the final element of the one with the least number of
     * elements is included.
     *
     * @param $traversable - The `Traversable` to use to combine with the
     *                       elements of this `MutableVector`.
     *
     * @return - The `MutableVector` that combines the values of the current
     *           `MutableVector` with the provided `Traversable`.
     */
    public function zip<Tu>(
      Traversable<Tu> $traversable,
    )[]: MutableVector<Pair<Tv, Tu>>;
    /**
     * Returns a `MutableVector` containing the first `n` values of the current
     * `MutableVector`.
     *
     * The returned `MutableVector` will always be a proper subset of the current
     * `MutableVector`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     * @param $n - The last element that will be included in the returned
     *             `MutableVector`.
     *
     * @return - A `MutableVector` that is a proper subset of the current
     *           `MutableVector` up to `n` elements.
     */
    public function take(int $n)[]: MutableVector<Tv>;
    /**
     * Returns a `MutableVector` containing the values of the current
     * `MutableVector` up to but not including the first value that produces
     * `false` when passed to the specified callback.
     *
     * The returned `MutableVector` will always be a proper subset of the current
     * `MutableVector`.
     *
     * @param $fn - The callback that is used to determine the stopping
     *              condition.
     *
     * @return - A `MutableVector` that is a proper subset of the current
     *           `MutableVector` up until the callback returns `false`.
     */
    public function takeWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: MutableVector<Tv>;
    /**
     * Returns a `MutableVector` containing the values after the `n`-th element of
     * the current `MutableVector`.
     *
     * The returned `MutableVector` will always be a proper subset of the current
     * `MutableVector`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     * @param $n - The last element to be skipped; the $n+1 element will be the
     *             first one in the returned `MutableVector`.
     *
     * @return - A `MutableVector` that is a proper subset of the current
     *           `MutableVector` containing values after the specified `n`-th
     *           element.
     */
    public function skip(int $n)[]: MutableVector<Tv>;
    /**
     * Returns a `MutableVector` containing the values of the current
     * `MutableVector` starting after and including the first value that produces
     * `true` when passed to the specified callback.
     *
     * The returned `MutableVector` will always be a proper subset of the current
     * `MutableVector`.
     *
     * @param $fn - The callback used to determine the starting element for the
     *              returned `MutableVector`.
     *
     * @return - A `MutableVector` that is a proper subset of the current
     *           `MutableVector` starting after the callback returns `true`.
     */
    public function skipWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: MutableVector<Tv>;
    /**
     * Returns a subset of the current `MutableVector` starting from a given key
     * up to, but not including, the element at the provided length from the
     * starting key.
     *
     * `$start` is 0-based. $len is 1-based. So `slice(0, 2)` would return the
     * elements at key 0 and 1.
     *
     * The returned `MutableVector` will always be a proper subset of this
     * `MutableVector`.
     *
     * @param $start - The starting key of this Vector to begin the returned
     *                 `MutableVector`.
     * @param $len - The length of the returned `MutableVector`.
     *
     * @return - A `MutableVector` that is a proper subset of the current
     *           `MutableVector` starting at `$start` up to but not including the
     *           element `$start + $len`.
     */
    public function slice(int $start, int $len)[]: MutableVector<Tv>;
    /**
     * Returns a `MutableVector` that is the concatenation of the values of the
     * current `MutableVector` and the values of the provided `Traversable`.
     *
     * The values of the provided `Traversable` is concatenated to the end of the
     * current `MutableVector` to produce the returned `MutableVector`.
     *
     * @param $traversable - The `Traversable` to concatenate to the current
     *                       `MutableVector`.
     *
     * @return - The concatenated `MutableVector`.
     *
     * @guide /hack/generics/constraints
     */
    public function concat<Tu super Tv>(
      Traversable<Tu> $traversable,
    )[]: MutableVector<Tu>;
    /**
     * Returns the first value in the current `MutableVector`.
     *
     * @return - The first value in the current `MutableVector`, or `null` if the
     *           current `MutableVector` is empty.
     */
    public function firstValue()[]: ?Tv;
    /**
     * Returns the first key in the current `MutableVector`.
     *
     * @return - The first key in the current `MutableVector`, or `null` if the
     *           current `MutableVector` is empty.
     */
    public readonly function firstKey()[]: ?int;
    /**
     * Returns the last value in the current `MutableVector`.
     *
     * @return - The last value in the current `MutableVector`, or `null` if the
     *           current `MutableVector` is empty.
     */
    public function lastValue()[]: ?Tv;
    /**
     * Returns the last key in the current `MutableVector`.
     *
     * @return - The last key in the current `MutableVector`, or `null` if the
     *           current `MutableVector` is empty.
     */
    public readonly function lastKey()[]: ?int;
    /**
     * Returns the index of the first element that matches the search value.
     *
     * If no element matches the search value, this function returns -1.
     *
     * @param $search_value - The value that will be search for in the current
     *                          `MutableVector`.
     *
     * @return - The key (index) where that value is found; -1 if it is not found.
     *
     * @guide /hack/generics/constraints
     */
    public readonly function linearSearch(mixed $search_value)[]: int;
    public function toVArray()[]: varray<Tv>;
    public function toDArray()[]: darray<int, Tv>;
  }

  /**
   * Represents a read-only (immutable) sequence of key/value pairs, (i.e. a map).
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   */
  <<__Sealed(ImmMap::class, MutableMap::class)>>
  interface ConstMap<Tk as arraykey, +Tv>
    extends
      KeyedContainer<Tk, Tv>,
      ConstCollection<Pair<Tk, Tv>>,
      ConstMapAccess<Tk, Tv>,
      HH\KeyedIterable<Tk, Tv> {
    /**
     * Returns a `ConstVector` containing the values of the current `ConstMap`.
     *
     * The indices of the `ConstVector will be integer-indexed starting from 0,
     * no matter the keys of the `ConstMap`.
     *
     * @return - a `ConstVector` containing the values of the current `ConstMap`.
     */
    public function values()[]: ConstVector<Tv>;
    /**
     * Returns a `ConstVector` containing the keys of the current `ConstMap`.
     *
     * @return - a `ConstVector` containing the keys of the current `ConstMap`.
     */
    public readonly function keys()[]: ConstVector<Tk>;
    /**
     * Returns a `ConstMap` after an operation has been applied to each value in
     * the current `ConstMap`.
     *
     * Every value in the current Map is affected by a call to `map()`, unlike
     * `filter()` where only values that meet a certain criteria are affected.
     *
     * The keys will remain unchanged from the current `ConstMap` to the returned
     * `ConstMap`.
     *
     * @param $fn - The callback containing the operation to apply to the current
     *              `ConstMap` values.
     *
     * @return - a `ConstMap` containing key/value pairs after a user-specified
     *           operation is applied.
     *
     * @guide /hack/collections/examples
     */
    public function map<Tu>(
      (function(Tv)[_]: Tu) $fn,
    )[ctx $fn]: ConstMap<Tk, Tu>;
    /**
     * Returns a `ConstMap` after an operation has been applied to each key and
     * value in the current `ConstMap`.
     *
     * Every key and value in the current `ConstMap` is affected by a call to
     * `mapWithKey()`, unlike `filterWithKey()` where only values that meet a
     * certain criteria are affected.
     *
     * The keys will remain unchanged from this `ConstMap` to the returned
     * `ConstMap`. The keys are only used to help in the mapping operation.
     *
     * @param $fn - The callback containing the operation to apply to the current
     *              `ConstMap` keys and values.
     *
     * @return - a `ConstMap` containing the values after a user-specified
     *           operation on the current `ConstMap`'s keys and values is applied.
     */
    public function mapWithKey<Tu>(
      (function(Tk, Tv)[_]: Tu) $fn,
    )[ctx $fn]: ConstMap<Tk, Tu>;
    /**
     * Returns a `ConstMap` containing the values of the current `ConstMap` that
     * meet a supplied condition.
     *
     * Only values that meet a certain criteria are affected by a call to
     * `filter()`, while all values are affected by a call to `map()`.
     *
     * The keys associated with the current `ConstMap` remain unchanged in the
     * returned `ConstMap`.
     *
     * @param $fn - The callback containing the condition to apply to the current
     *              `ConstMap` values.
     *
     * @return - a Map containing the values after a user-specified condition
     *           is applied.
     *
     * @guide /hack/collections/examples
     */
    public function filter(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: ConstMap<Tk, Tv>;
    /**
     * Returns a `ConstMap` containing the values of the current `ConstMap` that
     * meet a supplied condition applied to its keys and values.
     *
     * Only keys and values that meet a certain criteria are affected by a call to
     * `filterWithKey()`, while all values are affected by a call to
     * `mapWithKey()`.
     *
     * The keys associated with the current `ConstMap` remain unchanged in the
     * returned `ConstMap`; the keys will be used in the filtering process only.
     *
     * @param $fn - The callback containing the condition to apply to the current
     *              `ConstMap` keys and values.
     *
     * @return - a `ConstMap` containing the values after a user-specified
     *           condition is applied to the keys and values of the current
     *           `ConstMap`.
     */
    public function filterWithKey(
      (function(Tk, Tv)[_]: bool) $fn,
    )[ctx $fn]: ConstMap<Tk, Tv>;
    /**
     * Returns a `ConstMap` where each value is a `Pair` that combines the value
     * of the current `ConstMap` and the provided `Traversable`.
     *
     * If the number of values of the current `ConstMap` are not equal to the
     * number of elements in the `Traversable`, then only the combined elements
     * up to and including the final element of the one with the least number of
     * elements is included.
     *
     * The keys associated with the current `ConstMap` remain unchanged in the
     * returned `ConstMap`.
     *
     * @param $traversable - The `Traversable` to use to combine with the
     *                       elements of the current `ConstMap`.
     *
     * @return - The `ConstMap` that combines the values of the current
     *           `ConstMap` with the provided `Traversable`.
     */
    public function zip<Tu>(
      Traversable<Tu> $traversable,
    )[]: ConstMap<Tk, Pair<Tv, Tu>>;
    /**
     * Returns a `ConstMap` containing the first `n` key/values of the current
     * `ConstMap`.
     *
     * The returned `ConstMap` will always be a proper subset of the current
     * `ConstMap`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     * @param $n - The last element that will be included in the `ConstMap`.
     *
     * @return - A `ConstMap` that is a proper subset of the current `ConstMap`
     *           up to `n` elements.
     */
    public function take(int $n)[]: ConstMap<Tk, Tv>;
    /**
     * Returns a `ConstMap` containing the keys and values of the current
     * `ConstMap` up to but not including the first value that produces `false`
     * when passed to the specified callback.
     *
     * The returned `ConstMap` will always be a proper subset of the current
     * `ConstMap`.
     *
     * @param $fn - The callback that is used to determine the stopping condition.
     *
     * @return - A `ConstMap` that is a proper subset of the current `ConstMap`
     *           up until the callback returns `false`.
     */
    public function takeWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: ConstMap<Tk, Tv>;
    /**
     * Returns a `ConstMap` containing the values after the `n`-th element of the
     * current `ConstMap`.
     *
     * The returned `ConstMap` will always be a proper subset of the current
     * `ConstMap`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     *
     * @param $n - The last element to be skipped; the `$n+1` element will be the
     *             first one in the returned `ConstMap`.
     *
     * @return - A `ConstMap` that is a proper subset of the current `ConstMap`
     *           containing values after the specified `n`-th element.
     */
    public function skip(int $n)[]: ConstMap<Tk, Tv>;
    /**
     * Returns a `ConstMap` containing the values of the current `ConstMap`
     * starting after and including the first value that produces `true` when
     * passed to the specified callback.
     *
     * The returned `ConstMap` will always be a proper subset of the current
     * `ConstMap`.
     *
     * @param $fn - The callback used to determine the starting element for the
     *              current `ConstMap`.
     *
     * @return - A `ConstMap` that is a proper subset of the current `ConstMap`
     *           starting after the callback returns `true`.
     */
    public function skipWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: ConstMap<Tk, Tv>;
    /**
     * Returns a subset of the current `ConstMap` starting from a given key
     * location up to, but not including, the element at the provided length from
     * the starting key location.
     *
     * `$start` is 0-based. `$len` is 1-based. So `slice(0, 2)` would return the
     * keys and values at key location 0 and 1.
     *
     * The returned `ConstMap` will always be a proper subset of the current
     * `ConstMap`.
     *
     * @param $start - The starting key location of the current `ConstMap` for the
     *                 returned `ConstMap`.
     * @param $len - The length of the returned `ConstMap`.
     *
     * @return - A `ConstMap` that is a proper subset of the current `ConstMap`
     *           starting at `$start` up to but not including the element
     *           `$start + $len`.
     */
    public function slice(int $start, int $len)[]: ConstMap<Tk, Tv>;
    /**
     * Returns a `ConstVector` that is the concatenation of the values of the
     * current `ConstMap` and the values of the provided `Traversable`.
     *
     * The provided `Traversable` is concatenated to the end of the current
     * `ConstMap` to produce the returned `ConstVector`.
     *
     * @param $traversable - The `Traversable` to concatenate to the current
     *                       `ConstMap`.
     *
     * @return - The integer-indexed concatenated `ConstVector`.
     *
     * @guide /hack/generics/constraints
     */
    public function concat<Tu super Tv>(
      Traversable<Tu> $traversable,
    )[]: ConstVector<Tu>;
    /**
     * Returns the first value in the current `ConstMap`.
     *
     * @return - The first value in the current `ConstMap`,  or `null` if the
     *           `ConstMap` is empty.
     */
    public function firstValue()[]: ?Tv;
    /**
     * Returns the first key in the current `ConstMap`.
     *
     * @return - The first key in the current `ConstMap`, or `null` if the
     *           `ConstMap` is empty.
     */
    public readonly function firstKey()[]: ?Tk;
    /**
     * Returns the last value in the current `ConstMap`.
     *
     * @return - The last value in the current `ConstMap`, or `null` if the
     *           `ConstMap` is empty.
     */
    public function lastValue()[]: ?Tv;
    /**
     * Returns the last key in the current `ConstMap`.
     *
     * @return - The last key in the current `ConstMap`, or null if the `ConstMap`
     *           is empty.
     */
    public readonly function lastKey()[]: ?Tk;
    public function toVArray()[]: varray<Tv>;
    public function toDArray()[]: darray<Tk, Tv>;
  }

  /**
   * Represents a write-enabled (mutable) sequence of key/value pairs
   * (i.e. a map).
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   */
  <<__Sealed(Map::class)>>
  interface MutableMap<Tk as arraykey, Tv>
    extends ConstMap<Tk, Tv>, Collection<Pair<Tk, Tv>>, MapAccess<Tk, Tv> {
    /**
     * Returns a `MutableVector` containing the values of the current
     * `MutableMap`.
     *
     * The indices of the `MutableVector will be integer-indexed starting from 0,
     * no matter the keys of the `MutableMap`.
     *
     * @return - a `MutableVector` containing the values of the current
     *           `MutableMap`.
     */
    public function values()[]: MutableVector<Tv>;
    /**
     * Returns a `MutableVector` containing the keys of the current `MutableMap`.
     *
     * @return - a `MutableVector` containing the keys of the current
     *           `MutableMap`.
     */
    public readonly function keys()[]: MutableVector<Tk>;
    /**
     * Returns a `MutableMap` after an operation has been applied to each value
     * in the current `MutableMap`.
     *
     * Every value in the current Map is affected by a call to `map()`, unlike
     * `filter()` where only values that meet a certain criteria are affected.
     *
     * The keys will remain unchanged from the current `MutableMap` to the
     * returned `MutableMap`.
     *
     * @param $fn - The callback containing the operation to apply to the current
     *              `MutableMap` values.
     *
     * @return - a `MutableMap` containing key/value pairs after a user-specified
     *           operation is applied.
     *
     * @guide /hack/collections/examples
     */
    public function map<Tu>(
      (function(Tv)[_]: Tu) $fn,
    )[ctx $fn]: MutableMap<Tk, Tu>;
    /**
     * Returns a `MutableMap` after an operation has been applied to each key and
     * value in the current `MutableMap`.
     *
     * Every key and value in the current `MutableMap` is affected by a call to
     * `mapWithKey()`, unlike `filterWithKey()` where only values that meet a
     * certain criteria are affected.
     *
     * The keys will remain unchanged from this `MutableMap` to the returned
     * `MutableMap`. The keys are only used to help in the mapping operation.
     *
     * @param $fn - The callback containing the operation to apply to the current
     *              `MutableMap` keys and values.
     *
     * @return - a `MutableMap` containing the values after a user-specified
     *           operation on the current `MutableMap`'s keys and values is
     *           applied.
     */
    public function mapWithKey<Tu>(
      (function(Tk, Tv)[_]: Tu) $fn,
    )[ctx $fn]: MutableMap<Tk, Tu>;
    /**
     * Returns a `MutableMap` containing the values of the current `MutableMap`
     * that meet a supplied condition.
     *
     * Only values that meet a certain criteria are affected by a call to
     * `filter()`, while all values are affected by a call to `map()`.
     *
     * The keys associated with the current `MutableMap` remain unchanged in the
     * returned `MutableMap`.
     *
     * @param $fn - The callback containing the condition to apply to the current
     *              `MutableMap` values.
     *
     * @return - a Map containing the values after a user-specified condition
     *           is applied.
     *
     * @guide /hack/collections/examples
     */
    public function filter(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: MutableMap<Tk, Tv>;
    /**
     * Returns a `MutableMap` containing the values of the current `MutableMap`
     * that meet a supplied condition applied to its keys and values.
     *
     * Only keys and values that meet a certain criteria are affected by a call
     * to `filterWithKey()`, while all values are affected by a call to
     * `mapWithKey()`.
     *
     * The keys associated with the current `MutableMap` remain unchanged in the
     * returned `MutableMap`; the keys will be used in the filtering process only.
     *
     * @param $fn - The callback containing the condition to apply to the current
     *              `MutableMap` keys and values.
     *
     * @return - a `MutableMap` containing the values after a user-specified
     *           condition is applied to the keys and values of the current
     *           `MutableMap`.
     */
    public function filterWithKey(
      (function(Tk, Tv)[_]: bool) $fn,
    )[ctx $fn]: MutableMap<Tk, Tv>;
    /**
     * Returns a `MutableMap` where each value is a `Pair` that combines the
     * value of the current `MutableMap` and the provided `Traversable`.
     *
     * If the number of values of the current `MutableMap` are not equal to the
     * number of elements in the `Traversable`, then only the combined elements
     * up to and including the final element of the one with the least number of
     * elements is included.
     *
     * The keys associated with the current `MutableMap` remain unchanged in the
     * returned `MutableMap`.
     *
     * @param $traversable - The `Traversable` to use to combine with the
     *                       elements of the current `MutableMap`.
     *
     * @return - The `MutableMap` that combines the values of the current
     *           `MutableMap` with the provided `Traversable`.
     */
    public function zip<Tu>(
      Traversable<Tu> $traversable,
    )[]: MutableMap<Tk, Pair<Tv, Tu>>;
    /**
     * Returns a `MutableMap` containing the first `n` key/values of the current
     * `MutableMap`.
     *
     * The returned `MutableMap` will always be a proper subset of the current
     * `MutableMap`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     * @param $n - The last element that will be included in the `MutableMap`.
     *
     * @return - A `MutableMap` that is a proper subset of the current
     *          `MutableMap` up to `n` elements.
     */
    public function take(int $n)[]: MutableMap<Tk, Tv>;
    /**
     * Returns a `MutableMap` containing the keys and values of the current
     * `MutableMap` up to but not including the first value that produces `false`
     * when passed to the specified callback.
     *
     * The returned `MutableMap` will always be a proper subset of the current
     * `MutableMap`.
     *
     * @param $fn - The callback that is used to determine the stopping condition.
     *
     * @return - A `MutableMap` that is a proper subset of the current
     *           `MutableMap` up until the callback returns `false`.
     */
    public function takeWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: MutableMap<Tk, Tv>;
    /**
     * Returns a `MutableMap` containing the values after the `n`-th element of
     * the current `MutableMap`.
     *
     * The returned `MutableMap` will always be a proper subset of the current
     * `MutableMap`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     *
     * @param $n - The last element to be skipped; the `$n+1` element will be the
     *             first one in the returned `MutableMap`.
     *
     * @return - A `MutableMap` that is a proper subset of the current
     *           `MutableMap` containing values after the specified `n`-th
     *           element.
     */
    public function skip(int $n)[]: MutableMap<Tk, Tv>;
    /**
     * Returns a `MutableMap` containing the values of the current `MutableMap`
     * starting after and including the first value that produces `true` when
     * passed to the specified callback.
     *
     * The returned `MutableMap` will always be a proper subset of the current
     * `MutableMap`.
     *
     * @param $fn - The callback used to determine the starting element for the
     *              current `MutableMap`.
     *
     * @return - A `MutableMap` that is a proper subset of the current
     *           `MutableMap` starting after the callback returns `true`.
     */
    public function skipWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: MutableMap<Tk, Tv>;
    /**
     * Returns a subset of the current `MutableMap` starting from a given key
     * location up to, but not including, the element at the provided length from
     * the starting key location.
     *
     * `$start` is 0-based. `$len` is 1-based. So `slice(0, 2)` would return the
     * keys and values at key location 0 and 1.
     *
     * The returned `MutableMap` will always be a proper subset of the current
     * `MutableMap`.
     *
     * @param $start - The starting key location of the current `MutableMap` for
     *                  the feturned `MutableMap`.
     * @param $len - The length of the returned `MutableMap`.
     *
     * @return - A `MutableMap` that is a proper subset of the current
     *           `MutableMap` starting at `$start` up to but not including the
     *           element `$start + $len`.
     */
    public function slice(int $start, int $len)[]: MutableMap<Tk, Tv>;
    /**
     * Returns a `MutableVector` that is the concatenation of the values of the
     * current `MutableMap` and the values of the provided `Traversable`.
     *
     * The provided `Traversable` is concatenated to the end of the current
     * `MutableMap` to produce the returned `MutableVector`.
     *
     * @param $traversable - The `Traversable` to concatenate to the current
     *                       `MutableMap`.
     *
     * @return - The integer-indexed concatenated `MutableVector`.
     *
     * @guide /hack/generics/constraints
     */
    public function concat<Tu super Tv>(
      Traversable<Tu> $traversable,
    )[]: MutableVector<Tu>;
    /**
     * Returns the first value in the current `MutableMap`.
     *
     * @return - The first value in the current `MutableMap`,  or `null` if the
     *           `MutableMap` is empty.
     */
    public function firstValue()[]: ?Tv;
    /**
     * Returns the first key in the current `MutableMap`.
     *
     * @return - The first key in the current `MutableMap`, or `null` if the
     *           `MutableMap` is empty.
     */
    public readonly function firstKey()[]: ?Tk;
    /**
     * Returns the last value in the current `MutableMap`.
     *
     * @return - The last value in the current `MutableMap`, or `null` if the
     *           `MutableMap` is empty.
     */
    public function lastValue()[]: ?Tv;
    /**
     * Returns the last key in the current `MutableMap`.
     *
     * @return - The last key in the current `MutableMap`, or null if the
     *           `MutableMap` is empty.
     */
    public readonly function lastKey()[]: ?Tk;
    public function toVArray()[]: varray<Tv>;
    public function toDArray()[]: darray<Tk, Tv>;
  }

  /**
   * Represents a read-only (immutable) set of values, with no keys
   * (i.e., a set).
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   */
  <<__Sealed(ImmSet::class, MutableSet::class)>>
  interface ConstSet<+Tv as arraykey>
    extends
      KeyedContainer<Tv, Tv>,
      ConstCollection<Tv>,
      ConstSetAccess<Tv>,
      HH\KeyedIterable<arraykey, Tv> {
    /**
     * Returns a `ConstVector` containing the values of the current `ConstSet`.
     *
     * This method is interchangeable with `keys()`.
     *
     * @return - a `ConstVector` (integer-indexed) containing the values of the
     *           current `ConstSet`.
     */
    public function values()[]: ConstVector<Tv>;
    /**
     * Returns a `ConstVector` containing the values of the current `ConstSet`.
     *
     * Sets don't have keys, so this will return the values.
     *
     * This method is interchangeable with `values()`.
     *
     * @return - a `ConstVector` (integer-indexed) containing the values of the
     *           current `ConstSet`.
     */
    public readonly function keys()[]: ConstVector<arraykey>;
    /**
     * Returns a `ConstSet` containing the values after an operation has been
     * applied to each value in the current `ConstSet`.
     *
     * Every value in the current `ConstSet` is affected by a call to `map()`,
     * unlike `filter()` where only values that meet a certain criteria are
     * affected.
     *
     * @param $fn - The callback containing the operation to apply to the
     *              current `ConstSet` values.
     *
     * @return - a `ConstSet` containing the values after a user-specified
     *           operation is applied.
     *
     * @guide /hack/collections/examples
     */
  public function map<Tu as arraykey>((function(Tv)[_]: Tu) $fn)[ctx $fn]: ConstSet<Tu>;
    /**
     * Returns a `ConstSet` containing the values after an operation has been
     * applied to each "key" and value in the current Set.
     *
     * Since sets don't have keys, the callback uses the values as the keys
     * as well.
     *
     * Every value in the current `ConstSet` is affected by a call to
     * `mapWithKey()`, unlike `filterWithKey()` where only values that meet a
     * certain criteria are affected.
     *
     * @param $fn - The callback containing the operation to apply to the
     *              current `ConstSet` "keys" and values.
     *
     * @return - a `ConstSet` containing the values after a user-specified
     *           operation on the current `ConstSet`'s values is applied.
     */
  public function mapWithKey<Tu as arraykey>((function(arraykey, Tv)[_]: Tu) $fn)[ctx $fn]: ConstSet<Tu>;
    /**
     * Returns a `ConstSet` containing the values of the current `ConstSet` that
     * meet a supplied condition applied to each value.
     *
     * Only values that meet a certain criteria are affected by a call to
     * `filter()`, while all values are affected by a call to `map()`.
     *
     * @param $fn - The callback containing the condition to apply to the
     *              current `ConstSet` values.
     *
     * @return - a `ConstSet` containing the values after a user-specified
     *           condition is applied.
     *
     * @guide /hack/collections/examples
     */
    public function filter((function(Tv)[_]: bool) $fn)[ctx $fn]: ConstSet<Tv>;
    /**
     * Returns a `ConstSet` containing the values of the current `ConstSet` that
     * meet a supplied condition applied to its "keys" and values.
     *
     * Since sets don't have keys, the callback uses the values as the keys
     * as well.
     *
     * Only values that meet a certain criteria are affected by a call to
     * `filterWithKey()`, while all values are affected by a call to
     * `mapWithKey()`.
     *
     * @param $fn - The callback containing the condition to apply to the
     *              current `ConstSet` "keys" and values.
     *
     * @return - a `ConstSet` containing the values after a user-specified
     *           condition is applied to the values of the current `ConstSet`.
     */
    public function filterWithKey(
      (function(arraykey, Tv)[_]: bool) $fn,
    )[ctx $fn]: ConstSet<Tv>;
    /**
     * Returns a `ConstSet` where each value is a `Pair` that combines the value
     * of the current `ConstSet` and the provided `Traversable`.
     *
     * If the number of values of the current `ConstMap` are not equal to the
     * number of elements in the `Traversable`, then only the combined elements
     * up to and including the final element of the one with the least number of
     * elements is included.
     *
     * Note that some implementations of sets only support certain types of keys
     * (e.g., only `int` or `string` values allowed). In that case, this method
     * could thrown an exception since a `Pair` wouldn't be supported/
     *
     * @param $traversable - The `Traversable` to use to combine with the
     *                       elements of the current `ConstSet`.
     *
     * @return - The `ConstSet` that combines the values of the current
     *           `ConstSet` with the provided `Traversable`.
     */
    public function zip<Tu>(Traversable<Tu> $traversable)[]: ConstSet<nothing>;
    /**
     * Returns a `ConstSet` containing the first `n` values of the current
     * `ConstSet`.
     *
     * The returned `ConstSet` will always be a proper subset of the current
     * `ConstSet`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     * @param $n - The last element that will be included in the `ConstSet`.
     *
     * @return - A `ConstSet` that is a proper subset of the current `ConstSet`
     *           up to `n` elements.
     */
    public function take(int $n)[]: ConstSet<Tv>;
    /**
     * Returns a `ConstSet` containing the values of the current `ConstSet` up to
     * but not including the first value that produces `false` when passed to the
     * specified callback.
     *
     * The returned `ConstSet` will always be a proper subset of the current
     * `ConstSet`.
     *
     * @param $fn - The callback that is used to determine the stopping condition.
     *
     * @return - A `ConstSet` that is a proper subset of the current `ConstSet`
     *           up until the callback returns `false`.
     */
    public function takeWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: ConstSet<Tv>;
    /**
     * Returns a `ConstSet` containing the values after the `n`-th element of the
     * current `ConstSet`.
     *
     * The returned `ConstSet` will always be a proper subset of the current
     * `ConstSet`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     * @param $n - The last element to be skipped; the `$n+1` element will be the
     *             first one in the returned `ConstSet`.
     *
     * @return - A `ConstSet` that is a proper subset of the current `ConstSet`
     *           containing values after the specified `n`-th element.
     */
    public function skip(int $n)[]: ConstSet<Tv>;
    /**
     * Returns a `ConstSet` containing the values of the current `ConstSet`
     * starting after and including the first value that produces `true` when
     * passed to the specified callback.
     *
     * The returned `ConstSet` will always be a proper subset of the current
     * `ConstSet`.
     *
     * @param $fn - The callback used to determine the starting element for the
     *              `ConstSet`.
     *
     * @return - A `ConstSet` that is a proper subset of the current `ConstSet`
     *           starting after the callback returns `true`.
     */
    public function skipWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: ConstSet<Tv>;
    /**
     * Returns a subset of the current `ConstSet` starting from a given key up
     * to, but not including, the element at the provided length from the
     * starting key.
     *
     * `$start` is 0-based. `$len` is 1-based. So `slice(0, 2)` would return the
     * elements at key 0 and 1.
     *
     * The returned `ConstSet` will always be a proper subset of the current
     * `ConstSet`.
     *
     * @param $start - The starting value in the current `ConstSet` for the
     *                 returned `ConstSet`.
     * @param $len - The length of the returned `ConstSet`.
     *
     * @return - A `ConstSet` that is a proper subset of the current `ConstSet`
     *           starting at `$start` up to but not including the element
     *           `$start + $len`.
     */
    public function slice(int $start, int $len)[]: ConstSet<Tv>;
    /**
     * Returns a `ConstVector` that is the concatenation of the values of the
     * current `ConstSet` and the values of the provided `Traversable`.
     *
     * The values of the provided `Traversable` is concatenated to the end of the
     * current `ConstSet` to produce the returned `ConstVector`.
     *
     * @param $traversable - The `Traversable` to concatenate to the current
     *                       `ConstSet`.
     *
     * @return - The concatenated `ConstVector`.
     *
     * @guide /hack/generics/constraints
     */
    public function concat<Tu super Tv>(
      Traversable<Tu> $traversable,
    )[]: ConstVector<Tu>;
    /**
     * Returns the first value in the current `ConstSet`.
     *
     * @return - The first value in the current `ConstSet`, or `null` if the
     *           `ConstSet` is empty.
     */
    public function firstValue()[]: ?Tv;
    /**
     * Returns the first "key" in the current `ConstSet`.
     *
     * Since sets do not have keys, it returns the first value.
     *
     * This method is interchangeable with `firstValue()`.
     *
     * @return - The first value in the current `ConstSet`, or `null` if the
     *           `ConstSet` is empty.
     */
    public readonly function firstKey()[]: ?arraykey;
    /**
     * Returns the last value in the current `ConstSet`.
     *
     * @return - The last value in the current `ConstSet`, or `null` if the
     *           current `ConstSet` is empty.
     */
    public function lastValue()[]: ?Tv;
    /**
     * Returns the last "key" in the current `ConstSet`.
     *
     * Since sets do not have keys, it returns the last value.
     *
     * This method is interchangeable with `lastValue()`.
     *
     * @return - The last value in the current `ConstSet`, or `null` if the
     *           current `ConstSet` is empty.
     */
    public readonly function lastKey()[]: ?arraykey;
    public function toVArray()[]: varray<Tv>;
    public function toDArray()[]: darray<Tv, Tv>;
  }

  /**
   * Represents a write-enabled (mutable) set of values, not indexed by keys
   * (i.e. a set).
   *
   * @guide /hack/collections/introduction
   * @guide /hack/collections/interfaces
   */
  <<__Sealed(Set::class)>>
  interface MutableSet<Tv as arraykey>
    extends ConstSet<Tv>, Collection<Tv>, SetAccess<Tv> {
    /**
     * Returns a `MutableVector` containing the values of the current
     * `MutableSet`.
     *
     * This method is interchangeable with `keys()`.
     *
     * @return - a `MutableVector` (integer-indexed) containing the values of the
     *           current `MutableSet`.
     */
    public function values()[]: MutableVector<Tv>;
    /**
     * Returns a `MutableVector` containing the values of the current
     * `MutableSet`.
     *
     * Sets don't have keys, so this will return the values.
     *
     * This method is interchangeable with `values()`.
     *
     * @return - a `MutableVector` (integer-indexed) containing the values of the
     *           current `MutableSet`.
     */
    public readonly function keys()[]: MutableVector<arraykey>;
    /**
     * Returns a `MutableSet` containing the values after an operation has been
     * applied to each value in the current `MutableSet`.
     *
     * Every value in the current `MutableSet` is affected by a call to `map()`,
     * unlike `filter()` where only values that meet a certain criteria are
     * affected.
     *
     * @param $fn - The callback containing the operation to apply to the
     *              current `MutableSet` values.
     *
     * @return - a `MutableSet` containing the values after a user-specified
     *           operation is applied.
     *
     * @guide /hack/collections/examples
     */
    public function map<Tu as arraykey>(
      (function(Tv)[_]: Tu) $fn,
    )[ctx $fn]: MutableSet<Tu>;
    /**
     * Returns a `MutableSet` containing the values after an operation has been
     * applied to each "key" and value in the current Set.
     *
     * Since sets don't have keys, the callback uses the values as the keys
     * as well.
     *
     * Every value in the current `MutableSet` is affected by a call to
     * `mapWithKey()`, unlike `filterWithKey()` where only values that meet a
     * certain criteria are affected.
     *
     * @param $fn - The callback containing the operation to apply to the
     *              current `MutableSet` "keys" and values.
     *
     * @return - a `MutableSet` containing the values after a user-specified
     *           operation on the current `MutableSet`'s values is applied.
     */
    public function mapWithKey<Tu as arraykey>(
      (function(arraykey, Tv)[_]: Tu) $fn,
    )[ctx $fn]: MutableSet<Tu>;
    /**
     * Returns a `MutableSet` containing the values of the current `MutableSet`
     * that meet a supplied condition applied to each value.
     *
     * Only values that meet a certain criteria are affected by a call to
     * `filter()`, while all values are affected by a call to `map()`.
     *
     * @param $fn - The callback containing the condition to apply to the
     *              current `MutableSet` values.
     *
     * @return - a `MutableSet` containing the values after a user-specified
     *           condition is applied.
     *
     * @guide /hack/collections/examples
     */
    public function filter(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: MutableSet<Tv>;
    /**
     * Returns a `MutableSet` containing the values of the current `MutableSet`
     * that meet a supplied condition applied to its "keys" and values.
     *
     * Since sets don't have keys, the callback uses the values as the keys
     * as well.
     *
     * Only values that meet a certain criteria are affected by a call to
     * `filterWithKey()`, while all values are affected by a call to
     * `mapWithKey()`.
     *
     * @param $fn - The callback containing the condition to apply to the
     *              current `MutableSet` "keys" and values.
     *
     * @return - a `MutableSet` containing the values after a user-specified
     *           condition is applied to the values of the current `MutableSet`.
     */
    public function filterWithKey(
      (function(arraykey, Tv)[_]: bool) $fn,
    )[ctx $fn]: MutableSet<Tv>;
    /**
     * Returns a `MutableSet` where each value is a `Pair` that combines the
     * value of the current `MutableSet` and the provided `Traversable`.
     *
     * If the number of values of the current `ConstMap` are not equal to the
     * number of elements in the `Traversable`, then only the combined elements
     * up to and including the final element of the one with the least number of
     * elements is included.
     *
     * Note that some implementations of sets only support certain types of keys
     * (e.g., only `int` or `string` values allowed). In that case, this method
     * could thrown an exception since a `Pair` wouldn't be supported/
     *
     * @param $traversable - The `Traversable` to use to combine with the
     *                       elements of the current `MutableSet`.
     *
     * @return - The `MutableSet` that combines the values of the current
     *           `MutableSet` with the provided `Traversable`.
     */
    public function zip<Tu>(
      Traversable<Tu> $traversable,
    )[]: MutableSet<nothing>;
    /**
     * Returns a `MutableSet` containing the first `n` values of the current
     * `MutableSet`.
     *
     * The returned `MutableSet` will always be a proper subset of the current
     * `MutableSet`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     * @param $n - The last element that will be included in the `MutableSet`.
     *
     * @return - A `MutableSet` that is a proper subset of the current
     *           `MutableSet` up to `n` elements.
     */
    public function take(int $n)[]: MutableSet<Tv>;
    /**
     * Returns a `MutableSet` containing the values of the current `MutableSet`
     * up to but not including the first value that produces `false` when passed
     * to the specified callback.
     *
     * The returned `MutableSet` will always be a proper subset of the current
     * `MutableSet`.
     *
     * @param $fn - The callback that is used to determine the stopping condition.
     *
     * @return - A `MutableSet` that is a proper subset of the current
     *           `MutableSet` up until the callback returns `false`.
     */
    public function takeWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: MutableSet<Tv>;
    /**
     * Returns a `MutableSet` containing the values after the `n`-th element of
     * the current `MutableSet`.
     *
     * The returned `MutableSet` will always be a proper subset of the current
     * `MutableSet`.
     *
     * `$n` is 1-based. So the first element is 1, the second 2, etc.
     *
     * @param $n - The last element to be skipped; the `$n+1` element will be the
     *             first one in the returned `MutableSet`.
     *
     * @return - A `MutableSet` that is a proper subset of the current
     *           `MutableSet` containing values after the specified `n`-th
     *           element.
     */
    public function skip(int $n)[]: MutableSet<Tv>;
    /**
     * Returns a `MutableSet` containing the values of the current `MutableSet`
     * starting after and including the first value that produces `true` when
     * passed to the specified callback.
     *
     * The returned `MutableSet` will always be a proper subset of the current
     * `MutableSet`.
     *
     * @param $fn - The callback used to determine the starting element for the
     *              `MutableSet`.
     *
     * @return - A `MutableSet` that is a proper subset of the current
     *           `MutableSet` starting after the callback returns `true`.
     */
    public function skipWhile(
      (function(Tv)[_]: bool) $fn,
    )[ctx $fn]: MutableSet<Tv>;
    /**
     * Returns a subset of the current `MutableSet` starting from a given key up
     * to, but not including, the element at the provided length from the
     * starting key.
     *
     * `$start` is 0-based. `$len` is 1-based. So `slice(0, 2)` would return the
     * elements at key 0 and 1.
     *
     * The returned `MutableSet` will always be a proper subset of the current
     * `MutableSet`.
     *
     * @param $start - The starting value in the current `MutableSet` for the
     *                 returned `MutableSet`.
     * @param $len - The length of the returned `MutableSet`.
     *
     * @return - A `MutableSet` that is a proper subset of the current
     *           `MutableSet` starting at `$start` up to but not including the
     *           element `$start + $len`.
     */
    public function slice(int $start, int $len)[]: MutableSet<Tv>;
    /**
     * Returns a `MutableVector` that is the concatenation of the values of the
     * current `MutableSet` and the values of the provided `Traversable`.
     *
     * The values of the provided `Traversable` is concatenated to the end of the
     * current `MutableSet` to produce the returned `MutableVector`.
     *
     * @param $traversable - The `Traversable` to concatenate to the current
     *                       `MutableSet`.
     *
     * @return - The concatenated `MutableVector`.
     *
     * @guide /hack/generics/constraints
     */
    public function concat<Tu super Tv>(
      Traversable<Tu> $traversable,
    )[]: MutableVector<Tu>;
    /**
     * Returns the first value in the current `MutableSet`.
     *
     * @return - The first value in the current `MutableSet`, or `null` if the
     *           `MutableSet` is empty.
     */
    public function firstValue()[]: ?Tv;
    /**
     * Returns the first "key" in the current `MutableSet`.
     *
     * Since sets do not have keys, it returns the first value.
     *
     * This method is interchangeable with `firstValue()`.
     *
     * @return - The first value in the current `MutableSet`, or `null` if the
     *           `MutableSet` is empty.
     */
    public readonly function firstKey()[]: ?arraykey;
    /**
     * Returns the last value in the current `MutableSet`.
     *
     * @return - The last value in the current `MutableSet`, or `null` if the
     *           current `MutableSet` is empty.
     */
    public function lastValue()[]: ?Tv;
    /**
     * Returns the last "key" in the current `MutableSet`.
     *
     * Since sets do not have keys, it returns the last value.
     *
     * This method is interchangeable with `lastValue()`.
     *
     * @return - The last value in the current `MutableSet`, or `null` if the
     *           current `MutableSet` is empty.
     */
    public readonly function lastKey()[]: ?arraykey;
    public function toVArray()[]: varray<Tv>;
    public function toDArray()[]: darray<Tv, Tv>;
  }

} // namespace
