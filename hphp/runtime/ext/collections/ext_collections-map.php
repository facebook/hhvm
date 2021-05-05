<?hh // partial
<<file:__EnableUnstableFeatures('readonly')>>

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
  public readonly function isEmpty()[]: bool {
    return !$this->count();
  }

  /** Returns the number of key/value pairs in the Map.
   * @return int
   */
  <<__Native>>
  public readonly function count()[]: int;

  /** Returns an Iterable that produces the key/value Pairs from this Map.
   * @return object
   */
  public function items()[]: \LazyKVZipIterable {
    return new \LazyKVZipIterable($this);
  }

  /** Returns a Vector built from the keys of this Map.
   * @return Vector
   */
  public readonly function keys()[]: \HH\Vector {
    $res = vec[];
    foreach (dict($this) as $k => $_) {
      $res[] = $k;
    }
    return new \HH\Vector($res);
  }

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
   * @param ?KeyedTraversable $iterable
   * @return Map
   */
  public function setAll(?\HH\KeyedTraversable $iterable)[write_props]: \HH\Map {
    if ($iterable is null) return $this;

    foreach ($iterable as $k => $v) {
      $this->set($k, $v);
    }
    return $this;
  }

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
  public readonly function contains(mixed $key)[]: bool;

  /** Returns true if the specified key is present in the Map, false otherwise.
   * @param mixed $key
   * @return bool
   */
  <<__Native>>
  public readonly function containsKey(mixed $key)[]: bool;

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

  public function toVArray()[]: varray {
    return $this->toValuesArray();
  }

  /** Returns a darray built from the keys and values from this Map.
   * @return darray
   */
  public function toDArray()[]: darray {
    return dict($this);
  }

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
   * @return Vector
   */
  public function values()[]: \HH\Vector {
    $res = vec(dict($this));
    return new \HH\Vector($res);
  }

  /** Returns a varray built from the keys from this Map.
   * @return varray
   */
  public function toKeysArray()[]: varray {
    $keys = varray[];
    foreach (dict($this) as $k => $_) {
      $keys[] = $k;
    }
    return $keys;
  }

  /** Returns a varray built from the values from this Map.
   * @return varray
   */
  public function toValuesArray()[]: varray {
    return vec(dict($this));
  }

  /** @param mixed $it
   * @return Map
   */
  public function differenceByKey(\HH\KeyedTraversable $it)[]: \HH\Map {
    $res = dict($this);
    foreach ($it as $k => $_) {
      if (\array_key_exists($k, $res)) {
        unset($res[$k]);
      }
    }
    return new \HH\Map($res);
  }

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
  public function map((function()[_]: void) $callback)[ctx $callback]: \HH\Map {
    $res = new \HH\Map($this);
    foreach (dict($this) as $k => $v) {
      $res[$k] = $callback($v);
    }
    return $res;
  }

  /** Returns a Map of the keys/values produced by applying the specified
   * callback on each key and value from this Map.
   * @param mixed $callback
   * @return object
   */
  public function mapWithKey((function()[_]: void) $callback)[ctx $callback]: \HH\Map {
    $res = new \HH\Map($this);
    foreach (dict($this) as $k => $v) {
      $res[$k] = $callback($k, $v);
    }
    return $res;
  }

  /** Returns a new Map of all the keys/values from this Map for which the
   * specified callback returns true.
   * @param mixed $callback
   * @return object
   */
  public function filter((function()[_]: void) $callback)[ctx $callback]: \HH\Map {
    $res = dict[];
    foreach (dict($this) as $k => $v) {
      if ($callback($v)) {
        $res[$k] = $v;
      }
    }
    return new \HH\Map($res);
  }

  /** Returns a new Map of all the keys/values from this Map for which the
   * specified callback returns true.
   * @param mixed $callback
   * @return object
   */
  public function filterWithKey((function()[_]: void) $callback)[ctx $callback]: \HH\Map {
    $res = dict[];
    foreach (dict($this) as $k => $v) {
      if ($callback($k, $v)) {
        $res[$k] = $v;
      }
    }
    return new \HH\Map($res);
  }

  /** Ensures that this Map contains only keys/values for which the specified
   * callback returns true when passed the value.
   * @param mixed $callback
   * @return object
   */
  public function retain(mixed $callback): \HH\Map {
    foreach (dict($this) as $k => $v) {
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
    foreach (dict($this) as $k => $v) {
      if (!$callback($k, $v)) {
        $this->removeKey($k);
      }
    }
    return $this;
  }

  /** Returns a KeyedIterable produced by combined the specified Iterables
   * pair-wise.
   * @param Traversable $iterable
   * @return Map
   */
  public function zip(\HH\Traversable $iterable)[]: \HH\Map {
    $res = dict[];
    $it = $this->getIterator();
    foreach ($iterable as $v) {
      if (!$it->valid()) break;
      $res[$it->key()] = Pair { $it->current(), $v };
      $it->next();
    }
    return new \HH\Map($res);
  }

  /** Returns a Map containing the first n key/value pairs of this Map.
   * @param int $n
   * @return Map
   */
  public function take(int $n)[]: \HH\Map {
    if ($n >= $this->count()) {
      return $this->toMap();
    } else if ($n <= 0) {
      return new \HH\Map();
    }

    $res = dict[];
    foreach (dict($this) as $k => $v) {
      $res[$k] = $v;
      if (--$n == 0) break;
    }

    return new \HH\Map($res);
  }

  /** Returns a Map containing the key/value pairs of this Map up to but not
   * including the first value that produces false when passed to the specified
   * callback.
   * @param mixed $callback
   * @return object
   */
  public function takeWhile((function()[_]: void) $callback)[ctx $callback]: \HH\Map {
    $res = dict[];
    foreach (dict($this) as $k => $v) {
      if (!$callback($v)) {
        break;
      }
      $res[$k] = $v;
    }
    return new \HH\Map($res);
  }

  /** Returns a Map containing all key/value pairs except the first n of this
   * Map.
   * @param int $n
   * @return Map
   */
  public function skip(int $n)[]: \HH\Map {
    if ($n <= 0) {
      return $this->toMap();
    }

    $res = dict[];
    foreach (dict($this) as $k => $v) {
      if ($n-- > 0) continue;
      $res[$k] = $v;
    }

    return new \HH\Map($res);
  }

  /** Returns a Map containing the key/value pairs of this Map excluding the
   * first values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  public function skipWhile((function()[_]: void) $fn)[ctx $fn]: \HH\Map {
    $res = dict[];
    $skipping = true;
    foreach (dict($this) as $k => $v) {
      if ($skipping) {
        if ($fn($v)) {
          continue;
        }
        $skipping = false;
      }
      $res[$k] = $v;
    }
    return new \HH\Map($res);
  }

  /** Returns a Map containing the specified range of key/value pairs from this
   * Map. The range is specified by two non-negative integers: a starting
   * position and a length.
   * @param int $start
   * @param int $len
   * @return Map
   */
  public function slice(int $start, int $len)[]: \HH\Map {
    if ($start < 0) {
      throw new InvalidArgumentException("Parameter start must be a non-negative integer");
    }
    if ($len < 0) {
      throw new InvalidArgumentException("Parameter len must be a non-negative integer");
    }

    $i = 0;
    $end = $start + $len;
    $res = dict[];

    foreach (dict($this) as $k => $v) {
      if ($i < $start) {
        $i++;
        continue;
      } else if($i == $end) {
        break;
      }
      $res[$k] = $v;
      $i++;
    }

    return new \HH\Map($res);
  }

  /** Builds a new Vector by concatenating the values of this Map with the
   * elements of the specified Iterable. Note that this ignores the keys of this
   * Map and the specified Iterable.
   * @param Traversable $iterable
   * @return Vector
   */
  public function concat(\HH\Traversable $iterable)[]: \HH\Vector {
    $res = vec(dict($this));

    foreach ($iterable as $v) {
      $res[] = $v;
    }

    return new \HH\Vector($res);
  }

  /** Returns the first value from this Map, or null if this Map is empty.
   * @return mixed
   */
  <<__Native>>
  public function firstValue()[]: mixed;

  /** Returns the first key from this Map, or null if this Map is empty.
   * @return mixed
   */
  <<__Native>>
  public readonly function firstKey()[]: mixed;

  /** Returns the last value from this Map, or null if this Map is empty.
   * @return mixed
   */
  <<__Native>>
  public function lastValue()[]: mixed;

  /** Returns the last key from this Map, or null if this Map is empty.
   * @return mixed
   */
  <<__Native>>
  public readonly function lastKey()[]: mixed;

  /** @return string
   */
  public function __toString()[]: string { return "Map"; }

  /** Returns a Map built from the key/value Pairs produced by the specified
   * Iterable.
   * @param ?Traversable $iterable
   * @return Map
   */
  public static function fromItems(?\HH\Traversable $iterable)[]: \HH\Map {
    if ($iterable is null) return new \HH\Map();

    $res = dict[];
    foreach ($iterable as $pair) {
      if ($pair is \HH\Pair) {
        $res[$pair[0]] = $pair[1];
      } else {
        throw new InvalidArgumentException("Parameter must be an instance of Iterable<Pair>");
      }
    }

    return new \HH\Map($res);
  }

  /** Returns a Map built from the keys and values from the specified array.
   * @param mixed $mp
   * @return Map
   */
  public static function fromArray(mixed $mp): \HH\Map {
    return new \HH\Map($mp);
  }
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
  public readonly function isEmpty()[]: bool {
    return !$this->count();
  }

  /** Returns the number of key/value pairs in the ImmMap.
   * @return int
   */
  <<__Native>>
  public readonly function count()[]: int;

  /** Returns an Iterable that produces the key/value Pairs from this ImmMap.
   * @return object
   */
  public function items()[]: \LazyKVZipIterable {
    return new \LazyKVZipIterable($this);
  }

  /** Returns a ImmVector built from the keys of this ImmMap.
   * @return ImmVector
   */
  public readonly function keys()[]: \HH\ImmVector {
    $res = vec[];
    foreach (dict($this) as $k => $_) {
      $res[] = $k;
    }
    return new \HH\ImmVector($res);
  }

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
  public readonly function contains(mixed $key)[]: bool;

  /** Returns true if the specified key is present in the ImmMap, false
   * otherwise.
   * @param mixed $key
   * @return bool
   */
  <<__Native>>
  public readonly function containsKey(mixed $key)[]: bool;

  public function toVArray()[]: varray {
    return $this->toValuesArray();
  }

  /** Returns a darray built from the keys and values from this ImmMap.
   * @return darray
   */
  public function toDArray()[]: darray {
    return dict($this);
  }

  /** Returns a Vector built from the values of this ImmMap.
   * @return object
   */
  <<__Native>>
  public function toVector()[]: object;

  /** Returns a ImmVector built from the values of this ImmMap.
   * @return object
   */
  <<__Native>>
  public function toImmVector()[]: object;

  /** Returns a Map built from the keys and values of this ImmMap.
   * @return object
   */
  <<__Native>>
  public function toMap()[]: object;

  /** Returns an immutable version of this collection.
   * @return object
   */
  public function toImmMap()[]: this {
    return $this;
  }

  /** Returns a Set built from the values of this ImmMap.
   * @return object
   */
  <<__Native>>
  public function toSet()[]: object;

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
   * @return ImmVector
   */
  public function values()[]: \HH\ImmVector {
    $res = vec(dict($this));
    return new \HH\ImmVector($res);
  }

  /** Returns a varray built from the keys from this ImmMap.
   * @return varray
   */
  public function toKeysArray()[]: varray {
    $keys = varray[];
    foreach (dict($this) as $k => $_) {
      $keys[] = $k;
    }
    return $keys;
  }

  /** Returns a varray built from the values from this ImmMap.
   * @return varray
   */
  public function toValuesArray()[]: varray {
    return vec(dict($this));
  }

  /** @param mixed $it
   * @return ImmMap
   */
  public function differenceByKey(\HH\KeyedTraversable $it)[]: \HH\ImmMap {
    $res = dict($this);
    foreach ($it as $k => $_) {
      unset($res[$k]);
    }
    return new \HH\ImmMap($res);
  }

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
  public function map((function()[_]: void) $callback)[ctx $callback]: \HH\ImmMap {
    $res = $this->toMap();
    foreach (dict($this) as $k => $v) {
      $res[$k] = $callback($v);
    }
    return $res->toImmMap();
  }

  /** Returns a ImmMap of the keys/values produced by applying the specified
   * callback on each key and value from this ImmMap.
   * @param mixed $callback
   * @return object
   */
  public function mapWithKey((function()[_]: void) $callback)[ctx $callback]: \HH\ImmMap {
    $res = $this->toMap();
    foreach (dict($this) as $k => $v) {
      $res[$k] = $callback($k, $v);
    }
    return $res->toImmMap();
  }

  /** Returns a ImmMap of all the keys/values from this ImmMap for which the
   * specified callback returns true.
   * @param mixed $callback
   * @return object
   */
  public function filter((function()[_]: void) $callback)[ctx $callback]: \HH\ImmMap {
    $res = dict[];
    foreach (dict($this) as $k => $v) {
      if ($callback($v)) {
        $res[$k] = $v;
      }
    }
    return new \HH\ImmMap($res);
  }

  /** Returns a ImmMap of all the keys/values from this ImmMap for which the
   * specified callback returns true.
   * @param mixed $callback
   * @return object
   */
  public function filterWithKey((function()[_]: void) $callback)[ctx $callback]: \HH\ImmMap {
    $res = dict[];
    foreach (dict($this) as $k => $v) {
      if ($callback($k, $v)) {
        $res[$k] = $v;
      }
    }
    return new \HH\ImmMap($res);
  }

  /** Returns a KeyedIterable produced by combined the specified Iterables
   * pair-wise.
   * @param Traversable $iterable
   * @return ImmMap
   */
  public function zip(\HH\Traversable $iterable)[]: \HH\ImmMap {
    $res = dict[];
    $it = $this->getIterator();
    foreach ($iterable as $v) {
      if (!$it->valid()) break;
      $res[$it->key()] = Pair { $it->current(), $v };
      $it->next();
    }
    return new \HH\ImmMap($res);
  }

  /** Returns a ImmMap containing the first n key/value pairs of this ImmMap.
   * @param int $n
   * @return ImmMap
   */
  public function take(int $n)[]: \HH\ImmMap {
    if ($n >= $this->count()) {
      return $this;
    } else if ($n <= 0) {
      return new \HH\ImmMap();
    }

    $res = dict[];
    foreach (dict($this) as $k => $v) {
      $res[$k] = $v;
      if (--$n == 0) break;
    }

    return new \HH\ImmMap($res);
  }

  /** Returns a ImmMap containing the key/value pairs of this ImmMap up to but
   * not including the first value that produces false when passed to the
   * specified callback.
   * @param mixed $callback
   * @return object
   */
  public function takeWhile((function()[_]: void) $callback)[ctx $callback]: \HH\ImmMap {
    $res = dict[];
    foreach (dict($this) as $k => $v) {
      if (!$callback($v)) {
        break;
      }
      $res[$k] = $v;
    }
    return new \HH\ImmMap($res);
  }

  /** Returns a ImmMap containing all key/value pairs except the first n of this
   * ImmMap.
   * @param int $n
   * @return object
   */
  public function skip(int $n)[]: \HH\ImmMap {
    if ($n <= 0) {
      return $this;
    }

    $res = dict[];
    foreach (dict($this) as $k => $v) {
      if ($n-- > 0) continue;
      $res[$k] = $v;
    }

    return new \HH\ImmMap($res);
  }

  /** Returns a ImmMap containing the key/value pairs of this ImmMap excluding
   * the first values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  public function skipWhile((function()[_]: void) $fn)[ctx $fn]: \HH\ImmMap {
    $res = dict[];
    $skipping = true;
    foreach (dict($this) as $k => $v) {
      if ($skipping) {
        if ($fn($v)) {
          continue;
        }
        $skipping = false;
      }
      $res[$k] = $v;
    }
    return new \HH\ImmMap($res);
  }

  /** Returns a ImmMap containing the specified range of key/value pairs from
   * this ImmMap. The range is specified by two non-negative integers: a
   * starting position and a length.
   * @param int $start
   * @param int $len
   * @return ImmMap
   */
  public function slice(int $start, int $len)[]: \HH\ImmMap {
    if ($start < 0) {
      throw new InvalidArgumentException("Parameter start must be a non-negative integer");
    }
    if ($len < 0) {
      throw new InvalidArgumentException("Parameter len must be a non-negative integer");
    }

    $i = 0;
    $end = $start + $len;
    $res = dict[];

    foreach (dict($this) as $k => $v) {
      if ($i < $start) {
        $i++;
        continue;
      } else if($i == $end) {
        break;
      }
      $res[$k] = $v;
      $i++;
    }

    return new \HH\ImmMap($res);
  }

  /** Builds a new ImmVector by concatenating the values of this ImmMap with the
   * elements of the specified Iterable. Note that this ignores the keys of this
   * ImmMap and the specified Iterable.
   * @param Traversable $iterable
   * @return ImmVector
   */
  public function concat(\HH\Traversable $iterable)[]: \HH\ImmVector {
    $res = vec(dict($this));

    foreach ($iterable as $v) {
      $res[] = $v;
    }

    return new \HH\ImmVector($res);
  }

  /** Returns the first value from this ImmMap, or null if this ImmMap is empty.
   * @return mixed
   */
  <<__Native>>
  public function firstValue()[]: mixed;

  /** Returns the first key from this ImmMap, or null if this ImmMap is empty.
   * @return mixed
   */
  <<__Native>>
  public readonly function firstKey()[]: mixed;

  /** Returns the last value from this ImmMap, or null if this ImmMap is empty.
   * @return mixed
   */
  <<__Native>>
  public function lastValue()[]: mixed;

  /** Returns the last key from this ImmMap, or null if this ImmMap is empty.
   * @return mixed
   */
  <<__Native>>
  public readonly function lastKey()[]: mixed;

  /** @return string
   */
  public function __toString()[]: string { return "ImmMap"; }

  /** Returns a ImmMap built from the key/value Pairs produced by the specified
   * Iterable.
   * @param ?Traversable $iterable
   * @return ImmMap
   */
  public static function fromItems(?\HH\Traversable $iterable)[]: \HH\ImmMap {
    if ($iterable is null) return new \HH\ImmMap();

    $res = dict[];
    foreach ($iterable as $pair) {
      if ($pair is \HH\Pair) {
        $res[$pair[0]] = $pair[1];
      } else {
        throw new InvalidArgumentException("Parameter must be an instance of Iterable<Pair>");
      }
    }

    return new \HH\ImmMap($res);
  }
}

} // namespace HH
