<?hh // partial

namespace {

/** An iterator implementation for iterating over a Vector/ImmVector.
 */
<<__NativeData("VectorIterator")>>
final class VectorIterator implements HH\Rx\KeyedIterator {

  /** Do nothing */
  <<__Rx>>
  public function __construct(): void { }

  /** Returns the current value that the iterator points to.
   * @return mixed
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function current(): mixed;

  /** Returns the current key that the iterator points to.
   * @return mixed
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function key(): mixed;

  /** Returns true if the iterator points to a valid value, returns false
   * otherwise.
   * @return bool
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function valid(): bool;

  /** Advance this iterator forward one position.
   */
  <<__Native, __Rx, __Mutable>>
  public function next(): void;

  /** Move this iterator back to the first position.
   */
  <<__Native, __Rx, __Mutable>>
  public function rewind(): void;
}

} // empty namespace
namespace HH {

/** An ordered collection where values are keyed using integers 0 thru n-1 in
 * order.
 */
final class Vector implements \MutableVector {

  /** Returns a Vector built from the values produced by the specified Iterable.
   * @param mixed $iterable
   */
  <<__Native, __Rx, __AtMostRxAsArgs>>
  public function __construct(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable = null): void;

  /** Returns true if the Vector is empty, false otherwise.
   * @return bool
   */
  <<__Rx, __MaybeMutable>>
  public function isEmpty(): bool {
    return !$this->count();
  }

  /** Returns the number of values in the Vector.
   * @return int
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function count(): int;

  /** Returns an Iterable that produces the values from this Vector.
   * @return object
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function items(): \LazyIterableView {
    return new \LazyIterableView($this);
  }

  /** Returns a Vector built from the keys of this Vector.
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function keys(): object;

  /** Returns a copy of this Vector.
   * @return object
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function values(): this {
    return new self($this);
  }

  /** Returns a lazy iterable view of this Vector.
   * @return object
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function lazy(): \LazyKeyedIterableView {
    return new \LazyKeyedIterableView($this);
  }

  /** Returns the value at the specified key. If the key is not present, an
   * exception is thrown.
   * @param mixed $key
   * @return mixed
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function at(mixed $key): mixed;

  /** Returns the value at the specified key. If the key is not present, null is
   * returned.
   * @param mixed $key
   * @return mixed
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function get(mixed $key): mixed;

  /** Stores a value into the Vector with the specified key, overwriting any
   * previous value that was associated with the key; if the key is outside the
   * bounds of the Vector, an exception is thrown.
   * @param mixed $key
   * @param mixed $value
   * @return object
   */
  <<__Native, __Rx, __Mutable, __ReturnsVoidToRx>>
  public function set(mixed $key,
                      mixed $value): object;

  /** Stores each value produced by the specified KeyedIterable into the Vector
   * using its corresponding key, overwriting any previous value that was
   * associated with that key; if the key is outside the bounds of the Vector,
   * an exception is thrown.
   * @param mixed $iterable
   * @return object
   */
  <<__Native, __Rx, __Mutable, __AtMostRxAsArgs, __ReturnsVoidToRx>>
  public function setAll(<<__MaybeMutable, __OnlyRxIfImpl(Rx\KeyedTraversable::class)>> mixed $iterable): object;

  /** Removes all values from the Vector.
   * @return object
   */
  <<__Native, __Rx, __Mutable, __ReturnsVoidToRx>>
  public function clear(): object;

  /** Returns true if the specified key is present in the Vector, returns false
   * otherwise.
   * @param mixed $key
   * @return bool
   */
  <<__Deprecated(
    "Use Vector::containsKey() for key search or Vector::linearSearch() for value search"
  )>>
  public function contains(mixed $key): bool {
    if (!\is_int($key)) {
      throw new \InvalidArgumentException(
        "Only integer keys may be used with Vectors"
      );
    }
    return ($key >= 0) && ($key < $this->count());
  }

  /** Returns true if the specified key is present in the Vector, returns false
   * otherwise.
   * @param mixed $key
   * @return bool
   */
  <<__Rx, __MaybeMutable>>
  public function containsKey(mixed $key): bool {
    if (!\is_int($key)) {
      throw new \InvalidArgumentException(
        "Only integer keys may be used with Vectors"
      );
    }
    return ($key >= 0) && ($key < $this->count());
  }

