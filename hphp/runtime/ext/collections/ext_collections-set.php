<?hh

namespace {

/* An iterator implementation for iterating over a Set.
 */
<<__NativeData("SetIterator")>>
final class SetIterator implements HH\Iterator {

  public function __construct(): void {}

  /* Returns the current value that the iterator points to.
   * @return mixed
   */
  <<__Native>>
  public function current(): mixed;

  /* @return mixed
   */
  <<__Native>>
  public function key(): mixed;

  /* Returns true if the iterator points to a valid value, returns false
   * otherwise.
   * @return bool
   */
  <<__Native>>
  public function valid(): bool;

  /* Advance this iterator forward one position.
   */
  <<__Native>>
  public function next(): void;

  /* Move this iterator back to the first position.
   */
  <<__Native>>
  public function rewind(): void;
}

} // empty namespace
namespace HH {

/* An ordered set-style collection.
 */
final class Set implements \MutableSet {

  /* Returns a Set built from the values produced by the specified Iterable.
   * @param mixed $iterable
   */
  <<__Native>>
  public function __construct(mixed $iterable = null): void;

  /* Returns true if the Set is empty, false otherwise.
   * @return bool
   */
  public function isEmpty(): bool { return !$this->count(); }

  /* Returns the number of values in the Set.
   * @return int
   */
  <<__Native>>
  public function count(): int;

  /* Returns an Iterable that produces the values from this Set.
   * @return object
   */
  public function items(): \LazyIterableView {
    return new \LazyIterableView($this);
  }


  /* Returns a Vector built from the keys of this Set.
   * @return object
   */
  public function keys() { return $this->values(); }

  /* Returns a Vector built from the values of this Set.
   * @return object
   */
  <<__Native>>
  public function values(): object;

  /* Returns a lazy iterable view of this Set.
   * @return object
   */
  public function lazy(): \LazyKeyedIterableView {
    return new \LazyKeyedIterableView($this);
  }

  /* Removes all values from the Set.
   * @return object
   */
  <<__Native>>
  public function clear(): object;

  /* Returns true if the specified value is present in the Set, returns false
   * otherwise.
   * @param mixed $val
   * @return bool
   */
  <<__Native>>
  public function contains(mixed $val): bool;

  /* Removes the specified value from this Set.
   * @param mixed $val
   * @return object
   */
  <<__Native>>
  public function remove(mixed $val): object;

  /* Adds the specified value to this Set.
   * @param mixed $val
   * @return object
   */
  <<__Native>>
  public function add(mixed $val): object;

  /* Adds the values produced by the specified Iterable to this Set.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function addAll(mixed $iterable): object;

  /* Adds the keys of the specified KeyedContainer to this Set.
   * @param mixed $container
   * @return object
   */
  <<__Native>>
  public function addAllKeysOf(mixed $container): object;

  /* Instructs this Set to grow its capacity to accommodate the given number of
   * elements. The caller is expected to make the appropriate add/addAll calls
   * to fill that reserved capacity.
   * @param mixed $sz
   */
  <<__Native>>
  public function reserve(int $sz): void;

  /* Returns an array built from the values from this Set, array(val1 => val1,
   * val2 => val2, ...). This maintains set-like semantics in array() land: O(1)
   * membership test with `array_has_key($a['key'])` and iteration with
   * `foreach($a as $member)`. Int-like strings end up with numerical array
   * keys.
   * @return array
   */
  <<__Native>>
  public function toArray(): array;

  /* Returns a Vector built from the values of this Set.
   * @return object
   */
  <<__Native>>
  public function toVector(): object;

  /* Returns a ImmVector built from the values of this Set.
   * @return object
   */
  <<__Native>>
  public function toImmVector(): object;

  /* Returns a Map built from the keys and values of this Set.
   * @return object
   */
  <<__Native>>
  public function toMap(): object;

  /* Returns a ImmMap built from the keys and values of this Set.
   * @return object
   */
  <<__Native>>
  public function toImmMap(): object;

