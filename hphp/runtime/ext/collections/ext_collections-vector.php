<?hh

namespace {

/* An iterator implementation for iterating over a Vector/ImmVector.
 */
<<__NativeData("VectorIterator")>>
final class VectorIterator implements HH\KeyedIterator {

  /* Do nothing */
  public function __construct(): void { }

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

/* An ordered collection where values are keyed using integers 0 thru n-1 in
 * order.
 */
final class Vector implements \MutableVector {

  /* Returns a Vector built from the values produced by the specified Iterable.
   * @param mixed $iterable
   */
  <<__Native>>
  public function __construct(mixed $iterable = null): void;

  /* Returns true if the Vector is empty, false otherwise.
   * @return bool
   */
  public function isEmpty(): bool {
    return !$this->count();
  }

  /* Returns the number of values in the Vector.
   * @return int
   */
  <<__Native>>
  public function count(): int;

  /* Returns an Iterable that produces the values from this Vector.
   * @return object
   */
  public function items(): \LazyIterableView {
    return new \LazyIterableView($this);
  }

  /* Returns a Vector built from the keys of this Vector.
   * @return object
   */
  <<__Native>>
  public function keys(): object;

  /* Returns a clone of this Vector.
   * @return object
   */
  public function values(): this {
    return clone $this;
  }

  /* Returns a lazy iterable view of this Vector.
   * @return object
   */
  public function lazy(): \LazyKeyedIterableView {
    return new \LazyKeyedIterableView($this);
  }

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

  /* Stores a value into the Vector with the specified key, overwriting any
   * previous value that was associated with the key; if the key is outside the
   * bounds of the Vector, an exception is thrown.
   * @param mixed $key
   * @param mixed $value
   * @return object
   */
  <<__Native>>
  public function set(mixed $key,
                      mixed $value): object;

  /* Stores each value produced by the specified KeyedIterable into the Vector
   * using its corresponding key, overwriting any previous value that was
   * associated with that key; if the key is outside the bounds of the Vector,
   * an exception is thrown.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function setAll(mixed $iterable): object;

  /* Removes all values from the Vector.
   * @return object
   */
  <<__Native>>
  public function clear(): object;

  /* Returns true if the specified key is present in the Vector, returns false
   * otherwise.
   * @param mixed $key
   * @return bool
   */
  <<__Deprecated(
    "Use Vector::containsKey() for key search or Vector::linearSearch() for value serach"
  )>>
  public function contains(mixed $key): bool {
    if (!\is_int($key)) {
      throw new \InvalidArgumentException(
        "Only integer keys may be used with Vectors"
      );
    }
    return ($key >= 0) && ($key < $this->count());
  }

  /* Returns true if the specified key is present in the Vector, returns false
   * otherwise.
   * @param mixed $key
   * @return bool
   */
  public function containsKey(mixed $key): bool {
    if (!\is_int($key)) {
      throw new \InvalidArgumentException(
        "Only integer keys may be used with Vectors"
      );
    }
    return ($key >= 0) && ($key < $this->count());
  }

  /* Removes the element with the specified key from this Vector and renumbers
   * the keys of all subsequent elements.
   * @param mixed $key
   * @return object
   */
  <<__Native>>
  public function removeKey(mixed $key): object;

  /* @param mixed $val
   * @return object
   */
  <<__Native>>
  public function append(mixed $val): object;

  /* Adds the specified value to the end of this Vector using the next available
   * integer key.
   * @param mixed $val
   * @return object
   */
  <<__Native>>
  public function add(mixed $val): object;

  /* Adds the values produced by the specified Iterable to the end of this
   * Vector using the next available integer keys.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function addAll(mixed $iterable): object;

  /* Adds the keys of the specified KeyedContainer to the end of this Vector
   * using the next available integer keys.
   * @param mixed $container
   * @return object
   */
  <<__Native>>
  public function addAllKeysOf(mixed $container): object;

  /* @return mixed
   */
  <<__Native>>
  public function pop(): mixed;

  /* @param mixed $sz
   * @param mixed $value
   */
  <<__Native>>
  public function resize(mixed $sz,
                         mixed $value): void;

  /* Instructs this Vector to grow its capacity to accommodate the given number
   * of elements. The caller is expected to make the appropriate add/addAll
   * calls to fill that reserved capacity.
   * @param mixed $sz
   */
  <<__Native>>
  public function reserve(mixed $sz): void;

  /* Returns an array built from the values from this Vector.
   * @return array
   */
  <<__Native>>
  public function toArray(): array;

  /* Returns a copy of this Vector.
   * @return object
   */
  public function toVector(): this {
    return clone $this;
  }

  /* Returns a ImmVector built from the values of this Vector.
   * @return object
   */
  <<__Native>>
  public function toImmVector(): object;

  /* Returns an immutable version of this collection.
   * @return object
   */
  public function immutable(): \HH\ImmVector {
    return $this->toImmVector();
  }

  /* Returns a Map built from the keys and values of this Vector.
   * @return object
   */
  <<__Native>>
  public function toMap(): object;