  /** Removes the element with the specified key from this Vector and renumbers
   * the keys of all subsequent elements.
   * @param mixed $key
   * @return object
   */
  <<__Native, __Rx, __Mutable, __ReturnsVoidToRx>>
  public function removeKey(mixed $key): object;

  /** @param mixed $val
   * @return object
   */
  <<__Native>>
  public function append(mixed $val): object;

  /** Adds the specified value to the end of this Vector using the next available
   * integer key.
   * @param mixed $val
   * @return object
   */
  <<__Native, __Rx, __Mutable, __ReturnsVoidToRx>>
  public function add(mixed $val): object;

  /** Adds the values produced by the specified Iterable to the end of this
   * Vector using the next available integer keys.
   * @param mixed $iterable
   * @return object
   */
  <<__Native, __Rx, __Mutable, __AtMostRxAsArgs, __ReturnsVoidToRx>>
  public function addAll(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable): object;

  /** Adds the keys of the specified KeyedContainer to the end of this Vector
   * using the next available integer keys.
   * @param mixed $container
   * @return object
   */
  <<__Native, __Rx, __Mutable, __ReturnsVoidToRx>>
  public function addAllKeysOf(mixed $container): object;

  /** @return mixed
   */
  <<__Native, __Rx, __Mutable>>
  public function pop(): mixed;

  /** @param mixed $sz
   * @param mixed $value
   */
  <<__Native, __Rx, __Mutable>>
  public function resize(mixed $sz,
                         mixed $value): void;

  /** Instructs this Vector to grow its capacity to accommodate the given number
   * of elements. The caller is expected to make the appropriate add/addAll
   * calls to fill that reserved capacity.
   * @param mixed $sz
   */
  <<__Native, __Rx, __Mutable>>
  public function reserve(mixed $sz): void;

  /** Returns an array built from the values from this Vector.
   * @return array
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function toArray(): array;

  <<__Native, __Rx, __MaybeMutable>>
  public function toVArray(): varray;

  <<__Native, __Rx, __MaybeMutable>>
  public function toDArray(): darray;

  /** Returns a copy of this Vector.
   * @return object
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function toVector(): this {
    return new self($this);
  }

  /** Returns a ImmVector built from the values of this Vector.
   * @return object
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function toImmVector(): object;

  /** Returns an immutable version of this collection.
   * @return object
   */
  <<__Rx, __MaybeMutable>>
  public function immutable(): \HH\ImmVector {
    return $this->toImmVector();
  }

  /** Returns a Map built from the keys and values of this Vector.
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function toMap(): object;

  /** Returns a ImmMap built from the keys and values of this Vector.
   * @return object
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function toImmMap(): object;

  /** Returns a Set built from the values of this Vector.
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function toSet(): object;

  /** Returns a ImmSet built from the values of this Vector.
   * @return object
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function toImmSet(): object;

  /** Returns an array built from the keys from this Vector.
   * @return array
   */
  <<__Rx, __MaybeMutable>>
  public function toKeysArray(): varray {
    $count = $this->count();
    return $count ? varray(\range(0, $count - 1)) : varray[];
  }

  /** Returns an array built from the values from this Vector.
   * @return array
   */
  <<__Rx, __MaybeMutable>>
  public function toValuesArray(): varray {
    return $this->toVArray();
  }

  /** Returns an iterator that points to beginning of this Vector.
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function getIterator(): object;

  /** Returns a Vector of the values produced by applying the specified callback
   * on each value from this Vector.
   * @param mixed $callback
   * @return object
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function map(<<__AtMostRxAsFunc>> mixed $callback): \HH\Vector {
    $ret = vec[];
    foreach ($this as $v) {
      $ret[] = $callback($v);
    }
    return new \HH\Vector($ret);
  }

  /** Returns a Vector of the values produced by applying the specified callback
   * on each key and value from this Vector.
   * @param mixed $callback
   * @return object
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function mapWithKey(<<__AtMostRxAsFunc>> mixed $callback): \HH\Vector {
    $ret = vec[];
    foreach ($this as $k => $v) {
      $ret[] = $callback($k, $v);
    }
    return new \HH\Vector($ret);
  }

  /** Returns a Vector of all the values from this Vector for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filter(<<__AtMostRxAsFunc>> mixed $callback): \HH\Vector {
    $ret = vec[];
    foreach ($this as $v) {
      if ($callback($v)) {
        $ret[] = $v;
      }
    }
    return new \HH\Vector($ret);
  }

  /** Returns a Vector of all the values from this Vector for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filterWithKey(<<__AtMostRxAsFunc>> mixed $callback): \HH\Vector {
    $ret = vec[];
    foreach ($this as $k => $v) {
      if ($callback($k, $v)) {
        $ret[] = $v;
      }
    }
    return new \HH\Vector($ret);
  }

  /** Returns a KeyedIterable produced by combined the specified Iterables
   * pair-wise.
   * @param mixed $iterable
   * @return object
   */
  <<__Native, __Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function zip(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable): object;

