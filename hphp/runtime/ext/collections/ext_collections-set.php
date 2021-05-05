<?hh // partial
<<file:__EnableUnstableFeatures('readonly')>>

namespace {

/** An iterator implementation for iterating over a Set.
 */
<<__NativeData("SetIterator")>>
final class SetIterator implements HH\Rx\Iterator {

  public function __construct()[]: void {}

  /** Returns the current value that the iterator points to.
   * @return mixed
   */
  <<__Native>>
  public function current()[]: mixed;

  /** @return mixed
   */
  <<__Native>>
  public function key()[]: mixed;

  /** Returns true if the iterator points to a valid value, returns false
   * otherwise.
   * @return bool
   */
  <<__Native>>
  public function valid()[]: bool;

  /** Advance this iterator forward one position.
   */
  <<__Native>>
  public function next()[write_props]: void;

  /** Move this iterator back to the first position.
   */
  <<__Native>>
  public function rewind()[write_props]: void;
}

} // empty namespace
namespace HH {

/** An ordered set-style collection.
 */
final class Set implements \MutableSet {

  /** Returns a Set built from the values produced by the specified Iterable.
   * @param mixed $iterable
   */
  <<__Native>>
  public function __construct(mixed $iterable = null)[]: void;

  /** Returns true if the Set is empty, false otherwise.
   * @return bool
   */
  public function isEmpty()[]: bool { return !$this->count(); }

  /** Returns the number of values in the Set.
   * @return int
   */
  <<__Native>>
  public function count()[]: int;

  /** Returns an Iterable that produces the values from this Set.
   * @return object
   */
  public function items()[]: \LazyIterableView {
    return new \LazyIterableView($this);
  }


  /** Returns a Vector built from the keys of this Set.
   * @return object
   */
  public function keys()[] { return $this->values(); }

  /** Returns a Vector built from the values of this Set.
   * @return object
   */
  <<__Native>>
  public function values()[]: object;

  /** Returns a lazy iterable view of this Set.
   * @return object
   */
  public function lazy()[]: \LazyKeyedIterableView {
    return new \LazyKeyedIterableView($this);
  }

  /** Removes all values from the Set.
   * @return object
   */
  <<__Native>>
  public function clear()[write_props]: object;

  /** Returns true if the specified value is present in the Set, returns false
   * otherwise.
   * @param mixed $val
   * @return bool
   */
  <<__Native>>
  public readonly function contains(mixed $val)[]: bool;

  /** Removes the specified value from this Set.
   * @param mixed $val
   * @return object
   */
  <<__Native>>
  public function remove(mixed $val)[write_props]: object;

  /** Adds the specified value to this Set.
   * @param mixed $val
   * @return object
   */
  <<__Native>>
  public function add(mixed $val)[write_props]: object;

  /** Adds the values produced by the specified Iterable to this Set.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function addAll(mixed $iterable)[write_props]: object;

  /** Adds the keys of the specified KeyedContainer to this Set.
   * @param mixed $container
   * @return object
   */
  <<__Native>>
  public function addAllKeysOf(mixed $container)[write_props]: object;

  /** Instructs this Set to grow its capacity to accommodate the given number of
   * elements. The caller is expected to make the appropriate add/addAll calls
   * to fill that reserved capacity.
   * @param mixed $sz
   */
  <<__Native>>
  public function reserve(int $sz)[]: void;

  <<__Native>>
  public function toVArray()[]: varray;

  /** Returns a darray built from the values from this Set, darray[val1 => val1,
   * val2 => val2, ...]. This maintains set-like semantics in darray[] land:
   * O(1) membership test with `array_has_key($a['key'])` and iteration with
   *  `foreach($a as $member)`.
   * @return darray
   */
  <<__Native>>
  public function toDArray()[]: darray;

  /** Returns a Vector built from the values of this Set.
   * @return object
   */
  <<__Native>>
  public function toVector()[]: object;

  /** Returns a ImmVector built from the values of this Set.
   * @return object
   */
  <<__Native>>
  public function toImmVector()[]: object;

  /** Returns a Map built from the keys and values of this Set.
   * @return object
   */
  <<__Native>>
  public function toMap()[]: object;

  /** Returns a ImmMap built from the keys and values of this Set.
   * @return object
   */
  <<__Native>>
  public function toImmMap()[]: object;

  /** Returns a copy of this Set.
   * @return object
   */
  public function toSet()[]: this {
    return new self($this);
  }

  /** Returns a ImmSet built from the values of this Set.
   * @return object
   */
  <<__Native>>
  public function toImmSet()[]: object;

  /** Returns an immutable version of this collection.
   * @return object
   */
  public function immutable()[] { return $this->toImmSet(); }

