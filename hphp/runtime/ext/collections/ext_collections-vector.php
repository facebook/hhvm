<?hh // partial

namespace {

/** An iterator implementation for iterating over a Vector/ImmVector.
 */
<<__NativeData("VectorIterator")>>
final class VectorIterator implements HH\Rx\KeyedIterator {

  /** Do nothing */
  <<__Pure>>
  public function __construct(): void { }

  /** Returns the current value that the iterator points to.
   * @return mixed
   */
  <<__Native, __Pure, __MaybeMutable>>
  public function current(): mixed;

  /** Returns the current key that the iterator points to.
   * @return mixed
   */
  <<__Native, __Pure, __MaybeMutable>>
  public function key(): mixed;

  /** Returns true if the iterator points to a valid value, returns false
   * otherwise.
   * @return bool
   */
  <<__Native, __Pure, __MaybeMutable>>
  public function valid(): bool;

  /** Advance this iterator forward one position.
   */
  <<__Native, __Pure, __Mutable>>
  public function next(): void;

  /** Move this iterator back to the first position.
   */
  <<__Native, __Pure, __Mutable>>
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
  <<__Native, __Pure, __AtMostRxAsArgs>>
  public function __construct(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable = null): void;

  /** Returns true if the Vector is empty, false otherwise.
   * @return bool
   */
  <<__Pure, __MaybeMutable>>
  public function isEmpty(): bool {
    return !$this->count();
  }

  /** Returns the number of values in the Vector.
   * @return int
   */
  <<__Pure, __MaybeMutable>>
  public function count(): int {
    return \count(vec($this));
  }

  /** Returns an Iterable that produces the values from this Vector.
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function items(): \LazyIterableView {
    return new \LazyIterableView($this);
  }

  /** Returns a Vector built from the keys of this Vector.
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function keys(): this {
    return new self($this->toKeysArray());
  }

  /** Returns a copy of this Vector.
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function values(): this {
    return new self($this);
  }

  /** Returns a lazy iterable view of this Vector.
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function lazy(): \LazyKeyedIterableView {
    return new \LazyKeyedIterableView($this);
  }

  /** Returns the value at the specified key. If the key is not present, an
   * exception is thrown.
   * @param mixed $key
   * @return mixed
   */
  <<__Pure, __MaybeMutable>>
  public function at(mixed $key): mixed {
    return $this[$key];
  }

  /** Returns the value at the specified key. If the key is not present, null is
   * returned.
   * @param mixed $key
   * @return mixed
   */
  <<__Pure, __MaybeMutable>>
  public function get(mixed $key): mixed {
    return idx($this, $key);
  }

  /** Stores a value into the Vector with the specified key, overwriting any
   * previous value that was associated with the key; if the key is outside the
   * bounds of the Vector, an exception is thrown.
   * @param mixed $key
   * @param mixed $value
   * @return object
   */
  <<__Pure, __Mutable, __ReturnsVoidToRx>>
  public function set(mixed $key, mixed $value): this {
    $result = $this;
    $result[$key] = $value;
    return $result;
  }

  /** Stores each value produced by the specified KeyedIterable into the Vector
   * using its corresponding key, overwriting any previous value that was
   * associated with that key; if the key is outside the bounds of the Vector,
   * an exception is thrown.
   * @param mixed $iterable
   * @return object
   */
  <<__Pure, __Mutable, __AtMostRxAsArgs, __ReturnsVoidToRx>>
  public function setAll(<<__MaybeMutable, __OnlyRxIfImpl(Rx\KeyedTraversable::class)>> mixed $iterable): this {
    if ($iterable === null) {
      return $this;
    }
    foreach ($iterable as $key => $value) {
      $this->set($key, $value);
    }
    return $this;
  }

  /** Removes all values from the Vector.
   * @return object
   */
  <<__Native, __Pure, __Mutable, __ReturnsVoidToRx>>
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
  <<__Pure, __MaybeMutable>>
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
  <<__Native, __Pure, __Mutable, __ReturnsVoidToRx>>
  public function removeKey(mixed $key): object;

  /** @param mixed $value
   * @return object
   */
  public function append(mixed $value): this {
    $result = $this;
    $result[] = $value;
    return $result;
  }