  /* Returns a copy of this Set.
   * @return object
   */
  public function toSet(): this {
    return clone $this;
  }

  /* Returns a ImmSet built from the values of this Set.
   * @return object
   */
  <<__Native>>
  public function toImmSet(): object;

  /* Returns an immutable version of this collection.
   * @return object
   */
  public function immutable() { return $this->toImmSet(); }

  /* Returns an array built from the values from this Set.
   * @return array
   */
  <<__Native>>
  public function toKeysArray(): array;

  /* Returns an array built from the values from this Set.
   * @return array
   */
  <<__Native>>
  public function toValuesArray(): array;

  /* Returns an iterator that points to beginning of this Set.
   * @return object
   */
  <<__Native>>
  public function getIterator(): object;

  /* Returns a Set of the values produced by applying the specified callback on
   * each value from this Set.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function map(mixed $callback): object;

  /* Returns a Set of the values produced by applying the specified callback on
   * each key and value from this Set.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function mapWithKey(mixed $callback): object;

  /* Returns a Set of all the values from this Set for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function filter(mixed $callback): object;

  /* Returns a Set of all the values from this Set for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function filterWithKey(mixed $callback): object;

  /* Ensures that this Set contains only values for which the specified callback
   * returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function retain(mixed $callback): object;

  /* Ensures that this Set contains only keys/values for which the specified
   * callback returns true when passed the key and the value.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function retainWithKey(mixed $callback): object;

  /* Returns a Iterable produced by combined the specified Iterables pair-wise.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function zip(mixed $iterable): object;

  /* Returns a Set containing the first n values of this Set.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function take(mixed $n): object;

  /* Returns a Set containing the values of this Set up to but not including the
   * first value that produces false when passed to the specified callback.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function takeWhile(mixed $callback): object;

  /* Returns a Set containing all values except the first n of this Set.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function skip(mixed $n): object;

  /* Returns a Set containing all the values of this Set excluding the first
   * values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  <<__Native>>
  public function skipWhile(mixed $fn): object;

  /* Returns a Set containing the specified range of values from this Set. The
   * range is specified by two non-negative integers: a starting position and a
   * length.
   * @param mixed $start
   * @param mixed $len
   * @return object
   */
  <<__Native>>
  public function slice(mixed $start,
                        mixed $len): object;

  /* Builds a new Vector by concatenating the elements of this Set with the
   * elements of the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function concat(mixed $iterable): object;

  /* Returns the first value from this Set, or null if this Set is empty.
   * @return mixed
   */
  <<__Native>>
  public function firstValue(): mixed;

  /* Returns the first key from this Set, or null if this Vector is empty.
   * @return mixed
   */
  public function firstKey() {
    return $this->firstValue();
  }

  /* Returns the last value from this Set, or null if this Set is empty.
   * @return mixed
   */
  <<__Native>>
  public function lastValue(): mixed;

  /* Returns the last key from this Set, or null if this Set is empty.
   * @return mixed
   */
  public function lastKey() {
    return $this->lastValue();
  }

  /* @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function removeAll(mixed $iterable): object;

  /* @param mixed $iterable
   * @return object
   */
  public function difference(mixed $iterable) {
    // This isn't really a difference method, it lies
    return $this->removeAll($iterable);
  }

  /* @return string
   */
  public function __toString(): string { return "Set"; }

  /* @param mixed $name
   * @return mixed
   */
  public function __get(mixed $name): mixed {
    throw new \InvalidOperationException(
      "Cannot access a property on a collection"
    );
  }

  /* @param mixed $name
   * @param mixed $value
   * @return mixed
   */
  public function __set(mixed $name,
                        mixed $value): mixed {
    throw new \InvalidOperationException(
      "Cannot access a property on a collection"
    );
  }

  /* @param mixed $name
   * @return bool
   */
  public function __isset(mixed $name): bool { return false; }