  /** Returns a varray built from the values from this Set.
   * @return varray
   */
  <<__Native>>
  public function toKeysArray()[]: varray;

  /** Returns a varray built from the values from this Set.
   * @return varray
   */
  <<__Native>>
  public function toValuesArray()[]: varray;

  /** Returns an iterator that points to beginning of this Set.
   * @return object
   */
  <<__Native>>
  public function getIterator()[]: object;

  /** Returns a Set of the values produced by applying the specified callback on
   * each value from this Set.
   * @param mixed $callback
   * @return object
   */
  public function map((function()[_]: void) $callback)[ctx $callback]: \HH\Set {
    $ret = new \HH\Set();
    foreach ($this as $v) {
      $ret[] = $callback($v);
    }
    return $ret;
  }

  /** Returns a Set of the values produced by applying the specified callback on
   * each key and value from this Set.
   * @param mixed $callback
   * @return object
   */
  public function mapWithKey((function()[_]: void) $callback)[ctx $callback]: \HH\Set {
    $ret = new \HH\Set();
    foreach ($this as $k => $v) {
      $ret[] = $callback($k, $v);
    }
    return $ret;
  }

  /** Returns a Set of all the values from this Set for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  public function filter((function()[_]: void) $callback)[ctx $callback]: \HH\Set {
    $ret = new \HH\Set();
    foreach ($this as $v) {
      if ($callback($v)) {
        $ret[] = $v;
      }
    }
    return $ret;
  }

  /** Returns a Set of all the values from this Set for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  public function filterWithKey((function()[_]: void) $callback)[ctx $callback]: \HH\Set {
    $ret = new \HH\Set();
    foreach ($this as $k => $v) {
      if ($callback($k, $v)) {
        $ret[] = $v;
      }
    }
    return $ret;
  }

  /** Ensures that this Set contains only values for which the specified callback
   * returns true.
   * @param mixed $callback
   * @return object
   */
  public function retain((function()[_]: void) $callback)[ctx $callback]: \HH\Set {
    foreach ($this as $k => $v) {
      if (!$callback($v)) {
        $this->remove($k);
      }
    }
    return $this;
  }

  /** Ensures that this Set contains only keys/values for which the specified
   * callback returns true when passed the key and the value.
   * @param mixed $callback
   * @return object
   */
  public function retainWithKey((function()[_]: void) $callback)[ctx $callback]: \HH\Set {
    foreach ($this as $k => $v) {
      if (!$callback($k, $v)) {
        $this->remove($k);
      }
    }
    return $this;
  }

  /** Returns a Iterable produced by combined the specified Iterables pair-wise.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function zip(mixed $iterable)[]: object;

  /** Returns a Set containing the first n values of this Set.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function take(mixed $n)[]: object;

  /** Returns a Set containing the values of this Set up to but not including the
   * first value that produces false when passed to the specified callback.
   * @param mixed $callback
   * @return object
   */
  public function takeWhile((function()[_]: void) $callback)[ctx $callback]: \HH\Set {
    $ret = new \HH\Set();
    foreach ($this as $v) {
      if (!$callback($v)) {
        break;
      }
      $ret[] = $v;
    }
    return $ret;
  }

  /** Returns a Set containing all values except the first n of this Set.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function skip(mixed $n)[]: object;

  /** Returns a Set containing all the values of this Set excluding the first
   * values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  public function skipWhile((function()[_]: void) $fn)[ctx $fn]: \HH\Set {
    $ret = new \HH\Set();
    $skipping = true;
    foreach ($this as $v) {
      if ($skipping) {
        if ($fn($v)) {
          continue;
        }
        $skipping = false;
      }
      $ret[] = $v;
    }
    return $ret;
  }

  /** Returns a Set containing the specified range of values from this Set. The
   * range is specified by two non-negative integers: a starting position and a
   * length.
   * @param mixed $start
   * @param mixed $len
   * @return object
   */
  <<__Native>>
  public function slice(mixed $start,
                        mixed $len)[]: object;

  /** Builds a new Vector by concatenating the elements of this Set with the
   * elements of the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function concat(mixed $iterable)[]: object;

  /** Returns the first value from this Set, or null if this Set is empty.
   * @return mixed
   */
  <<__Native>>
  public function firstValue()[]: mixed;

  /** Returns the first key from this Set, or null if this Vector is empty.
   * @return mixed
   */
  public readonly function firstKey()[] {
    return $this->firstValue();
  }

  /** Returns the last value from this Set, or null if this Set is empty.
   * @return mixed
   */
  <<__Native>>
  public function lastValue()[]: mixed;

  /** Returns the last key from this Set, or null if this Set is empty.
   * @return mixed
   */
  public readonly function lastKey()[] {
    return $this->lastValue();
  }

