<?hh
<<file:__EnableUnstableFeatures('readonly')>>

namespace {

/**
 * @internal
 *
 * An iterator implementation for iterating over a Pair.
 */
<<__NativeData>>
final class PairIterator implements HH\KeyedIterator<int, mixed> {

  public function __construct()[]: void {}

  /** Returns the current value that the iterator points to.
   * @return mixed
   */
  <<__Native>>
  public function current()[]: mixed;

  /** Returns the current key that the iterator points to.
   * @return mixed
   */
  <<__Native>>
  public function key()[]: int;

  /** Returns true if the iterator points to a valid value, returns false
   * otherwise.
   * @return bool
   */
  <<__Native>>
  public function valid()[]: bool;

  /** Advance this iterator forward one position.
   */
  <<__Native>>
  public function next()[]: void;

  /** Move this iterator back to the first position.
   */
  <<__Native>>
  public function rewind()[]: void;

}

} // empty namespace
namespace HH {

/**
 * `Pair` is a fixed-size collection with exactly two elements (possibly of
 * different types). HHVM provides a native implementation for this class.
 * The PHP class definition below is not actually used at run time; it is
 * simply provided for the typechecker and for developer reference.
 *
 * Like all objects in PHP, `Pair`s have reference-like semantics. The elements
 * or a `Pair` cannot be mutated (i.e. you can't assign to the elements of a
 * `Pair`) though `Pair`s may contain mutable objects.
 *
 * `Pair`s only support integer keys. If a non-integer key is used, an
 * exception will be thrown. Only keys 0 and 1 will ever exist in a `Pair`,
 * and these keys will always exist.
 *
 * `Pair`s support `$m[$k]` style syntax for getting values by key. `Pair`s
 * also support `isset($m[$k])` and `empty($m[$k])` syntax, and they provide
 * similar semantics to arrays.
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
  public function __construct()[]: void {
    throw new \InvalidOperationException(
      "Pairs cannot be created using the new operator"
    );
  }

  /**
   * Returns `false`; a `Pair` cannot be empty.
   *
   * @return - `false`
   */
  public readonly function isEmpty()[]: bool { return false; }

  /**
   * Returns 2; a `Pair` always has two values.
   *
   * @return - 2
   */
  public readonly function count()[]: int { return 2; }

  /**
   * Returns an `Iterable` view of the current `Pair`.
   *
   * The `Iterable` returned is one that produces the values from the current
   * `Pair`.
   *
   * @return - The `Iterable` view of the current `Pair`.
   */
  public function items()[]: Iterable<mixed> {
    /* HH_FIXME[2049] */
    return new \LazyIterableView($this);
  }

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
  /* HH_FIXME[2049] */
  public readonly function keys()[]: ImmVector<int> {
    return ImmVector { 0, 1 };
  }

  /**
   * Returns an `ImmVector` containing the values of the current `Pair`.
   *
   * This method is interchangeable with `toImmVector()`.
   *
   * @return - an `ImmVector` containing the values of the current `Pair`.
   */
  <<__Native>>
  /* HH_FIXME[2049] */
  public function values()[]: ImmVector<mixed>;

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
  public function lazy()[]: KeyedIterable<int, mixed> {
    /* HH_FIXME[2049] */
    return new \LazyKeyedIterableView($this);
  }

  public function toVArray()[]: varray<mixed> {
    return vec($this);
  }

  public function toDArray()[]: darray<int, mixed> {
    return dict($this);
  }

  /**
    * Returns a `Vector` containing the elements of the current `Pair`.
    *
    * The returned `Vector` will, of course, be mutable.
    *
    * @return - a `Vector` with the elements of the current `Pair`.
    */
  <<__Native>>
  /* HH_FIXME[2049] */
  public function toVector()[]: Vector<mixed>;

  /**
    * Returns an immutable vector (`ImmVector`) containing the elements of the
    * current `Pair`.
    *
    * @return - an `ImmVector` with the elements of the current `Pair`.
    */
  <<__Native>>
  /* HH_FIXME[2049] */
  public function toImmVector()[]: ImmVector<mixed>;

  /**
   * Returns an integer-keyed `Map` based on the elements of the current `Pair`.
   *
   * The keys are 0 and 1.
   *
   * @return - an integer-keyed `Map` with the values of the current `Pair`.
   */
  <<__Native>>
  /* HH_FIXME[2049] */
  public function toMap()[]: Map<int, mixed>;

  /**
   * Returns an immutable, integer-keyed map (`ImmMap`) based on the elements of
   * the current `Pair`.
   *
   * The keys are 0 and 1.
   *
   * @return - an `ImmMap` with the values of the current `Pair`.
   */
  <<__Native>>
  /* HH_FIXME[2049] */
  public function toImmMap()[]: ImmMap<int, mixed>;

  /**
   * Returns a `Set` with the values of the current `Pair`.
   *
   * @return - a `Set` with the current values of the current `Pair`.
   */
  <<__Native>>
  public function toSet(
  )[]: Set<arraykey> where Tv1 as arraykey, Tv2 as arraykey;