  /* @param mixed $name
   * @return mixed
   */
  public function __unset(mixed $name): mixed {
    throw new \InvalidOperationException(
      "Cannot access a property on a collection"
    );
  }

  /* Returns a Set built from the values produced by the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public static function fromItems(mixed $iterable): object;

  /* Returns a Set built from the keys of the specified container.
   * @param mixed $container
   * @return object
   */
  <<__Native>>
  public static function fromKeysOf(mixed $container): object;

  /* Returns a Set built from the values from the specified array.
   * @param mixed $arr
   * @return object
   */
  <<__Native>>
  public static function fromArray(mixed $arr): object;

  /* Returns a Set built from the values from the specified arrays.
   * @return object
   */
  public static function fromArrays(...$argv) {
    if (!$argv) return \HH\Set {};
    $ret = \HH\Set {};
    foreach ($argv as $arr) {
      if (!\is_array($arr)) {
        throw new \InvalidArgumentException("Parameters must be arrays");
      }
      $ret->addAll($arr);
    }
    return $ret;
  }
}

/* An immutable ordered set-style collection.
 */
final class ImmSet implements \ConstSet {

  /* Returns a ImmSet built from the values produced by the specified Iterable.
   * @param mixed $iterable
   */
  <<__Native>>
  public function __construct(mixed $iterable = null): void;

  /* Returns true if the ImmSet is empty, false otherwise.
   * @return bool
   */
  public function isEmpty(): bool { return !$this->count(); }

  /* Returns the number of values in the ImmSet.
   * @return int
   */
  <<__Native>>
  public function count(): int;

  /* Returns an Iterable that produces the values from this ImmSet.
   * @return object
   */
  public function items(): \LazyIterableView {
    return new \LazyIterableView($this);
  }


  /* Returns a Vector built from the keys of this ImmSet.
   * @return object
   */
  public function keys() { return $this->values(); }

  /* Returns a ImmVector built from the values of this ImmSet.
   * @return object
   */
  <<__Native>>
  public function values(): object;

  /* Returns a lazy iterable view of this ImmSet.
   * @return object
   */
  public function lazy(): \LazyKeyedIterableView {
    return new \LazyKeyedIterableView($this);
  }

  /* Returns true if the specified value is present in the ImmSet, returns false
   * otherwise.
   * @param mixed $val
   * @return bool
   */
  <<__Native>>
  public function contains(mixed $val): bool;

  /* Returns an array built from the values from this ImmSet, array(val1 =>
   * val1, val2 => val2, ...). This maintains set-like semantics in array()
   * land: O(1) membership test with `array_has_key($a['key'])` and iteration
   * with `foreach($a as $member)`. Int-like strings end up with numerical array
   * keys.
   * @return array
   */
  <<__Native>>
  public function toArray(): array;

  /* Returns a Vector built from the values of this ImmSet.
   * @return object
   */
  <<__Native>>
  public function toVector(): object;

  /* Returns a ImmVector built from the values of this ImmSet.
   * @return object
   */
  <<__Native>>
  public function toImmVector(): object;

  /* Returns a Map built from the keys and values of this ImmSet.
   * @return object
   */
  <<__Native>>
  public function toMap(): object;

  /* Returns a ImmMap built from the keys and values of this ImmSet.
   * @return object
   */
  <<__Native>>
  public function toImmMap(): object;

  /* Returns a Set built from the values of this ImmSet.
   * @return object
   */
  <<__Native>>
  public function toSet(): object;

  /* Returns an immutable version of this collection.
   * @return object
   */
  public function toImmSet(): this {
    return $this;
  }

  /* Returns an immutable version of this collection.
   * @return object
   */
  public function immutable(): this {
    return $this;
  }

  /* Returns an array built from the values from this ImmSet.
   * @return array
   */
  <<__Native>>
  public function toKeysArray(): array;

  /* Returns an array built from the values from this ImmSet.
   * @return array
   */
  <<__Native>>
  public function toValuesArray(): array;