  /** Adds the specified value to the end of this Vector using the next available
   * integer key.
   * @param mixed $value
   * @return object
   */
  <<__Pure, __Mutable, __ReturnsVoidToRx>>
  public function add(mixed $value): this {
    return $this->append($value);
  }

  /** Adds the values produced by the specified Iterable to the end of this
   * Vector using the next available integer keys.
   * @param mixed $iterable
   * @return object
   */
  <<__Pure, __Mutable, __AtMostRxAsArgs, __ReturnsVoidToRx>>
  public function addAll(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable): this {
    if ($iterable === null) {
      return $this;
    }
    foreach ($iterable as $value) {
      $this->append($value);
    }
    return $this;
  }

  /** Adds the keys of the specified KeyedContainer to the end of this Vector
   * using the next available integer keys.
   * @param mixed $container
   * @return object
   */
  <<__Pure, __Mutable, __ReturnsVoidToRx>>
  public function addAllKeysOf(mixed $container): this {
    if ($container === null) {
      return $this;
    }
    foreach ($container as $key => $_) {
      $this->append($key);
    }
    return $this;
  }

  /** @return mixed
   */
  <<__Native, __Pure, __Mutable>>
  public function pop(): mixed;

  /** @param mixed $size
   * @param mixed $value
   */
  <<__Native, __Pure, __Mutable>>
  public function resize(mixed $size, mixed $value): void;

  /** Instructs this Vector to grow its capacity to accommodate the given number
   * of elements. The caller is expected to make the appropriate add/addAll
   * calls to fill that reserved capacity.
   * @param mixed $sz
   */
  <<__Native, __Pure, __Mutable>>
  public function reserve(mixed $sz): void;

  /** Returns a varray built from the values from this Vector.
   * @return varray
   */
  <<__Pure, __MaybeMutable>>
  public function toVArray(): varray {
    return varray($this);
  }

  <<__Pure, __MaybeMutable>>
  public function toDArray(): darray {
    return darray($this);
  }

  /** Returns a copy of this Vector.
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function toVector(): Vector {
    return new self($this);
  }

  /** Returns a ImmVector built from the values of this Vector.
   * @return object
   */
  <<__Pure, __MaybeMutable>>
  public function toImmVector(): ImmVector {
    return new ImmVector($this);
  }

  /** Returns an immutable version of this collection.
   * @return object
   */
  <<__Pure, __MaybeMutable>>
  public function immutable(): ImmVector {
    return $this->toImmVector();
  }

  /** Returns a Map built from the keys and values of this Vector.
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function toMap(): Map {
    return new Map($this);
  }

  /** Returns a ImmMap built from the keys and values of this Vector.
   * @return object
   */
  <<__Pure, __MaybeMutable>>
  public function toImmMap(): ImmMap {
    return new ImmMap($this);
  }

  /** Returns a Set built from the values of this Vector.
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function toSet(): Set {
    return new Set($this);
  }

  /** Returns a ImmSet built from the values of this Vector.
   * @return object
   */
  <<__Pure, __MaybeMutable>>
  public function toImmSet(): ImmSet {
    return new ImmSet($this);
  }

  /** Returns a varray built from the keys from this Vector.
   * @return varray
   */
  <<__Pure, __MaybeMutable, __ProvenanceSkipFrame>>
  public function toKeysArray(): varray {
    $count = $this->count();
    return $count ? varray(\range(0, $count - 1)) : varray[];
  }

  /** Returns a varray built from the values from this Vector.
   * @return varray
   */
  <<__Pure, __MaybeMutable, __ProvenanceSkipFrame>>
  public function toValuesArray(): varray {
    return $this->toVArray();
  }

  /** Returns an iterator that points to beginning of this Vector.
   * @return object
   */
  <<__Native, __Pure, __MutableReturn, __MaybeMutable>>
  public function getIterator(): object;

