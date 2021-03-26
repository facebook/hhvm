<?hh // partial

namespace {

/** An iterator implementation for iterating over a Map.
 */
<<__NativeData("MapIterator")>>
final class MapIterator implements HH\Rx\KeyedIterator {

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

} // empty namesapce
namespace HH {

/** An ordered dictionary-style collection.
 */
final class Map implements \MutableMap {

  /** Returns a Map built from the keys and values produced by the specified
   * KeyedIterable.
   * @param mixed $iterable
   */
  <<__Native>>
  public function __construct(mixed $iterable = null)[]: void;

  /** Returns true if the Map is empty, false otherwise.
   * @return bool
   */
  public function isEmpty()[]: bool {
    return !$this->count();
  }

  /** Returns the number of key/value pairs in the Map.
   * @return int
   */
  <<__Native>>
  public function count()[]: int;

  /** Returns an Iterable that produces the key/value Pairs from this Map.
   * @return object
   */
  public function items()[]: \LazyKVZipIterable {
    return new \LazyKVZipIterable($this);
  }

  /** Returns a Vector built from the keys of this Map.
   * @return object
   */
  <<__Native>>
  public function keys()[]: object;

  /** Returns a lazy iterable view of this Map.
   * @return object
   */
  public function lazy()[]: \LazyKeyedIterableView {
    return new \LazyKeyedIterableView($this);
  }

  /** Returns the value at the specified key. If the key is not present, an
   * exception is thrown.
   * @param mixed $key
   * @return mixed
   */
  <<__Native>>
  public function at(mixed $key)[]: mixed;

  /** Returns the value at the specified key. If the key is not present, null is
   * returned.
   * @param mixed $key
   * @return mixed
   */
  <<__Native>>
  public function get(mixed $key)[]: mixed;

  /** Stores a value into the Map with the specified key, overwriting any
   * previous value that was associated with the key.
   * @param mixed $key
   * @param mixed $value
   * @return object
   */
  <<__Native>>
  public function set(mixed $key,
                      mixed $value)[write_props]: object;

  /** Stores each value produced by the specified KeyedIterable into the Map
   * using its corresponding key, overwriting any previous value that was
   * associated with that key.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function setAll(mixed $iterable)[write_props]: object;

  /** Removes all key/value pairs from the Map.
   * @return object
   */
  <<__Native>>
  public function clear()[write_props]: object;

  /** Returns true if the specified key is present in the Map, false otherwise.
   * @param mixed $key
   * @return bool
   */
  <<__Native>>
  public function contains(mixed $key)[]: bool;

  /** Returns true if the specified key is present in the Map, false otherwise.
   * @param mixed $key
   * @return bool
   */
  <<__Native>>
  public function containsKey(mixed $key)[]: bool;

  /** Removes the specified key from this Map.
   * @param mixed $key
   * @return object
   */
  public function remove(mixed $key)[] {
    return $this->removeKey($key);
  }

  /** Removes the specified key from this Map.
   * @param mixed $key
   * @return object
   */
  <<__Native>>
  public function removeKey(mixed $key)[]: object;

  /** Adds the specified key/value Pair to this Map. If an element with the same
   * key is already present, an exception is thrown.
   * @param mixed $val
   * @return object
   */
  <<__Native>>
  public function add(mixed $val)[write_props]: object;

  /** Adds the key/value Pairs produced by the specified Iterable to this Map.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function addAll(mixed $iterable)[write_props]: object;

  /** Instructs this Map to grow its capacity to accommodate the given number of
   * elements. The caller is expected to make the appropriate
   * add/set/addAll/setAll calls to fill this reserved capacity.
   * @param mixed $sz
   */
  <<__Native>>
  public function reserve(int $sz)[]: void;

  <<__Native>>
  public function toVArray()[]: varray;

  /** Returns a darray built from the keys and values from this Map.
   * @return darray
   */
  <<__Native>>
  public function toDArray()[]: darray;

  /** Returns a Vector built from the values of this Map.
   * @return object
   */
  <<__Native>>
  public function toVector()[]: object;

  /** Returns a ImmVector built from the values of this Map.
   * @return object
   */
  <<__Native>>
  public function toImmVector()[]: object;

  /** Returns a copy of this Map.
   * @return object
   */
  public function toMap()[]: this {
    return new self($this);
  }

  /** Returns a ImmMap built from the keys and values of this Map.
   * @return object
   */
  <<__Native>>
  public function toImmMap()[]: object;

  /** Returns a Set built from the values of this Map.
   * @return object
   */
  <<__Native>>
  public function toSet()[]: object;

  /** Returns a ImmSet built from the values of this Map.
   * @return object
   */
  <<__Native>>
  public function toImmSet()[]: object;