  /** Returns a Vector containing the first n values of this Vector.
   * @param mixed $n
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function take(mixed $n): object;

  /** Returns a Vector containing the values of this Vector up to but not
   * including the first value that produces false when passed to the specified
   * callback.
   * @param mixed $callback
   * @return object
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function takeWhile(<<__AtMostRxAsFunc>> mixed $callback): \HH\Vector {
    $ret = vec[];
    foreach ($this as $v) {
      if (!$callback($v)) {
        break;
      }
      $ret[] = $v;
    }
    return new \HH\Vector($ret);
  }

  /** Returns a Vector containing all the values except the first n of this
   * Vector.
   * @param mixed $n
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function skip(mixed $n): object;

  /** Returns a Vector containing the values of this Vector excluding the first
   * values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function skipWhile(<<__AtMostRxAsFunc>> mixed $fn): \HH\Vector {
    $ret = vec[];
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
    return new \HH\Vector($ret);
  }

  /** Returns a Vector containing the specified range of values from this Vector.
   * The range is specified by two non-negative integers: a starting position
   * and a length.
   * @param mixed $start
   * @param mixed $len
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function slice(mixed $start,
                        mixed $len): object;

  /** Builds a new Vector by concatenating the elements of this Vector with the
   * elements of the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native, __Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function concat(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable): object;

  /** Returns the first value from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function firstValue(): mixed;

  /** Returns the first key from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  <<__Rx, __MaybeMutable>>
  public function firstKey(): mixed {
    return $this->count() ? 0 : null;
  }

  /** Returns the last value from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function lastValue(): mixed;

  /** Returns the last key from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  <<__Rx, __MaybeMutable>>
  public function lastKey(): mixed {
    $count = $this->count();
    return $count ? ($count - 1) : null;
  }

  /** Reverses the values of the Vector in place.
   */
  <<__Native, __Rx, __Mutable>>
  public function reverse(): void;

  /** Splices the values of the Vector in place (see the documentation for
   * array_splice() on php.net for more details.
   * @param mixed $offset
   * @param mixed $len
   * @param mixed $replacement
   */
  <<__Native, __Rx, __Mutable>>
  public function splice(mixed $offset,
                         mixed $len = null,
                         mixed $replacement = null): void;

  /** Returns index of the specified value if it is present, -1 otherwise.
   * @param mixed $search_value
   * @return int
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function linearSearch(mixed $search_value): int;

  /** Shuffles the values of the Vector randomly in place.
   */
  <<__Native, __Rx, __Mutable>>
  public function shuffle(): void;

  /** @return string
   */
  <<__Rx, __MaybeMutable>>
  public function __toString(): string { return "Vector"; }

  /** @param mixed $name
   * @return mixed
   */
  public function __get(mixed $name): mixed {
    throw new \InvalidOperationException(
      "Cannot access a property on a collection"
    );
  }

  /** @param mixed $name
   * @param mixed $value
   * @return mixed
   */
  public function __set(mixed $name,
                        mixed $value): mixed {
    throw new \InvalidOperationException(
      "Cannot access a property on a collection"
    );
  }

  /** @param mixed $name
   * @return bool
   */
  public function __isset(mixed $name): bool { return false; }

  /** @param mixed $name
   * @return mixed
   */
  public function __unset(mixed $name): mixed {
    throw new \InvalidOperationException(
      "Cannot access a property on a collection"
    );
  }

  /** Returns a Vector built from the values produced by the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native, __Rx, __AtMostRxAsArgs, __MutableReturn>>
  public static function fromItems(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable): object;

  /** Returns a Vector built from the keys of the specified container.
   * @param mixed $container
   * @return object
   */
  <<__Native, __Rx, __MutableReturn>>
  public static function fromKeysOf(mixed $container): object;