  /** Returns a Vector of the values produced by applying the specified callback
   * on each value from this Vector.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable, __ProvenanceSkipFrame>>
  public function map(<<__AtMostRxAsFunc>> mixed $callback): Vector {
    $ret = vec[];
    foreach ($this as $v) {
      $ret[] = $callback($v);
    }
    return new Vector($ret);
  }

  /** Returns a Vector of the values produced by applying the specified callback
   * on each key and value from this Vector.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable, __ProvenanceSkipFrame>>
  public function mapWithKey(<<__AtMostRxAsFunc>> mixed $callback): Vector {
    $ret = vec[];
    foreach ($this as $k => $v) {
      $ret[] = $callback($k, $v);
    }
    return new Vector($ret);
  }

  /** Returns a Vector of all the values from this Vector for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filter(<<__AtMostRxAsFunc>> mixed $callback): Vector {
    $ret = vec[];
    foreach ($this as $v) {
      if ($callback($v)) {
        $ret[] = $v;
      }
    }
    return new Vector($ret);
  }

  /** Returns a Vector of all the values from this Vector for which the specified
   * callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filterWithKey(<<__AtMostRxAsFunc>> mixed $callback): Vector {
    $ret = vec[];
    foreach ($this as $k => $v) {
      if ($callback($k, $v)) {
        $ret[] = $v;
      }
    }
    return new Vector($ret);
  }

  /** Returns a KeyedIterable produced by combined the specified Iterables
   * pair-wise.
   * @param mixed $iterable
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function zip(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable): this {
    $i = 0;
    $result = vec[];
    $count = $this->count();
    foreach ($iterable as $value) {
      if ($i === $count) {
        break;
      }
      $result[] = Pair { $this[$i], $value };
      $i++;
    }
    return new self($result);
  }

  /** Returns a Vector containing the first n values of this Vector.
   * @param mixed $n
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function take(mixed $n): this {
    if (!($n is int)) {
      throw new InvalidArgumentException("Parameter n must be an integer");
    }
    $count = $this->count();
    $n = $n < 0 ? 0 : ($n > $count ? $count : $n);
    $result = vec[];
    for ($i = 0; $i < $n; $i++) {
      $result[] = $this[$i];
    }
    return new self($result);
  }

  /** Returns a Vector containing the values of this Vector up to but not
   * including the first value that produces false when passed to the specified
   * callback.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function takeWhile(<<__AtMostRxAsFunc>> mixed $callback): Vector {
    $ret = vec[];
    foreach ($this as $v) {
      if (!$callback($v)) {
        break;
      }
      $ret[] = $v;
    }
    return new Vector($ret);
  }

  /** Returns a Vector containing all the values except the first n of this
   * Vector.
   * @param mixed $n
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function skip(mixed $n): this {
    if (!($n is int)) {
      throw new InvalidArgumentException("Parameter n must be an integer");
    }
    $count = $this->count();
    $n = $n < 0 ? 0 : ($n > $count ? $count : $n);
    $result = vec[];
    for ($i = $n; $i < $count; $i++) {
      $result[] = $this[$i];
    }
    return new self($result);
  }

  /** Returns a Vector containing the values of this Vector excluding the first
   * values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function skipWhile(<<__AtMostRxAsFunc>> mixed $fn): Vector {
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
    return new Vector($ret);
  }

  /** Returns a Vector containing the specified range of values from this Vector.
   * The range is specified by two non-negative integers: a starting position
   * and a length.
   * @param mixed $start
   * @param mixed $len
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function slice(mixed $start, mixed $len): this {
    if (!($start is int) || $start < 0) {
      throw new InvalidArgumentException(
        "Parameter start must be a non-negative integer");
    }
    if (!($len is int) || $len < 0) {
      throw new InvalidArgumentException(
        "Parameter len must be a non-negative integer");
    }
    $count = $this->count();
    $skip = $start < $count ? $start : $count;
    $size = $len < $count - $skip ? $len : $count - $skip;
    $result = vec[];
    for ($i = $skip; $i < $skip + $size; $i++) {
      $result[] = $this[$i];
    }
    return new self($result);
  }

  /** Builds a new Vector by concatenating the elements of this Vector with the
   * elements of the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function concat(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable): this {
    $result = vec($this);
    foreach ($iterable as $value) {
      $result[] = $value;
    }
    return new self($result);
  }

  /** Returns the first value from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  <<__Pure, __MaybeMutable>>
  public function firstValue(): mixed {
    return idx($this, 0);
  }

  /** Returns the first key from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  <<__Pure, __MaybeMutable>>
  public function firstKey(): mixed {
    return $this->count() ? 0 : null;
  }

  /** Returns the last value from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  <<__Pure, __MaybeMutable>>
  public function lastValue(): mixed {
    $count = $this->count();
    return $count ? $this[$count - 1] : null;
  }

  /** Returns the last key from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  <<__Pure, __MaybeMutable>>
  public function lastKey(): mixed {
    $count = $this->count();
    return $count ? ($count - 1) : null;
  }

  /** Reverses the values of the Vector in place.
   */
  <<__Native, __Pure, __Mutable>>
  public function reverse(): void;

