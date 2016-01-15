<?hh

namespace {

/* An iterator implementation for iterating over a Pair.
 */
<<__NativeData("PairIterator")>>
final class PairIterator implements HH\KeyedIterator {

  public function __construct(): void {}

  /* Returns the current value that the iterator points to.
   * @return mixed
   */
  <<__Native>>
  public function current(): mixed;

  /* Returns the current key that the iterator points to.
   * @return mixed
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

/* An immutable ordered fixed-sized container.
 */
<<__Collection>>
final class Pair implements \ConstVector {

  public function __construct(): void {
    throw new \InvalidOperationException(
      "Pairs cannot be created using the new operator"
    );
  }

  /* Returns true if this Pair is empty, false otherwise.
   * @return bool
   */
  public function isEmpty(): bool { return false; }

  /* Returns the number of values in the Pair.
   * @return int
   */
  public function count(): int { return 2; }

  /* Returns an Iterable that produces the values from this Pair.
   * @return object
   */
  public function items(): \LazyIterableView {
    return new \LazyIterableView($this);
  }

  /* Returns a Vector built from the keys of this Pair.
   * @return object
   */
  public function keys(): ImmVector<int> {
    return ImmVector { 0, 1 };
  }

  /* Returns a Vector built from the values of this Pair.
   * @return object
   */
  <<__Native>>
  public function values(): object;

  /* Returns a lazy iterable view of this Pair.
   * @return object
   */
  public function lazy(): \LazyKeyedIterableView {
    return new \LazyKeyedIterableView($this);
  }

  /* Returns an array built from the values from this Pair.
   * @return array
   */
  <<__Native>>
  public function toArray(): array;

  /* Returns a Vector build from the values from this Pair.
   * @return object
   */
  <<__Native>>
  public function toVector(): object;

  /* Returns a ImmVector built from the values of this Pair.
   * @return object
   */
  <<__Native>>
  public function toImmVector(): object;

  /* Returns a Map built from the keys and values of this Pair.
   * @return object
   */
  <<__Native>>
  public function toMap(): object;

  /* Returns a ImmMap built from the keys and values of this Pair.
   * @return object
   */
  <<__Native>>
  public function toImmMap(): object;

  /* Returns a Set built from the values of this Pair.
   * @return object
   */
  <<__Native>>
  public function toSet(): object;

  /* Returns a ImmSet built from the values of this Pair.
   * @return object
   */
  <<__Native>>
  public function toImmSet(): object;

  /* Returns an immutable version of this collection.
   * @return object
   */
  public function immutable(): this { return $this; }

  /* Returns an array built from the keys from this Pair.
   * @return array
   */
  public function toKeysArray(): array { return array(0, 1); }

  /* Returns an array built from the values from this Pair.
   * @return array
   */
  <<__Native>>
  public function toValuesArray(): array;

  /* Returns an iterator that points to beginning of this Pair.
   * @return object
   */
  <<__Native>>
  public function getIterator(): object;

  /* Returns a Vector of the values produced by applying the specified callback
   * on each value from this Pair.
   * @param mixed $callback
   * @return object
   */
  public function map($callback) {
    return ImmVector { $callback($this[0]), $callback($this[1]) };
  }

  /* Returns a Vector of the values produced by applying the specified callback
   * on each key and value from this Pair.
   * @param mixed $callback
   * @return object
   */
  public function mapWithKey($callback) {
    return ImmVector { $callback(0, $this[0]), $callback(1, $this[1]) };
  }

  /* Returns a Vector of all the values from this Pair for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  public function filter($callback) {
    $values = $this->toArray();
    if (!$callback($values[0])) { unset($values[0]); }
    if (!$callback($values[1])) { unset($values[1]); }
    return new \HH\ImmVector($values);
  }

  /* Returns a Vector of all the values from this Pair for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  public function filterWithKey($callback) {
    $values = $this->toArray();
    if (!$callback(0, $values[0])) { unset($values[0]); }
    if (!$callback(1, $values[1])) { unset($values[1]); }
    return new \HH\ImmVector($values);
  }

  /* Returns a KeyedIterable produced by combined the specified Iterables
   * pair-wise.
   * @param mixed $iterable
   * @return object
   */
  public function zip($iterable) {
    $ret = \HH\Vector {};
    $i = 0;
    foreach ($iterable as $value) {
      if ($i >= 2) break;
      $ret[] = \HH\Pair { $this[$i++], $value };
    }
    return $ret->immutable();
  }