  /* Returns an iterator that points to beginning of this ImmSet.
   * @return object
   */
  <<__Native>>
  public function getIterator(): object;

  /* Returns a ImmSet of the values produced by applying the specified callback
   * on each value from this ImmSet.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function map(mixed $callback): object;

  /* Returns a ImmSet of the values produced by applying the specified callback
   * on each key and value from this ImmSet.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function mapWithKey(mixed $callback): object;

  /* Returns a ImmSet of all the values from this ImmSet for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function filter(mixed $callback): object;

  /* Returns a ImmSet of all the values from this ImmSet for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function filterWithKey(mixed $callback): object;

  /* Returns an Iterable produced by combining the specified Iterables
   * pair-wise.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function zip(mixed $iterable): object;

  /* Returns a ImmSet containing the first n values of this ImmSet.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function take(mixed $n): object;

  /* Returns a ImmSet containing the values of this ImmSet up to but not
   * including the first value that produces false when passed to the specified
   * callback.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function takeWhile(mixed $callback): object;

  /* Returns a ImmSet containing all values except the first n of this ImmSet.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function skip(mixed $n): object;

  /* Returns a ImmSet containing all the values of this ImmSet excluding the
   * first values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  <<__Native>>
  public function skipWhile(mixed $fn): object;

  /* Returns a ImmSet containing the specified range of values from this ImmSet.
   * The range is specified by two non-negative integers: a starting position
   * and a length.
   * @param mixed $start
   * @param mixed $len
   * @return object
   */
  <<__Native>>
  public function slice(mixed $start,
                        mixed $len): object;

  /* Builds a new ImmVector by concatenating the elements of this ImmSet with
   * the elements of the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function concat(mixed $iterable): object;

  /* Returns the first value from this ImmSet, or null if this ImmSet is empty.
   * @return mixed
   */
  <<__Native>>
  public function firstValue(): mixed;

  /* Returns the first key from this ImmSet, or null if this ImmSet is empty.
   * @return mixed
   */
  public function firstKey() {
    return $this->firstValue();
  }

  /* Returns the last value from this ImmSet, or null if this ImmSet is empty.
   * @return mixed
   */
  <<__Native>>
  public function lastValue(): mixed;

  /* Returns the last key from this ImmSet, or null if this ImmSet is empty.
   * @return mixed
   */
  public function lastKey() {
    return $this->lastValue();
  }

  /* @return string
   */
  public function __toString(): string { return "ImmSet"; }

  /* @param mixed $name
   * @return mixed
   */
  public function __get(mixed $name): mixed {
    throw new \InvalidOperationException(
      "Cannot access a property on a collection"
    );
  }

  /* @param mixed $name
   * @param mixed $value
   * @return mixed
   */
  public function __set(mixed $name,
                        mixed $value): mixed {
    throw new \InvalidOperationException(
      "Cannot access a property on a collection"
    );
  }

  /* @param mixed $name
   * @return bool
   */
  public function __isset(mixed $name): bool { return false; }

  /* @param mixed $name
   * @return mixed
   */
  public function __unset(mixed $name): mixed {
    throw new \InvalidOperationException(
      "Cannot access a property on a collection"
    );
  }

  /* Returns a ImmSet built from the values produced by the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public static function fromItems(mixed $iterable): object;

  /* Returns a ImmSet built from the keys of the specified container.
   * @param mixed $container
   * @return object
   */
  <<__Native>>
  public static function fromKeysOf(mixed $container): object;

  /* Returns a ImmSet built from the values from the specified arrays.
   * @return object
   */
  public static function fromArrays(...$argv) {
    if (!$argv) return \HH\ImmSet {};
    $ret = \HH\Set {};
    foreach ($argv as $arr) {
      if (!\is_array($arr)) {
        throw new \InvalidArgumentException("Parameters must be arrays");
      }
      $ret->addAll($arr);
    }
    return $ret->toImmSet();
  }
}

} // namespace HH