  /* Returns a ImmMap built from the keys and values of this Vector.
   * @return object
   */
  <<__Native>>
  public function toImmMap(): object;

  /* Returns a Set built from the values of this Vector.
   * @return object
   */
  <<__Native>>
  public function toSet(): object;

  /* Returns a ImmSet built from the values of this Vector.
   * @return object
   */
  <<__Native>>
  public function toImmSet(): object;

  /* Returns an array built from the keys from this Vector.
   * @return array
   */
  public function toKeysArray(): array {
    $count = $this->count();
    return $count ? range(0, $count - 1) : array();
  }

  /* Returns an array built from the values from this Vector.
   * @return array
   */
  public function toValuesArray(): array {
    return $this->toArray();
  }

  /* Returns an iterator that points to beginning of this Vector.
   * @return object
   */
  <<__Native>>
  public function getIterator(): object;

  /* Returns a Vector of the values produced by applying the specified callback
   * on each value from this Vector.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function map(mixed $callback): object;

  /* Returns a Vector of the values produced by applying the specified callback
   * on each key and value from this Vector.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function mapWithKey(mixed $callback): object;

  /* Returns a Vector of all the values from this Vector for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function filter(mixed $callback): object;

  /* Returns a Vector of all the values from this Vector for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function filterWithKey(mixed $callback): object;

  /* Returns a KeyedIterable produced by combined the specified Iterables
   * pair-wise.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function zip(mixed $iterable): object;

  /* Returns a Vector containing the first n values of this Vector.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function take(mixed $n): object;

  /* Returns a Vector containing the values of this Vector up to but not
   * including the first value that produces false when passed to the specified
   * callback.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function takeWhile(mixed $callback): object;

  /* Returns a Vector containing all the values except the first n of this
   * Vector.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function skip(mixed $n): object;

  /* Returns a Vector containing the values of this Vector excluding the first
   * values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  <<__Native>>
  public function skipWhile(mixed $fn): object;

  /* Returns a Vector containing the specified range of values from this Vector.
   * The range is specified by two non-negative integers: a starting position
   * and a length.
   * @param mixed $start
   * @param mixed $len
   * @return object
   */
  <<__Native>>
  public function slice(mixed $start,
                        mixed $len): object;

  /* Builds a new Vector by concatenating the elements of this Vector with the
   * elements of the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function concat(mixed $iterable): object;

  /* Returns the first value from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  <<__Native>>
  public function firstValue(): mixed;

  /* Returns the first key from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  public function firstKey(): mixed {
    return $this->count() ? 0 : null;
  }

  /* Returns the last value from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  <<__Native>>
  public function lastValue(): mixed;

  /* Returns the last key from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  public function lastKey(): mixed {
    $count = $this->count();
    return $count ? ($count - 1) : null;
  }

  /* Reverses the values of the Vector in place.
   */
  <<__Native>>
  public function reverse(): void;

  /* Splices the values of the Vector in place (see the documentation for
   * array_splice() on php.net for more details.
   * @param mixed $offset
   * @param mixed $len
   * @param mixed $replacement
   */
  <<__Native>>
  public function splice(mixed $offset,
                         mixed $len = null,
                         mixed $replacement = null): void;

  /* Returns index of the specified value if it is present, -1 otherwise.
   * @param mixed $search_value
   * @return int
   */
  <<__Native>>
  public function linearSearch(mixed $search_value): int;

  /* Shuffles the values of the Vector randomly in place.
   */
  <<__Native>>
  public function shuffle(): void;

  /* @return string
   */
  public function __toString(): string { return "Vector"; }

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

  /* Returns a Vector built from the values produced by the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public static function fromItems(mixed $iterable): object;

  /* Returns a Vector built from the keys of the specified container.
   * @param mixed $container
   * @return object
   */
  <<__Native>>
  public static function fromKeysOf(mixed $container): object;

  /* Returns a Vector built from the values from the specified array.
   * @param mixed $arr
   * @return object
   */
  <<__Native>>
  public static function fromArray(mixed $arr): object;
}

/* An immutable ordered collection where values are keyed using integers 0
 * thru n-1 in order.
 */
final class ImmVector implements \ConstVector {

  /* Returns a ImmVector built from the values produced by the specified
   * Iterable.
   * @param mixed $iterable
   */
  <<__Native>>
  public function __construct(mixed $iterable = null): void;

  /* Returns an ImmVector built from the values produced by the specified
   * Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public static function fromItems(mixed $iterable): object;

  /* Returns a ImmVector built from the keys of the specified container.
   * @param mixed $container
   * @return object
   */
  <<__Native>>
  public static function fromKeysOf(mixed $container): object;

  /* Returns true if the ImmVector is empty, false otherwise.
   * @return bool
   */
  public function isEmpty(): bool {
    return !$this->count();
  }

  /* Returns the number of values in the ImmVector.
   * @return int
   */
  <<__Native>>
  public function count(): int;