  /** Returns an immutable version of this collection.
   * @return object
   */
  public function immutable()[]: \HH\ImmMap {
    return $this->toImmMap();
  }

  /** Returns a Vector built from the values of this Map.
   * @return object
   */
  <<__Native>>
  public function values()[]: object;

  /** Returns a varray built from the keys from this Map.
   * @return varray
   */
  <<__Native>>
  public function toKeysArray()[]: varray;

  /** Returns a varray built from the values from this Map.
   * @return varray
   */
  <<__Native>>
  public function toValuesArray()[]: varray;

  /** @param mixed $it
   * @return object
   */
  <<__Native>>
  public function differenceByKey(mixed $it)[]: object;

  /** Returns an iterator that points to beginning of this Map.
   * @return object
   */
  <<__Native>>
  public function getIterator()[]: object;

  /** Returns a Map of the keys/values produced by applying the specified
   * callback on each value from this Map.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable, __ProvenanceSkipFrame>>
  public function map(<<__AtMostRxAsFunc>> mixed $callback): \HH\Map {
    $res = new \HH\Map($this);
    foreach ($this as $k => $v) {
      $res[$k] = $callback($v);
    }
    return $res;
  }

  /** Returns a Map of the keys/values produced by applying the specified
   * callback on each key and value from this Map.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable, __ProvenanceSkipFrame>>
  public function mapWithKey(<<__AtMostRxAsFunc>> mixed $callback): \HH\Map {
    $res = new \HH\Map($this);
    foreach ($this as $k => $v) {
      $res[$k] = $callback($k, $v);
    }
    return $res;
  }

  /** Returns a new Map of all the keys/values from this Map for which the
   * specified callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filter(<<__AtMostRxAsFunc>> mixed $callback): \HH\Map {
    $res = new \HH\Map();
    foreach ($this as $k => $v) {
      if ($callback($v)) {
        $res[$k] = $v;
      }
    }
    return $res;
  }

  /** Returns a new Map of all the keys/values from this Map for which the
   * specified callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filterWithKey(<<__AtMostRxAsFunc>> mixed $callback): \HH\Map {
    $res = new \HH\Map();
    foreach ($this as $k => $v) {
      if ($callback($k, $v)) {
        $res[$k] = $v;
      }
    }
    return $res;
  }

  /** Ensures that this Map contains only keys/values for which the specified
   * callback returns true when passed the value.
   * @param mixed $callback
   * @return object
   */
  public function retain(mixed $callback): \HH\Map {
    foreach ($this as $k => $v) {
      if (!$callback($v)) {
        $this->removeKey($k);
      }
    }
    return $this;
  }

  /** Ensures that this Map contains only keys/values for which the specified
   * callback returns true when passed the key and the value.
   * @param mixed $callback
   * @return object
   */
  public function retainWithKey(mixed $callback): \HH\Map {
    foreach ($this as $k => $v) {
      if (!$callback($k, $v)) {
        $this->removeKey($k);
      }
    }
    return $this;
  }

  /** Returns a KeyedIterable produced by combined the specified Iterables
   * pair-wise.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function zip(mixed $iterable)[]: object;

  /** Returns a Map containing the first n key/value pairs of this Map.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function take(mixed $n)[]: object;

  /** Returns a Map containing the key/value pairs of this Map up to but not
   * including the first value that produces false when passed to the specified
   * callback.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function takeWhile(<<__AtMostRxAsFunc>> mixed $callback): \HH\Map {
    $res = new \HH\Map();
    foreach ($this as $k => $v) {
      if (!$callback($v)) {
        break;
      }
      $res[$k] = $v;
    }
    return $res;
  }

  /** Returns a Map containing all key/value pairs except the first n of this
   * Map.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function skip(mixed $n)[]: object;

  /** Returns a Map containing the key/value pairs of this Map excluding the
   * first values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function skipWhile(<<__AtMostRxAsFunc>> mixed $fn): \HH\Map {
    $ret = dict[];
    $skipping = true;
    foreach ($this as $k => $v) {
      if ($skipping) {
        if ($fn($v)) {
          continue;
        }
        $skipping = false;
      }
      $ret[$k] = $v;
    }
    return new \HH\Map($ret);
  }

  /** Returns a Map containing the specified range of key/value pairs from this
   * Map. The range is specified by two non-negative integers: a starting
   * position and a length.
   * @param mixed $start
   * @param mixed $len
   * @return object
   */
  <<__Native>>
  public function slice(mixed $start,
                        mixed $len)[]: object;