  /** Splices the values of the Vector in place (see the documentation for
   * array_splice() on php.net for more details.
   * @param mixed $offset
   * @param mixed $len
   * @param mixed $replacement
   */
  <<__Native, __Pure, __Mutable>>
  public function splice(mixed $offset,
                         mixed $len = null,
                         mixed $replacement = null): void;

  /** Returns index of the specified value if it is present, -1 otherwise.
   * @param mixed $search_value
   * @return int
   */
  <<__Pure, __MaybeMutable>>
  public function linearSearch(mixed $search_value): int {
    foreach (vec($this) as $i => $value) {
      if ($value === $search_value) {
        return $i;
      }
    }
    return -1;
  }

  /** Shuffles the values of the Vector randomly in place.
   */
  <<__Native, __Pure, __Mutable>>
  public function shuffle(): void;

  /** @return string
   */
  <<__Pure, __MaybeMutable>>
  public function __toString(): string { return "Vector"; }

  /** Returns a Vector built from the values produced by the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn>>
  public static function fromItems(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable): this {
    return new self($iterable);
  }

  /** Returns a Vector built from the keys of the specified container.
   * @param mixed $container
   * @return object
   */
  <<__Pure, __MutableReturn>>
  public static function fromKeysOf(mixed $container): this {
    if ($container is null) {
      return new self();
    } else if (!($container is KeyedContainer<_,_>)) {
      throw new \InvalidArgumentException(
        "Parameter must be a container (array or collection)");
    }
    $result = vec[];
    foreach ($container as $key => $_) {
      $result[] = $key;
    }
    return new self($result);
  }

  /** Returns a Vector built from the values from the specified array.
   * @param mixed $arr
   * @return object
   */
  public static function fromArray(mixed $arr): this {
    return new self($arr);
  }
}

/** An immutable ordered collection where values are keyed using integers 0
 * thru n-1 in order.
 */
final class ImmVector implements \ConstVector {

  /** Returns a ImmVector built from the values produced by the specified
   * Iterable.
   * @param mixed $iterable
   */
  <<__Native, __Pure, __AtMostRxAsArgs>>
  public function __construct(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable = null): void;

  /** Returns an ImmVector built from the values produced by the specified
   * Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs>>
  public static function fromItems(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable): this {
    return new self($iterable);
  }

  /** Returns a ImmVector built from the keys of the specified container.
   * @param mixed $container
   * @return object
   */
  <<__Pure>>
  public static function fromKeysOf(mixed $container): this {
    if ($container is null) {
      return new self();
    } else if (!($container is KeyedContainer<_,_>)) {
      throw new \InvalidArgumentException(
        "Parameter must be a container (array or collection)");
    }
    $result = vec[];
    foreach ($container as $key => $_) {
      $result[] = $key;
    }
    return new self($result);
  }

  /** Returns true if the ImmVector is empty, false otherwise.
   * @return bool
   */
  <<__Pure, __MaybeMutable>>
  public function isEmpty(): bool {
    return !$this->count();
  }

  /** Returns the number of values in the ImmVector.
   * @return int
   */
  <<__Pure, __MaybeMutable>>
  public function count(): int {
    return \count(vec($this));
  }

  /** Returns an Iterable that produces the values from this ImmVector.
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function items(): \LazyIterableView {
    return new \LazyIterableView($this);
  }

  /** Returns true if the specified key is present in the ImmVector, returns
   * false otherwise.
   * @param mixed $key
   * @return bool
   */
  <<__Pure, __MaybeMutable>>
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
  <<__Pure, __MaybeMutable>>
  public function at(mixed $key): mixed {
    return $this[$key];
  }

  /** Returns the value at the specified key. If the key is not present, null is
   * returned.
   * @param mixed $key
   * @return mixed
   */
  <<__Pure, __MaybeMutable>>
  public function get(mixed $key): mixed {
    return idx($this, $key);
  }