  /* Returns an Iterable that produces the values from this ImmVector.
   * @return object
   */
  public function items(): \LazyIterableView {
    return new \LazyIterableView($this);
  }

  /* Returns true if the specified key is present in the ImmVector, returns
   * false otherwise.
   * @param mixed $key
   * @return bool
   */
  public function containsKey(mixed $key): bool {
    if (!\is_int($key)) {
      throw new \InvalidArgumentException(
        "Only integer keys may be used with Vectors"
      );
    }
    return ($key >= 0) && ($key < $this->count());
  }

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

  /* Returns an iterator that points to beginning of this ImmVector.
   * @return object
   */
  <<__Native>>
  public function getIterator(): object;

  /* Returns a Vector of the values produced by applying the specified callback
   * on each value from this ImmVector.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function map(mixed $callback): object;

  /* Returns a Vector of the values produced by applying the specified callback
   * on each key and value from this ImmVector.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function mapWithKey(mixed $callback): object;

  /* Returns a Vector of all the values from this ImmVector for which the
   * specified callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function filter(mixed $callback): object;

  /* Returns a Vector of all the values from this ImmVector for which the
   * specified callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function filterWithKey(mixed $callback): object;

  /* Returns a KeyedIterable produced by combined the specified Iterables
   * pair-wise.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function zip(mixed $iterable): object;

  /* Returns a ImmVector containing the first n values of this ImmVector.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function take(mixed $n): object;

  /* Returns a ImmVector containing the values of this ImmVector up to but not
   * including the first value that produces false when passed to the specified
   * callback.
   * @param mixed $callback
   * @return object
   */
  <<__Native>>
  public function takeWhile(mixed $callback): object;

  /* Returns a ImmVector containing all values except the first n of this
   * ImmVector.
   * @param mixed $n
   * @return object
   */
  <<__Native>>
  public function skip(mixed $n): object;

  /* Returns a ImmVector containing the values of this ImmVector excluding the
   * first values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  <<__Native>>
  public function skipWhile(mixed $fn): object;

  /* Returns an ImmVector containing the specified range of values from this
   * ImmVector. The range is specified by two non-negative integers: a starting
   * position and a length.
   * @param mixed $start
   * @param mixed $len
   * @return object
   */
  <<__Native>>
  public function slice(mixed $start,
                        mixed $len): object;

  /* Builds a new ImmVector by concatenating the elements of this ImmVector with
   * the elements of the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native>>
  public function concat(mixed $iterable): object;

  /* Returns the first value from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  <<__Native>>
  public function firstValue(): mixed;

  /* Returns the first key from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  public function firstKey(): mixed {
    return $this->count() ? 0 : null;
  }

  /* Returns the last value from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  <<__Native>>
  public function lastValue(): mixed;

  /* Returns the last key from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  public function lastKey(): mixed {
    $count = $this->count();
    return $count ? ($count - 1) : null;
  }

  /* Returns an Iterable that produces the keys from this ImmVector.
   * @return object
   */
  <<__Native>>
  public function keys(): object;

  /* @return string
   */
  public function __toString(): string { return "ImmVector"; }

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

  /* Returns a Vector built from the values of this ImmVector.
   * @return object
   */
  <<__Native>>
  public function toVector(): object;

  /* Returns an immutable version of this collection.
   * @return object
   */
  public function toImmVector(): this {
    return $this;
  }

  /* Returns a Map built from the keys and values of this ImmVector.
   * @return object
   */
  <<__Native>>
  public function toMap(): object;

  /* Returns a ImmMap built from the keys and values of this ImmVector.
   * @return object
   */
  <<__Native>>
  public function toImmMap(): object;

  /* Returns a Set built from the values of this ImmVector.
   * @return object
   */
  <<__Native>>
  public function toSet(): object;

  /* Returns a ImmSet built from the values of this ImmVector.
   * @return object
   */
  <<__Native>>
  public function toImmSet(): object;

  /* Returns an immutable version of this collection.
   * @return object
   */
  public function immutable(): this {
    return $this;
  }

  /* Returns a clone of this ImmVector.
   * @return object
   */
  public function values(): this {
    return clone $this;
  }

  /* Returns a lazy iterable view of this ImmVector.
   * @return object
   */
  public function lazy(): \LazyKeyedIterableView {
    return new \LazyKeyedIterableView($this);
  }

  /* Returns an array built from the values from this ImmVector.
   * @return array
   */
  <<__Native>>
  public function toArray(): array;

  /* Returns an array built from the keys from this ImmVector.
   * @return array
   */
  public function toKeysArray(): array {
    $count = $this->count();
    return $count ? range(0, $count - 1) : array();
  }

  /* Returns an array built from the values from this ImmVector.
   * @return array
   */
  public function toValuesArray(): array {
    return $this->toArray();
  }

  /* Returns index of the specified value if it is present, -1 otherwise.
   * @param mixed $search_value
   * @return int
   */
  <<__Native>>
  public function linearSearch(mixed $search_value): int;
}

} // namespace HH