  /**
   * Returns an immutable set (`ImmSet`) with the values of the current `Pair`.
   *
   * @return - an `ImmSet` with the current values of the current `Pair`.
   */
  <<__Native>>
  public function toImmSet(
  )[]: ImmSet<arraykey> where Tv1 as arraykey, Tv2 as arraykey;

  /**
   * Returns an immutable version of this collection.
   *
   * Because a `Pair` is immutable by default, this just returns the current
   * object.
   *
   * @return - this `Pair`
   */
  public function immutable(): this { return $this; }

  /**
   * Returns a `varray` whose values are the keys from the current `Pair`.
   *
   * @return - a `varray` with the integer keys from the current `Pair`.
   */
  public function toKeysArray()[]: varray<int> { return varray[0, 1]; }

  /**
   * Returns an `varray` containing the values from the current `Pair`.
   *
   * This method is interchangeable with `toVArray()`.
   *
   * @return - an `varray` containing the values from the current `Pair`.
   */
  public function toValuesArray()[]: varray<mixed> {
    return vec($this);
  }

  /**
   * Returns an iterator that points to beginning of the current `Pair`.
   *
   * @return - A `KeyedIterator` that allows you to traverse the current `Pair`.
   */
  <<__Native>>
  public function getIterator()[]: KeyedIterator<int, mixed>;

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
  /* HH_FIXME[2049] */
  public function map<Tu>((function(mixed)[_]: Tu) $callback)[ctx $callback]: ImmVector<Tu> {
    return ImmVector { $callback($this[0]), $callback($this[1]) };
  }

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
  public function mapWithKey<Tu>((function(int, mixed)[_]: Tu) $callback)[ctx $callback]:
    /* HH_FIXME[2049] */
    ImmVector<Tu> {
    return ImmVector { $callback(0, $this[0]), $callback(1, $this[1]) };
  }

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
  /* HH_FIXME[2049] */
  public function filter((function(mixed)[_]: bool) $callback)[ctx $callback]: ImmVector<mixed> {
    // TODO(T124995052): This code is dead, or at least not usable. The
    // `UNSAFE_CAST` here covers a legitimate bug, that unset on `varray` has
    // been dead since late 2020 to early 2021.
    $values = HH\FIXME\UNSAFE_CAST<varray<mixed>, darray<arraykey, mixed>>(
      $this->toVArray(),
    );
    if (!$callback($values[0])) { unset($values[0]); }
    if (!$callback($values[1])) { unset($values[1]); }
    /* HH_FIXME[2049] */
    return new \HH\ImmVector($values);
  }

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
  public function filterWithKey((function(int, mixed)[_]: bool) $callback)[ctx $callback]:
    /* HH_FIXME[2049] */
    ImmVector<mixed> {
    // TODO(T124995052): This code is dead, or at least not usable. The
    // `UNSAFE_CAST` here covers a legitimate bug, that unset on `varray` has
    // been dead since late 2020 to early 2021.
    $values = HH\FIXME\UNSAFE_CAST<varray<mixed>, darray<arraykey, mixed>>(
      $this->toVArray(),
    );
    if (!$callback(0, $values[0])) { unset($values[0]); }
    if (!$callback(1, $values[1])) { unset($values[1]); }
    /* HH_FIXME[2049] */
    return new \HH\ImmVector($values);
  }

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
  public function zip<Tu>(Traversable<Tu> $traversable)[]:
    /* HH_FIXME[2049] */
    ImmVector<\HH\Pair<mixed, Tu>> {
    $ret = vec[];
    $i = 0;
    foreach ($traversable as $value) {
      if ($i >= 2) break;
      $ret[] = \HH\Pair { $this->at($i), $value };
      $i++;
    }
    /* HH_FIXME[2049] */
    return new ImmVector($ret);
  }

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
  /* HH_FIXME[2049] */
  public function take(int $n)[]: ImmVector<mixed> {
    if (!\is_int($n)) {
      throw new \InvalidArgumentException(
        "Parameter n must be an integer"
      );
    }
    if ($n <= 0) return \HH\ImmVector {};
    if ($n == 1) return \HH\ImmVector { $this[0] };
    return $this->toImmVector();
  }

  /**
   * Returns an `ImmVector` containing the values of the current `Pair` up to
   * but not including the first value that produces `false` when passed to the
   * specified callback.
   *
   * @param $callback - The callback that is used to determine the stopping
   *                    condition.
   *
   * @return - An `ImmVector` that contains the values of the current `Pair` up
   *           until the callback returns `false`.
   */
  public function takeWhile((function(mixed)[_]: bool) $callback)[ctx $callback]:
    /* HH_FIXME[2049] */
    ImmVector<mixed> {
    $pair = $this->toVArray();
    if (!$callback($pair[0])) return \HH\ImmVector {};
    if (!$callback($pair[1])) return \HH\ImmVector { $pair[0] };
    return $this->toImmVector();
  }

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
  /* HH_FIXME[2049] */
  public function skip(int $n)[]: ImmVector<mixed> {
    if (!\is_int($n)) {
      throw new \InvalidArgumentException(
        "Parameter n must be an integer"
      );
    }
    if ($n <= 0) return $this->toImmVector();
    if ($n == 1) return \HH\ImmVector { $this[1] };
    return \HH\ImmVector {};
  }