  /** Returns an iterator that points to beginning of this ImmVector.
   * @return object
   */
  <<__Native, __Pure, __MutableReturn, __MaybeMutable>>
  public function getIterator(): object;

  /** Returns a Vector of the values produced by applying the specified callback
   * on each value from this ImmVector.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable, __ProvenanceSkipFrame>>
  public function map(<<__AtMostRxAsFunc>> mixed $callback): ImmVector {
    $ret = vec[];
    foreach ($this as $v) {
      $ret[] = $callback($v);
    }
    return new ImmVector($ret);
  }

  /** Returns a Vector of the values produced by applying the specified callback
   * on each key and value from this ImmVector.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable, __ProvenanceSkipFrame>>
  public function mapWithKey(<<__AtMostRxAsFunc>> mixed $callback): ImmVector {
    $ret = vec[];
    foreach ($this as $k => $v) {
      $ret[] = $callback($k, $v);
    }
    return new ImmVector($ret);
  }

  /** Returns a Vector of all the values from this ImmVector for which the
   * specified callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filter(<<__AtMostRxAsFunc>> mixed $callback): ImmVector {
    $ret = vec[];
    foreach ($this as $v) {
      if ($callback($v)) {
        $ret[] = $v;
      }
    }
    return new ImmVector($ret);
  }

  /** Returns a Vector of all the values from this ImmVector for which the
   * specified callback returns true.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filterWithKey(<<__AtMostRxAsFunc>> mixed $callback): ImmVector {
    $ret = vec[];
    foreach ($this as $k => $v) {
      if ($callback($k, $v)) {
        $ret[] = $v;
      }
    }
    return new ImmVector($ret);
  }

  /** Returns a KeyedIterable produced by combined the specified Iterables
   * pair-wise.
   * @param mixed $iterable
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function zip(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable): this {
    $i = 0;
    $result = vec[];
    $count = $this->count();
    foreach ($iterable as $value) {
      if ($i === $count) {
        break;
      }
      $result[] = Pair { $this[$i], $value };
      $i++;
    }
    return new self($result);
  }

  /** Returns a ImmVector containing the first n values of this ImmVector.
   * @param mixed $n
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function take(mixed $n): this {
    if (!($n is int)) {
      throw new InvalidArgumentException("Parameter n must be an integer");
    }
    $count = $this->count();
    $n = $n < 0 ? 0 : ($n > $count ? $count : $n);
    $result = vec[];
    for ($i = 0; $i < $n; $i++) {
      $result[] = $this[$i];
    }
    return new self($result);
  }

  /** Returns a ImmVector containing the values of this ImmVector up to but not
   * including the first value that produces false when passed to the specified
   * callback.
   * @param mixed $callback
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function takeWhile(<<__AtMostRxAsFunc>> mixed $callback): ImmVector {
    $ret = vec[];
    foreach ($this as $v) {
      if (!$callback($v)) {
        break;
      }
      $ret[] = $v;
    }
    return new ImmVector($ret);
  }

  /** Returns a ImmVector containing all values except the first n of this
   * ImmVector.
   * @param mixed $n
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function skip(mixed $n): this {
    if (!($n is int)) {
      throw new InvalidArgumentException("Parameter n must be an integer");
    }
    $count = $this->count();
    $n = $n < 0 ? 0 : ($n > $count ? $count : $n);
    $result = vec[];
    for ($i = $n; $i < $count; $i++) {
      $result[] = $this[$i];
    }
    return new self($result);
  }

  /** Returns a ImmVector containing the values of this ImmVector excluding the
   * first values that produces true when passed to the specified callback.
   * @param mixed $fn
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function skipWhile(<<__AtMostRxAsFunc>> mixed $fn): ImmVector {
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
    return new ImmVector($ret);
  }

  /** Returns an ImmVector containing the specified range of values from this
   * ImmVector. The range is specified by two non-negative integers: a starting
   * position and a length.
   * @param mixed $start
   * @param mixed $len
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function slice(mixed $start, mixed $len): ImmVector {
    if (!($start is int) || $start < 0) {
      throw new InvalidArgumentException(
        "Parameter start must be a non-negative integer");
    }
    if (!($len is int) || $len < 0) {
      throw new InvalidArgumentException(
        "Parameter len must be a non-negative integer");
    }
    $count = $this->count();
    $skip = $start < $count ? $start : $count;
    $size = $len < $count - $skip ? $len : $count - $skip;
    $result = vec[];
    for ($i = $skip; $i < $skip + $size; $i++) {
      $result[] = $this[$i];
    }
    return new self($result);
  }

  /** Builds a new ImmVector by concatenating the elements of this ImmVector with
   * the elements of the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function concat(<<__MaybeMutable, __OnlyRxIfImpl(Rx\Traversable::class)>> mixed $iterable): this {
    $result = vec($this);
    foreach ($iterable as $value) {
      $result[] = $value;
    }
    return new self($result);
  }

  /** Returns the first value from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  <<__Pure, __MaybeMutable>>
  public function firstValue(): mixed {
    return idx($this, 0);
  }

  /** Returns the first key from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  <<__Pure, __MaybeMutable>>
  public function firstKey(): mixed {
    return $this->count() ? 0 : null;
  }

  /** Returns the last value from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  <<__Pure, __MaybeMutable>>
  public function lastValue(): mixed {
    $count = $this->count();
    return $count ? $this[$count - 1] : null;
  }

  /** Returns the last key from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  <<__Pure, __MaybeMutable>>
  public function lastKey(): mixed {
    $count = $this->count();
    return $count ? ($count - 1) : null;
  }

  /** Returns an Iterable that produces the keys from this ImmVector.
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function keys(): this {
    return new self($this->toKeysArray());
  }

  /** @return string
   */
  <<__Pure, __MaybeMutable>>
  public function __toString(): string { return "ImmVector"; }