  /** @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function removeAll(mixed $iterable)[write_props]: object;

  /** @param mixed $iterable
   * @return object
   */
  public function difference(mixed $iterable) {
    // This isn't really a difference method, it lies
    return $this->removeAll($iterable);
  }

  /** @return string
   */
  public function __toString()[]: string { return "Set"; }

  /** Returns a Set built from the values produced by the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public static function fromItems(mixed $iterable)[]: object;

  /** Returns a Set built from the keys of the specified container.
   * @param mixed $container
   * @return object
   */
  <<__Native>>
  public static function fromKeysOf(mixed $container)[]: object;

  /** Returns a Set built from the values from the specified array.
   * @param mixed $arr
   * @return object
   */
  <<__Native>>
  public static function fromArray(mixed $arr): object;

  /** Returns a Set built from the values from the specified arrays.
   * @return object
   */
  public static function fromArrays(...$argv)[] {
    if (!$argv) return \HH\Set {};
    $ret = \HH\Set {};
    foreach ($argv as $arr) {
      if (!\HH\is_any_array($arr)) {
        throw new \InvalidArgumentException("Parameters must be array-likes");
      }
      $ret->addAll($arr);
    }
    return $ret;
  }
}

/** An immutable ordered set-style collection.
 */
final class ImmSet implements \ConstSet {

  /** Returns a ImmSet built from the values produced by the specified Iterable.
   * @param mixed $iterable
   */
  <<__Native>>
  public function __construct(mixed $iterable = null)[]: void;

  /** Returns true if the ImmSet is empty, false otherwise.
   * @return bool
   */
  public function isEmpty()[]: bool { return !$this->count(); }

  /** Returns the number of values in the ImmSet.
   * @return int
   */
  <<__Native>>
  public function count()[]: int;

  /** Returns an Iterable that produces the values from this ImmSet.
   * @return object
   */
  public function items()[]: \LazyIterableView {
    return new \LazyIterableView($this);
  }


  /** Returns a Vector built from the keys of this ImmSet.
   * @return object
   */
  public function keys()[] { return $this->values(); }

  /** Returns a ImmVector built from the values of this ImmSet.
   * @return object
   */
  <<__Native>>
  public function values()[]: object;

  /** Returns a lazy iterable view of this ImmSet.
   * @return object
   */
  public function lazy()[]: \LazyKeyedIterableView {
    return new \LazyKeyedIterableView($this);
  }

  /** Returns true if the specified value is present in the ImmSet, returns false
   * otherwise.
   * @param mixed $val
   * @return bool
   */
  <<__Native>>
  public function contains(mixed $val)[]: bool;

  <<__Native>>
  public function toVArray()[]: varray;

  /** Returns a darray built from the values from this ImmSet, darray[val1 =>
   * val1, val2 => val2, ...]. This maintains set-like semantics in darray[]
   * land: O(1) membership test with `array_has_key($a['key'])` and iteration
   * with `foreach($a as $member)`.
   * @return darray
   */
  <<__Native>>
  public function toDArray()[]: darray;

  /** Returns a Vector built from the values of this ImmSet.
   * @return object
   */
  <<__Native>>
  public function toVector()[]: object;

  /** Returns a ImmVector built from the values of this ImmSet.
   * @return object
   */
  <<__Native>>
  public function toImmVector()[]: object;

  /** Returns a Map built from the keys and values of this ImmSet.
   * @return object
   */
  <<__Native>>
  public function toMap()[]: object;

  /** Returns a ImmMap built from the keys and values of this ImmSet.
   * @return object
   */
  <<__Native>>
  public function toImmMap()[]: object;

  /** Returns a Set built from the values of this ImmSet.
   * @return object
   */
  <<__Native>>
  public function toSet()[]: object;

  /** Returns an immutable version of this collection.
   * @return object
   */
  public function toImmSet()[]: this {
    return $this;
  }

  /** Returns an immutable version of this collection.
   * @return object
   */
  public function immutable()[]: this {
    return $this;
  }

  /** Returns a varray built from the values from this ImmSet.
   * @return varray
   */
  <<__Native>>
  public function toKeysArray()[]: varray;

  /** Returns a varray built from the values from this ImmSet.
   * @return varray
   */
  <<__Native>>
  public function toValuesArray()[]: varray;

  /** Returns an iterator that points to beginning of this ImmSet.
   * @return object
   */
  <<__Native>>
  public function getIterator()[]: object;