  /** Builds a new Vector by concatenating the values of this Map with the
   * elements of the specified Iterable. Note that this ignores the keys of this
   * Map and the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function concat(mixed $iterable)[]: object;

  /** Returns the first value from this Map, or null if this Map is empty.
   * @return mixed
   */
  <<__Native>>
  public function firstValue()[]: mixed;

  /** Returns the first key from this Map, or null if this Map is empty.
   * @return mixed
   */
  <<__Native>>
  public function firstKey()[]: mixed;

  /** Returns the last value from this Map, or null if this Map is empty.
   * @return mixed
   */
  <<__Native>>
  public function lastValue()[]: mixed;

  /** Returns the last key from this Map, or null if this Map is empty.
   * @return mixed
   */
  <<__Native>>
  public function lastKey()[]: mixed;

  /** @return string
   */
  public function __toString()[]: string { return "Map"; }

  /** Returns a Map built from the key/value Pairs produced by the specified
   * Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public static function fromItems(mixed $iterable)[]: object;

  /** Returns a Map built from the keys and values from the specified array.
   * @param mixed $mp
   * @return object
   */
  <<__Native>>
  public static function fromArray(mixed $mp): object;
}

/** An immutable ordered dictionary-style collection.
 */
final class ImmMap implements \ConstMap {

  /** Returns a ImmMap built from the keys and values produced by the specified
   * KeyedIterable.
   * @param mixed $iterable
   */
  <<__Native>>
  public function __construct(mixed $iterable = null)[]: void;

  /** Returns true if the ImmMap is empty, false otherwise.
   * @return bool
   */
  public function isEmpty()[]: bool {
    return !$this->count();
  }

  /** Returns the number of key/value pairs in the ImmMap.
   * @return int
   */
  <<__Native>>
  public function count()[]: int;

  /** Returns an Iterable that produces the key/value Pairs from this ImmMap.
   * @return object
   */
  public function items()[]: \LazyKVZipIterable {
    return new \LazyKVZipIterable($this);
  }

  /** Returns a ImmVector built from the keys of this ImmMap.
   * @return object
   */
  <<__Native>>
  public function keys()[]: object;

  /** Returns a lazy iterable view of this ImmMap.
   * @return object
   */
  public function lazy()[]: \LazyKeyedIterableView {
    return new \LazyKeyedIterableView($this);
  }

  /** Returns the value at the specified key. If the key is not present, an
   * exception is thrown.
   * @param mixed $key
   * @return mixed
   */
  <<__Native>>
  public function at(mixed $key)[]: mixed;

  /** Returns the value at the specified key. If the key is not present, null is
   * returned.
   * @param mixed $key
   * @return mixed
   */
  <<__Native>>
  public function get(mixed $key)[]: mixed;

  /** Returns true if the specified key is present in the ImmMap, false
   * otherwise.
   * @param mixed $key
   * @return bool
   */
  <<__Native>>
  public function contains(mixed $key)[]: bool;

  /** Returns true if the specified key is present in the ImmMap, false
   * otherwise.
   * @param mixed $key
   * @return bool
   */
  <<__Native>>
  public function containsKey(mixed $key)[]: bool;

  <<__Native>>
  public function toVArray()[]: varray;

  /** Returns a darray built from the keys and values from this ImmMap.
   * @return darray
   */
  <<__Native>>
  public function toDArray()[]: darray;

  /** Returns a Vector built from the values of this ImmMap.
   * @return object
   */
  <<__Native, __Pure, __MutableReturn, __MaybeMutable>>
  public function toVector(): object;

  /** Returns a ImmVector built from the values of this ImmMap.
   * @return object
   */
  <<__Native>>
  public function toImmVector()[]: object;

  /** Returns a Map built from the keys and values of this ImmMap.
   * @return object
   */
  <<__Native, __Pure, __MutableReturn, __MaybeMutable>>
  public function toMap(): object;

  /** Returns an immutable version of this collection.
   * @return object
   */
  public function toImmMap()[]: this {
    return $this;
  }

  /** Returns a Set built from the values of this ImmMap.
   * @return object
   */
  <<__Native, __Pure, __MutableReturn, __MaybeMutable>>
  public function toSet(): object;

  /** Returns a ImmSet built from the values of this ImmMap.
   * @return object
   */
  <<__Native>>
  public function toImmSet()[]: object;

  /** Returns an immutable version of this collection.
   * @return object
   */
  public function immutable()[]: this {
    return $this;
  }

  /** Returns a ImmVector built from the values of this ImmMap.
   * @return object
   */
  <<__Native>>
  public function values()[]: object;

  /** Returns a varray built from the keys from this ImmMap.
   * @return varray
   */

  <<__Native>>
  public function toKeysArray()[]: varray;

  /** Returns a varray built from the values from this ImmMap.
   * @return varray
   */
  <<__Native>>
  public function toValuesArray()[]: varray;