  /* Returns a Vector containing the first n values of this Pair.
   * @param mixed $n
   * @return object
   */
  public function take($n) {
    if (!\is_int($n)) {
      throw new \InvalidArgumentException(
        "Parameter n must be an integer"
      );
    }
    if ($n <= 0) return \HH\ImmVector {};
    if ($n == 1) return \HH\ImmVector { $this[0] };
    return $this->toImmVector();
  }

  /* Returns a Vector containing the values of this Pair up to but not including
   * the first value that produces false when passed to the specified callback.
   * @param mixed $callback
   * @return object
   */
  public function takeWhile($callback) {
    $pair = $this->toArray();
    if (!$callback($pair[0])) return \HH\ImmVector {};
    if (!$callback($pair[1])) return \HH\ImmVector { $pair[0] };
    return $this->toImmVector();
  }

  /* Returns a Vector containing all values except the first n of this Pair.
   * @param mixed $n
   * @return object
   */
  public function skip($n) {
    if (!\is_int($n)) {
      throw new \InvalidArgumentException(
        "Parameter n must be an integer"
      );
    }
    if ($n <= 0) return $this->toImmVector();
    if ($n == 1) return \HH\ImmVector { $this[1] };
    return \HH\ImmVector {};
  }

  /* Returns a Vector containing all the values of this Pair excluding the first
   * values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  public function skipWhile($callback) {
    $pair = $this->toArray();
    if (!$callback($pair[0])) return $this->toImmVector();
    if (!$callback($pair[1])) return \HH\ImmVector { $pair[1] };
    return \HH\ImmVector {};
  }

  /* Returns index of the specified value if it is present, -1 otherwise.
   * @param mixed $search_value
   * @return int
   */
  <<__Native>>
  public function linearSearch(mixed $search_value): int;

  /* Returns an ImmVector containing the specified range of values from this
   * Pair. The range is specified by two non-negative integers: a starting
   * position and a length.
   * @param mixed $start
   * @param mixed $len
   * @return object
   */
  public function slice($start, $len) {
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

  /* Builds a new ImmVector by concatenating the elements of this Pair with the
   * elements of the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  public function concat($iterable) {
    $ret = $this->toVector();
    foreach($iterable as $value) {
      $ret->append($value);
    }
    return $ret->toImmVector();
  }

  /* Returns the first value from this Pair.
   * @return mixed
   */
  public function firstValue() { return $this[0]; }

  /* Returns the first key from this Pair.
   * @return mixed
   */
  public function firstKey() { return 0; }

  /* Returns the last value from this Pair.
   * @return mixed
   */
  public function lastValue() { return $this[1]; }

  /* Returns the last key from this Pair.
   * @return mixed
   */
  public function lastKey() { return 1; }

  /* Returns the value at the specified key. If the key is not present, an
   * exception is thrown.
   * @param mixed $key
   * @return mixed
   */
  <<__Native>>
  public function at(mixed $key): mixed;

  /* Returns the value at the specified key. If the key is not present, null is
   * returned.
   * @param mixed $key
   * @return mixed
   */
  <<__Native>>
  public function get(mixed $key): mixed;

  /* Returns true if the specified key is present in the Pair, returns false
   * otherwise.
   * @param mixed $key
   * @return bool
   */
  public function containsKey($key): bool {
    if (!\is_int($key)) {
      throw new \InvalidArgumentException(
        "Only integer keys may be used with Pairs"
      );
    }
    return ($key === 0) || ($key === 1);
  }

  /* @return string
   */
  public function __toString(): string { return "Pair"; }

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
}

} // namespace HH