  /** Returns a Vector built from the values from the specified array.
   * @param mixed $arr
   * @return object
   */
  <<__Native>>
  public static function fromArray(mixed $arr): object;
}

/** An immutable ordered collection where values are keyed using integers 0
 * thru n-1 in order.
 */
final class ImmVector implements \ConstVector {

  /** Returns a ImmVector built from the values produced by the specified
   * Iterable.
   * @param mixed $iterable
   */
  <<__Native, __Rx, __AtMostRxAsArgs>>
  public function __construct(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable = null): void;

  /** Returns an ImmVector built from the values produced by the specified
   * Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native, __Rx, __AtMostRxAsArgs>>
  public static function fromItems(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable): object;

  /** Returns a ImmVector built from the keys of the specified container.
   * @param mixed $container
   * @return object
   */
  <<__Native, __Rx>>
  public static function fromKeysOf(mixed $container): object;

  /** Returns true if the ImmVector is empty, false otherwise.
   * @return bool
   */
  <<__Rx, __MaybeMutable>>
  public function isEmpty(): bool {
    return !$this->count();
  }

  /** Returns the number of values in the ImmVector.
   * @return int
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function count(): int;

  /** Returns an Iterable that produces the values from this ImmVector.
   * @return object
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function items(): \LazyIterableView {
    return new \LazyIterableView($this);
  }

  /** Returns true if the specified key is present in the ImmVector, returns
   * false otherwise.
   * @param mixed $key
   * @return bool
   */
  <<__Rx, __MaybeMutable>>
  public function containsKey(mixed $key): bool {
    if (!\is_int($key)) {
      throw new \InvalidArgumentException(
        "Only integer keys may be used with Vectors"
      );
    }
    return ($key >= 0) && ($key < $this->count());
  }

  /** Returns the value at the specified key. If the key is not present, an
   * exception is thrown.
   * @param mixed $key
   * @return mixed
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function at(mixed $key): mixed;

  /** Returns the value at the specified key. If the key is not present, null is
   * returned.
   * @param mixed $key
   * @return mixed
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function get(mixed $key): mixed;

  /** Returns an iterator that points to beginning of this ImmVector.
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function getIterator(): object;

  /** Returns a Vector of the values produced by applying the specified callback
   * on each value from this ImmVector.
   * @param mixed $callback
   * @return object
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function map(<<__AtMostRxAsFunc>> mixed $callback): \HH\ImmVector {
    $ret = vec[];
    foreach ($this as $v) {
      $ret[] = $callback($v);
    }
    return new \HH\ImmVector($ret);
  }

  /** Returns a Vector of the values produced by applying the specified callback
   * on each key and value from this ImmVector.
   * @param mixed $callback
   * @return object
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function mapWithKey(<<__AtMostRxAsFunc>> mixed $callback): \HH\ImmVector {
    $ret = vec[];
    foreach ($this as $k => $v) {
      $ret[] = $callback($k, $v);
    }
    return new \HH\ImmVector($ret);
  }

  /** Returns a Vector of all the values from this ImmVector for which the
   * specified callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filter(<<__AtMostRxAsFunc>> mixed $callback): \HH\ImmVector {
    $ret = vec[];
    foreach ($this as $v) {
      if ($callback($v)) {
        $ret[] = $v;
      }
    }
    return new \HH\ImmVector($ret);
  }

  /** Returns a Vector of all the values from this ImmVector for which the
   * specified callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filterWithKey(<<__AtMostRxAsFunc>> mixed $callback): \HH\ImmVector {
    $ret = vec[];
    foreach ($this as $k => $v) {
      if ($callback($k, $v)) {
        $ret[] = $v;
      }
    }
    return new \HH\ImmVector($ret);
  }

  /** Returns a KeyedIterable produced by combined the specified Iterables
   * pair-wise.
   * @param mixed $iterable
   * @return object
   */
  <<__Native, __Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function zip(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable): object;