  /** @param mixed $it
   * @return object
   */
  <<__Native>>
  public function differenceByKey(mixed $it)[]: object;

  /** Returns an iterator that points to beginning of this ImmMap.
   * @return object
   */
  <<__Native>>
  public function getIterator()[]: object;

  /** Returns a ImmMap of the keys/values produced by applying the specified
   * callback on each value from this ImmMap.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable, __ProvenanceSkipFrame>>
  public function map(<<__AtMostRxAsFunc>> mixed $callback): \HH\ImmMap {
    $res = $this->toMap();
    foreach ($this as $k => $v) {
      $res[$k] = $callback($v);
    }
    return $res->toImmMap();
  }

  /** Returns a ImmMap of the keys/values produced by applying the specified
   * callback on each key and value from this ImmMap.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable, __ProvenanceSkipFrame>>
  public function mapWithKey(<<__AtMostRxAsFunc>> mixed $callback): \HH\ImmMap {
    $res = $this->toMap();
    foreach ($this as $k => $v) {
      $res[$k] = $callback($k, $v);
    }
    return $res->toImmMap();
  }

  /** Returns a ImmMap of all the keys/values from this ImmMap for which the
   * specified callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filter(<<__AtMostRxAsFunc>> mixed $callback): \HH\ImmMap {
    $res = new \HH\Map();
    foreach ($this as $k => $v) {
      if ($callback($v)) {
        $res[$k] = $v;
      }
    }
    return $res->toImmMap();
  }

  /** Returns a ImmMap of all the keys/values from this ImmMap for which the
   * specified callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filterWithKey(<<__AtMostRxAsFunc>> mixed $callback): \HH\ImmMap {
    $res = new \HH\Map();
    foreach ($this as $k => $v) {
      if ($callback($k, $v)) {
        $res[$k] = $v;
      }
    }
    return $res->toImmMap();
  }

  /** Returns a KeyedIterable produced by combined the specified Iterables
   * pair-wise.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function zip(mixed $iterable)[]: object;

  /** Returns a ImmMap containing the first n key/value pairs of this ImmMap.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function take(mixed $n)[]: object;

  /** Returns a ImmMap containing the key/value pairs of this ImmMap up to but
   * not including the first value that produces false when passed to the
   * specified callback.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function takeWhile(<<__AtMostRxAsFunc>> mixed $callback): \HH\ImmMap {
    $res = new \HH\Map();
    foreach ($this as $k => $v) {
      if (!$callback($v)) {
        break;
      }
      $res[$k] = $v;
    }
    return $res->toImmMap();
  }

  /** Returns a ImmMap containing all key/value pairs except the first n of this
   * ImmMap.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function skip(mixed $n)[]: object;

  /** Returns a ImmMap containing the key/value pairs of this ImmMap excluding
   * the first values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function skipWhile(<<__AtMostRxAsFunc>> mixed $fn): \HH\ImmMap {
    $ret = dict[];
    $skipping = true;
    foreach ($this as $k => $v) {
      if ($skipping) {
        if ($fn($v)) {
          continue;
        }
        $skipping = false;
      }
      $ret[$k] = $v;
    }
    return new \HH\ImmMap($ret);
  }

  /** Returns a ImmMap containing the specified range of key/value pairs from
   * this ImmMap. The range is specified by two non-negative integers: a
   * starting position and a length.
   * @param mixed $start
   * @param mixed $len
   * @return object
   */
  <<__Native>>
  public function slice(mixed $start,
                        mixed $len)[]: object;

  /** Builds a new ImmVector by concatenating the values of this ImmMap with the
   * elements of the specified Iterable. Note that this ignores the keys of this
   * ImmMap and the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function concat(mixed $iterable)[]: object;

  /** Returns the first value from this ImmMap, or null if this ImmMap is empty.
   * @return mixed
   */
  <<__Native>>
  public function firstValue()[]: mixed;

  /** Returns the first key from this ImmMap, or null if this ImmMap is empty.
   * @return mixed
   */
  <<__Native>>
  public function firstKey()[]: mixed;

  /** Returns the last value from this ImmMap, or null if this ImmMap is empty.
   * @return mixed
   */
  <<__Native>>
  public function lastValue()[]: mixed;

  /** Returns the last key from this ImmMap, or null if this ImmMap is empty.
   * @return mixed
   */
  <<__Native>>
  public function lastKey()[]: mixed;

  /** @return string
   */
  public function __toString()[]: string { return "ImmMap"; }

  /** Returns a ImmMap built from the key/value Pairs produced by the specified
   * Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public static function fromItems(mixed $iterable)[]: object;
}

} // namespace HH