  /** Returns a ImmSet of the values produced by applying the specified callback
   * on each value from this ImmSet.
   * @param mixed $callback
   * @return object
   */
  public function map((function()[_]: void) $callback)[ctx $callback]: \HH\ImmSet {
    $ret = new \HH\Set();
    foreach ($this as $v) {
      $ret[] = $callback($v);
    }
    return new \HH\ImmSet($ret);
  }

  /** Returns a ImmSet of the values produced by applying the specified callback
   * on each key and value from this ImmSet.
   * @param mixed $callback
   * @return object
   */
  public function mapWithKey((function()[_]: void) $callback)[ctx $callback]: \HH\ImmSet {
    $ret = new \HH\Set();
    foreach ($this as $k => $v) {
      $ret[] = $callback($k, $v);
    }
    return new \HH\ImmSet($ret);
  }

  /** Returns a ImmSet of all the values from this ImmSet for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  public function filter((function()[_]: void) $callback)[ctx $callback]: \HH\ImmSet {
    $ret = new \HH\Set();
    foreach ($this as $v) {
      if ($callback($v)) {
        $ret[] = $v;
      }
    }
    return new \HH\ImmSet($ret);
  }

  /** Returns a ImmSet of all the values from this ImmSet for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  public function filterWithKey((function()[_]: void) $callback)[ctx $callback]: \HH\ImmSet {
    $ret = new \HH\Set();
    foreach ($this as $k => $v) {
      if ($callback($k, $v)) {
        $ret[] = $v;
      }
    }
    return new \HH\ImmSet($ret);
  }

  /** Returns an Iterable produced by combining the specified Iterables
   * pair-wise.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function zip(mixed $iterable)[]: object;

  /** Returns a ImmSet containing the first n values of this ImmSet.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function take(mixed $n)[]: object;

  /** Returns a ImmSet containing the values of this ImmSet up to but not
   * including the first value that produces false when passed to the specified
   * callback.
   * @param mixed $callback
   * @return object
   */
  public function takeWhile((function()[_]: void) $callback)[ctx $callback]: \HH\ImmSet {
    $ret = new \HH\Set();
    foreach ($this as $v) {
      if (!$callback($v)) {
        break;
      }
      $ret[] = $v;
    }
    return new \HH\ImmSet($ret);
  }

  /** Returns a ImmSet containing all values except the first n of this ImmSet.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function skip(mixed $n)[]: object;

  /** Returns a ImmSet containing all the values of this ImmSet excluding the
   * first values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  public function skipWhile((function()[_]: void) $fn)[ctx $fn]: \HH\ImmSet {
    $ret = new \HH\Set();
    $skipping = true;
    foreach ($this as $v) {
      if ($skipping) {
        if ($fn($v)) {
          continue;
        }
        $skipping = false;
      }
      $ret[] = $v;
    }
    return new \HH\ImmSet($ret);
  }

  /** Returns a ImmSet containing the specified range of values from this ImmSet.
   * The range is specified by two non-negative integers: a starting position
   * and a length.
   * @param mixed $start
   * @param mixed $len
   * @return object
   */
  <<__Native>>
  public function slice(mixed $start,
                        mixed $len)[]: object;

  /** Builds a new ImmVector by concatenating the elements of this ImmSet with
   * the elements of the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function concat(mixed $iterable)[]: object;

  /** Returns the first value from this ImmSet, or null if this ImmSet is empty.
   * @return mixed
   */
  <<__Native>>
  public function firstValue()[]: mixed;

  /** Returns the first key from this ImmSet, or null if this ImmSet is empty.
   * @return mixed
   */
  public readonly function firstKey()[] {
    return $this->firstValue();
  }

  /** Returns the last value from this ImmSet, or null if this ImmSet is empty.
   * @return mixed
   */
  <<__Native>>
  public function lastValue()[]: mixed;

  /** Returns the last key from this ImmSet, or null if this ImmSet is empty.
   * @return mixed
   */
  public readonly function lastKey()[] {
    return $this->lastValue();
  }

  /** @return string
   */
  public function __toString()[]: string { return "ImmSet"; }

  /** Returns a ImmSet built from the values produced by the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public static function fromItems(mixed $iterable)[]: object;

  /** Returns a ImmSet built from the keys of the specified container.
   * @param mixed $container
   * @return object
   */
  <<__Native>>
  public static function fromKeysOf(mixed $container)[]: object;

  /** Returns a ImmSet built from the values from the specified arrays.
   * @return object
   */
  public static function fromArrays(...$argv)[] {
    if (!$argv) return \HH\ImmSet {};
    $ret = \HH\Set {};
    foreach ($argv as $arr) {
      if (!\HH\is_any_array($arr)) {
        throw new \InvalidArgumentException("Parameters must be array-likes");
      }
      $ret->addAll($arr);
    }
    return $ret->toImmSet();
  }
}

} // namespace HH