  /** Returns a Vector built from the values of this ImmVector.
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function toVector(): Vector {
    return new Vector($this);
  }

  /** Returns an immutable version of this collection.
   * @return object
   */
  <<__Pure, __MaybeMutable>>
  public function toImmVector(): this {
    return $this;
  }

  /** Returns a Map built from the keys and values of this ImmVector.
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function toMap(): Map {
    return new Map($this);
  }

  /** Returns a ImmMap built from the keys and values of this ImmVector.
   * @return object
   */
  <<__Pure, __MaybeMutable>>
  public function toImmMap(): ImmMap {
    return new ImmMap($this);
  }

  /** Returns a Set built from the values of this ImmVector.
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function toSet(): Set {
    return new Set($this);
  }

  /** Returns a ImmSet built from the values of this ImmVector.
   * @return object
   */
  <<__Pure, __MaybeMutable>>
  public function toImmSet(): ImmSet {
    return new ImmSet($this);
  }

  /** Returns an immutable version of this collection.
   * @return object
   */
  <<__Pure, __MaybeMutable>>
  public function immutable(): this {
    return $this;
  }

  /** Returns a copy of this ImmVector.
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function values(): this {
    return new self($this);
  }

  /** Returns a lazy iterable view of this ImmVector.
   * @return object
   */
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function lazy(): \LazyKeyedIterableView {
    return new \LazyKeyedIterableView($this);
  }

  /** Returns a varray built from the values from this ImmVector.
   * @return varray
   */
  <<__Pure, __MaybeMutable>>
  public function toVArray(): varray {
    return varray($this);
  }

  <<__Pure, __MaybeMutable>>
  public function toDArray(): darray {
    return darray($this);
  }

  /** Returns a varray built from the keys from this ImmVector.
   * @return varray
   */
  <<__Pure, __ProvenanceSkipFrame>>
  public function toKeysArray(): varray {
    $count = $this->count();
    return $count ? varray(\range(0, $count - 1)) : varray[];
  }

  /** Returns a varray built from the values from this ImmVector.
   * @return varray
   */
  <<__Pure, __MaybeMutable, __ProvenanceSkipFrame>>
  public function toValuesArray(): varray {
    return $this->toVArray();
  }

  /** Returns index of the specified value if it is present, -1 otherwise.
   * @param mixed $search_value
   * @return int
   */
  <<__Pure, __MaybeMutable>>
  public function linearSearch(mixed $search_value): int {
    foreach (vec($this) as $i => $value) {
      if ($value === $search_value) {
        return $i;
      }
    }
    return -1;
  }
}

} // namespace HH