  /** Returns a ImmVector containing the first n values of this ImmVector.
   * @param mixed $n
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function take(mixed $n): object;

  /** Returns a ImmVector containing the values of this ImmVector up to but not
   * including the first value that produces false when passed to the specified
   * callback.
   * @param mixed $callback
   * @return object
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function takeWhile(<<__AtMostRxAsFunc>> mixed $callback): \HH\ImmVector {
    $ret = vec[];
    foreach ($this as $v) {
      if (!$callback($v)) {
        break;
      }
      $ret[] = $v;
    }
    return new \HH\ImmVector($ret);
  }

  /** Returns a ImmVector containing all values except the first n of this
   * ImmVector.
   * @param mixed $n
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function skip(mixed $n): object;

  /** Returns a ImmVector containing the values of this ImmVector excluding the
   * first values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function skipWhile(<<__AtMostRxAsFunc>> mixed $fn): \HH\ImmVector {
    $ret = vec[];
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
    return new \HH\ImmVector($ret);
  }

  /** Returns an ImmVector containing the specified range of values from this
   * ImmVector. The range is specified by two non-negative integers: a starting
   * position and a length.
   * @param mixed $start
   * @param mixed $len
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function slice(mixed $start,
                        mixed $len): object;

  /** Builds a new ImmVector by concatenating the elements of this ImmVector with
   * the elements of the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Native, __Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function concat(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable): object;

  /** Returns the first value from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function firstValue(): mixed;

  /** Returns the first key from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  <<__Rx, __MaybeMutable>>
  public function firstKey(): mixed {
    return $this->count() ? 0 : null;
  }

  /** Returns the last value from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function lastValue(): mixed;

  /** Returns the last key from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  <<__Rx, __MaybeMutable>>
  public function lastKey(): mixed {
    $count = $this->count();
    return $count ? ($count - 1) : null;
  }

  /** Returns an Iterable that produces the keys from this ImmVector.
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function keys(): object;

  /** @return string
   */
  <<__Rx, __MaybeMutable>>
  public function __toString(): string { return "ImmVector"; }

  /** @param mixed $name
   * @return mixed
   */
  public function __get(mixed $name): mixed {
    throw new \InvalidOperationException(
      "Cannot access a property on a collection"
    );
  }

  /** @param mixed $name
   * @param mixed $value
   * @return mixed
   */
  public function __set(mixed $name,
                        mixed $value): mixed {
    throw new \InvalidOperationException(
      "Cannot access a property on a collection"
    );
  }

  /** @param mixed $name
   * @return bool
   */
  public function __isset(mixed $name): bool { return false; }

  /** @param mixed $name
   * @return mixed
   */
  public function __unset(mixed $name): mixed {
    throw new \InvalidOperationException(
      "Cannot access a property on a collection"
    );
  }

  /** Returns a Vector built from the values of this ImmVector.
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function toVector(): object;

  /** Returns an immutable version of this collection.
   * @return object
   */
  <<__Rx, __MaybeMutable>>
  public function toImmVector(): this {
    return $this;
  }

  /** Returns a Map built from the keys and values of this ImmVector.
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function toMap(): object;

  /** Returns a ImmMap built from the keys and values of this ImmVector.
   * @return object
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function toImmMap(): object;

  /** Returns a Set built from the values of this ImmVector.
   * @return object
   */
  <<__Native, __Rx, __MutableReturn, __MaybeMutable>>
  public function toSet(): object;

  /** Returns a ImmSet built from the values of this ImmVector.
   * @return object
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function toImmSet(): object;

  /** Returns an immutable version of this collection.
   * @return object
   */
  <<__Rx, __MaybeMutable>>
  public function immutable(): this {
    return $this;
  }

  /** Returns a copy of this ImmVector.
   * @return object
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function values(): this {
    return new self($this);
  }

  /** Returns a lazy iterable view of this ImmVector.
   * @return object
   */
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function lazy(): \LazyKeyedIterableView {
    return new \LazyKeyedIterableView($this);
  }

  /** Returns an array built from the values from this ImmVector.
   * @return array
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function toArray(): array;

  <<__Native, __Rx, __MaybeMutable>>
  public function toVArray(): varray;

  <<__Native, __Rx, __MaybeMutable>>
  public function toDArray(): darray;

  /** Returns an array built from the keys from this ImmVector.
   * @return array
   */
  <<__Rx>>
  public function toKeysArray(): varray {
    $count = $this->count();
    return $count ? varray(\range(0, $count - 1)) : varray[];
  }

  /** Returns an array built from the values from this ImmVector.
   * @return array
   */
  <<__Rx, __MaybeMutable>>
  public function toValuesArray(): varray {
    return $this->toVArray();
  }

  /** Returns index of the specified value if it is present, -1 otherwise.
   * @param mixed $search_value
   * @return int
   */
  <<__Native, __Rx, __MaybeMutable>>
  public function linearSearch(mixed $search_value): int;
}

} // namespace HH