  /**
   * Returns an `ImmVector` containing the values of the current `Pair` starting
   * after and including the first value that produces `true` when passed to
   * the specified callback.
   *
   * @param $callback - The callback used to determine the starting element for
   *                    the `ImmVector`.
   *
   * @return - An `ImmVector` that contains the values of the current `Pair`
   *           starting after the callback returns `true`.
   */
  public function skipWhile((function(mixed)[_]: bool) $callback)[ctx $callback]:
    /* HH_FIXME[2049] */
    ImmVector<mixed> {
    $pair = $this->toVArray();
    if (!$callback($pair[0])) return $this->toImmVector();
    if (!$callback($pair[1])) return \HH\ImmVector { $pair[1] };
    return \HH\ImmVector {};
  }

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
  <<__Native>>
  public function linearSearch<Tu super mixed>(mixed $search_value)[]: int;

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
  /* HH_FIXME[2049] */
  public function slice(int $start, int $len)[]: ImmVector<mixed> {
    if (!\is_int($start) || ($start < 0)) {
      throw new \InvalidArgumentException(
        "Parameter start must be a non-negative integer"
      );
    }
    if (!\is_int($len) || ($len < 0)) {
      throw new \InvalidArgumentException(
        "Parameter len must be a non-negative integer"
      );
    }
    if (($len == 0) || ($start >= 2)) return \HH\ImmVector {};
    if ($start == 1) return \HH\ImmVector { $this[1] };
    if ($len == 1) return \HH\ImmVector { $this[0] };
    return $this->toImmVector();
  }

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
  public function concat<Tu super mixed>(Traversable<Tu> $traversable)[]:
    /* HH_FIXME[2049] */
    ImmVector<Tu> {
    $ret = vec($this);
    foreach ($traversable as $value) {
      $ret[] = $value;
    }
    /* HH_FIXME[2049] */
    return new ImmVector($ret);
  }

  /**
   * Returns the first value in the current `Pair`.
   *
   * @return - The first value in the current `Pair`.
   */
  public function firstValue()[]: mixed { return $this[0]; }

  /**
   * Returns the first key in the current `Pair`.
   *
   * The return will always be 0 since a `Pair` only has two keys, 0 and 1.
   *
   * @return - 0
   */
  public readonly function firstKey()[]: int { return 0; }

  /**
   * Returns the last value in the current `Pair`.
   *
   * @return - The last value in the current `Pair`.
   */
  public function lastValue()[]: mixed { return $this[1]; }

  /**
   * Returns the last key in the current `Pair`.
   *
   * The return will always be 1 since a `Pair` only has two keys, 0 and 1.
   *
   * @return - 0
   */
  public readonly function lastKey()[]: int { return 1; }

  /**
   * Returns the value at the specified key in the current `Pair`.
   *
   * If the key is not present, an exception is thrown. This essentially means
   * if you specify a key other than 0 or 1, you will get an exception. If you
   * don't want an exception to be thrown, use `get()` instead.
   *
   * $v = $p->at($k)" is semantically equivalent to `$v = $p[$k]`.
   *
   * @param $key - the key from which to retrieve the value.
   *
   * @return - The value at the specified key; or an exception if the key does
   *           not exist.
   */
  <<__Native>>
  public function at(mixed $key)[]: mixed;

  /**
   * Returns the value at the specified key in the current `Pair`.
   *
   * If the key is not present, `null` is returned. If you would rather have an
   * exception thrown when a key is not present, then use `at()`.
   *
   * @param $key - the key from which to retrieve the value.
   *
   * @return - The value at the specified key; or `null` if the key does not
   *           exist.
   */
  <<__Native>>
  public function get(mixed $key)[]: mixed;

  /**
   * Checks whether a provided key exists in the current `Pair`.
   *
   * This will only return `true` for provided keys of 0 and 1 since those are
   * the only two keys that can exist in a `Pair`.
   *
   * @param $k - the key to check existence for in the `Pair`.
   *
   * @return - `true` if the provided key exists in the `Pair`; `false`
   *           otherwise. This will only return `true` if the provided key is
   *           0 or 1.
   */
  public readonly function containsKey<Tu super int>(Tu $key)[]: bool {
    if (!\is_int($key)) {
      throw new \InvalidArgumentException(
        "Only integer keys may be used with Pairs"
      );
    }
    return ($key === 0) || ($key === 1);
  }

  /**
   * Returns the `string` version of the current `Pair`, which is `"Pair"`.
   *
   * @return - The `string` "Pair".
   */
  public function __toString()[]: string { return "Pair"; }
}

} // namespace HH
