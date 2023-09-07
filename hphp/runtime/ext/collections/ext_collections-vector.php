<?hh // partial
<<file:__EnableUnstableFeatures('readonly')>>

namespace {

/** An iterator implementation for iterating over a Vector/ImmVector.
 */
<<__NativeData>>
final class VectorIterator<T> implements HH\KeyedIterator<int, T> {

  /** Do nothing */
  public function __construct()[]: void { }

  /** Returns the current value that the iterator points to.
   * @return mixed
   */
  <<__Native>>
  public function current()[]: T;

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
  public function next()[write_props]: void;

  /** Move this iterator back to the first position.
   */
  <<__Native>>
  public function rewind()[write_props]: void;
}

} // empty namespace
namespace HH {

/** An ordered collection where values are keyed using integers 0 thru n-1 in
 * order.
 */
final class Vector<T> implements \MutableVector<T> {

  /** Returns a Vector built from the values produced by the specified Iterable.
   * @param mixed $iterable
   */
  <<__Native>>
  public function __construct(mixed $iterable = null)[]: void;

  /** Returns true if the Vector is empty, false otherwise.
   * @return bool
   */
  public readonly function isEmpty()[]: bool {
    return !$this->count();
  }

  /** Returns the number of values in the Vector.
   * @return int
   */
  public readonly function count()[]: int {
    return \count(vec($this));
  }

  /** Returns an Iterable that produces the values from this Vector.
   * @return object
   */
  /* HH_FIXME[2049] */
  public function items()[]: \LazyIterableView {
    /* HH_FIXME[2049] */
    return new \LazyIterableView($this);
  }

  /** Returns a Vector built from the keys of this Vector.
   * @return object
   */
  public readonly function keys()[]: Vector<int> {
    return new self($this->toKeysArray());
  }

  /** Returns a copy of this Vector.
   * @return object
   */
  public function values()[]: this {
    return new self($this);
  }

  /** Returns a lazy iterable view of this Vector.
   * @return object
   */
  /* HH_FIXME[2049] */
  public function lazy()[]: \LazyKeyedIterableView {
    /* HH_FIXME[2049] */
    return new \LazyKeyedIterableView($this);
  }

  /** Returns the value at the specified key. If the key is not present, an
   * exception is thrown.
   * @param mixed $key
   * @return mixed
   */
  public function at(mixed $key)[]: T {
    return $this[HH\FIXME\UNSAFE_CAST<mixed, int>($key)];
  }

  /** Returns the value at the specified key. If the key is not present, null is
   * returned.
   * @param mixed $key
   * @return mixed
   */
  public function get(mixed $key)[]: ?T {
    // TODO(T125421081) `idx` is special cased in the typechecker, I don't think
    // it quite understands how to deal with the input being `$this`, for some
    // reason.
    return HH\FIXME\UNSAFE_CAST<mixed, ?T>(idx($this, $key));
  }

  /** Stores a value into the Vector with the specified key, overwriting any
   * previous value that was associated with the key; if the key is outside the
   * bounds of the Vector, an exception is thrown.
   * @param mixed $key
   * @param mixed $value
   * @return object
   */
  public function set(mixed $key, T $value)[write_props]: this {
    $result = $this;
    $result[HH\FIXME\UNSAFE_CAST<mixed, int>($key)] = $value;
    return $result;
  }

  /** Stores each value produced by the specified KeyedIterable into the Vector
   * using its corresponding key, overwriting any previous value that was
   * associated with that key; if the key is outside the bounds of the Vector,
   * an exception is thrown.
   * @param mixed $iterable
   * @return object
   */
  public function setAll(mixed $iterable)[write_props]: this {
    if ($iterable === null) {
      return $this;
    }
    foreach (
      HH\FIXME\UNSAFE_CAST<nonnull, KeyedTraversable<int, T>>($iterable)
        as $key => $value
    ) {
      $this->set($key, $value);
    }
    return $this;
  }

  <<__Native>>
  private function clearNative()[write_props]: void;

  /** Removes all values from the Vector.
   * @return this
   */
  public function clear()[write_props]: this {
    $this->clearNative();
    return $this;
  }

  /** Returns true if the specified key is present in the Vector, returns false
   * otherwise.
   * @param mixed $key
   * @return bool
   */
  <<__Deprecated(
    "Use Vector::containsKey() for key search or Vector::linearSearch() for value search"
  )>>
  public readonly function contains(mixed $key): bool {
    if (!($key is int)) {
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
  public readonly function containsKey(mixed $key)[]: bool {
    if (!($key is int)) {
      throw new \InvalidArgumentException(
        "Only integer keys may be used with Vectors"
      );
    }
    return ($key >= 0) && ($key < $this->count());
  }

  <<__Native>>
  private function removeKeyNative(mixed $key)[write_props]: void;

  /** Removes the element with the specified key from this Vector and renumbers
   * the keys of all subsequent elements.
   * @param mixed $key
   * @return object
   */
  public function removeKey(mixed $key)[write_props]: this {
    $this->removeKeyNative($key);
    return $this;
  }

  /** @param mixed $value
   * @return object
   */
  public function append(T $value)[write_props]: this {
    $result = $this;
    $result[] = $value;
    return $result;
  }

  /** Adds the specified value to the end of this Vector using the next available
   * integer key.
   * @param mixed $value
   * @return object
   */
  public function add(T $value)[write_props]: this {
    return $this->append($value);
  }

  /** Adds the values produced by the specified Iterable to the end of this
   * Vector using the next available integer keys.
   * @param mixed $iterable
   * @return object
   */
  public function addAll(mixed $iterable)[write_props]: this {
    if ($iterable === null) {
      return $this;
    }
    foreach (
      HH\FIXME\UNSAFE_CAST<nonnull, KeyedTraversable<mixed, T>>($iterable)
        as $value
    ) {
      $this->append($value);
    }
    return $this;
  }

  /** Adds the keys of the specified KeyedContainer to the end of this Vector
   * using the next available integer keys.
   * @param mixed $container
   * @return object
   */
  public function addAllKeysOf(
    mixed $container,
  )[write_props]: this {
    if ($container === null) {
      return $this;
    }
    foreach (
      HH\FIXME\UNSAFE_CAST<nonnull, KeyedTraversable<T, mixed>>($container)
        as $key => $_
    ) {
      $this->append($key);
    }
    return $this;
  }

  /** @return mixed
   */
  <<__Native>>
  public function pop()[write_props]: mixed;

  /** @param mixed $size
   * @param mixed $value
   */
  <<__Native>>
  public function resize(mixed $size, mixed $value)[write_props]: void;

  /** Instructs this Vector to grow its capacity to accommodate the given number
   * of elements. The caller is expected to make the appropriate add/addAll
   * calls to fill that reserved capacity.
   * @param mixed $sz
   */
  <<__Native>>
  public function reserve(mixed $sz)[]: void;

  /** Returns a varray built from the values from this Vector.
   * @return varray
   */
  public function toVArray()[]: varray<T> {
    return varray($this);
  }

  public function toDArray()[]: darray<int, T> {
    return darray($this);
  }

  /** Returns a copy of this Vector.
   * @return object
   */
  public function toVector()[]: Vector<T> {
    return new self($this);
  }

  /** Returns a ImmVector built from the values of this Vector.
   * @return object
   */
  public function toImmVector()[]: ImmVector<T> {
    return new ImmVector($this);
  }

  /** Returns an immutable version of this collection.
   * @return object
   */
  public function immutable()[]: ImmVector<T> {
    return $this->toImmVector();
  }

  /** Returns a Map built from the keys and values of this Vector.
   * @return object
   */
  /* HH_FIXME[2049] */
  public function toMap()[]: Map<int, T> {
    /* HH_FIXME[2049] */
    return new Map($this);
  }

  /** Returns a ImmMap built from the keys and values of this Vector.
   * @return object
   */
  /* HH_FIXME[2049] */
  public function toImmMap()[]: ImmMap<int, T> {
    /* HH_FIXME[2049] */
    return new ImmMap($this);
  }

  /** Returns a Set built from the values of this Vector.
   * @return object
   */
  /* HH_FIXME[2049] */
  public function toSet()[]: Set<T> where T as arraykey {
    /* HH_FIXME[2049] */
    return new Set($this);
  }

  /** Returns a ImmSet built from the values of this Vector.
   * @return object
   */
  /* HH_FIXME[2049] */
  public function toImmSet()[]: ImmSet<T> where T as arraykey {
    /* HH_FIXME[2049] */
    return new ImmSet($this);
  }

  /** Returns a varray built from the keys from this Vector.
   * @return varray
   */
  public readonly function toKeysArray()[]: varray<int> {
    $count = $this->count();
    return $count
      ? varray(HH\FIXME\UNSAFE_CAST<mixed, varray<int>>(\range(0, $count - 1)))
      : varray[];
  }

  /** Returns a varray built from the values from this Vector.
   * @return varray
   */
  public function toValuesArray()[]: varray<T> {
    return $this->toVArray();
  }

  /** Returns an iterator that points to beginning of this Vector.
   * @return object
   */
  <<__Native>>
  public function getIterator()[]: Iterator<T>;

  /** Returns a Vector of the values produced by applying the specified callback
   * on each value from this Vector.
   * @param mixed $callback
   * @return object
   */
  public function map<To>(
    (function(T)[_]: To) $callback,
  )[ctx $callback]: Vector<To> {
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
  public function mapWithKey<To>(
    (function(int, T)[_]: To) $callback,
  )[ctx $callback]: Vector<To> {
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
  public function filter(
    (function(T)[_]: bool) $callback,
  )[ctx $callback]: this {
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
  public function filterWithKey(
    (function(int, T)[_]: bool) $callback,
  )[ctx $callback]: this {
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
  public function zip<To>(
    mixed $iterable,
  )[]: Vector<Pair<T, To>> {
    $i = 0;
    $result = vec[];
    $count = $this->count();
    foreach (
      HH\FIXME\UNSAFE_CAST<mixed, Traversable<To>>($iterable) as $value
    ) {
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
  public function take(mixed $n)[]: this {
    if (!($n is int)) {
      throw new \InvalidArgumentException("Parameter n must be an integer");
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
  public function takeWhile(
    (function(T)[_]: bool) $callback,
  )[ctx $callback]: this {
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
  public function skip(mixed $n)[]: this {
    if (!($n is int)) {
      throw new \InvalidArgumentException("Parameter n must be an integer");
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
  public function skipWhile((function(T)[_]: bool) $fn)[ctx $fn]: this {
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
  public function slice(mixed $start, mixed $len)[]: this {
    if (!($start is int) || $start < 0) {
      throw new \InvalidArgumentException(
        "Parameter start must be a non-negative integer");
    }
    if (!($len is int) || $len < 0) {
      throw new \InvalidArgumentException(
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
  public function concat<To super T>(
    mixed $iterable,
  )[]: Vector<To> {
    $result = vec($this);
    foreach (
      HH\FIXME\UNSAFE_CAST<mixed, Traversable<To>>($iterable) as $value
    ) {
      $result[] = $value;
    }
    return new self($result);
  }

  /** Returns the first value from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  public function firstValue()[]: mixed {
    return idx($this, 0);
  }

  /** Returns the first key from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  public readonly function firstKey()[]: mixed {
    return $this->count() ? 0 : null;
  }

  /** Returns the last value from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  public function lastValue()[]: mixed {
    $count = $this->count();
    return $count ? $this[$count - 1] : null;
  }

  /** Returns the last key from this Vector, or null if this Vector is empty.
   * @return mixed
   */
  public readonly function lastKey()[]: mixed {
    $count = $this->count();
    return $count ? ($count - 1) : null;
  }

  /** Reverses the values of the Vector in place.
   */
  <<__Native>>
  public function reverse()[write_props]: void;

  /** Splices the values of the Vector in place (see the documentation for
   * array_splice() on php.net for more details.
   * @param mixed $offset
   * @param mixed $len
   * @param mixed $replacement
   */
  <<__Native>>
  public function splice(mixed $offset,
                         mixed $len = null,
                         mixed $replacement = null)[write_props]: void;

  /** Returns index of the specified value if it is present, -1 otherwise.
   * @param mixed $search_value
   * @return int
   */
  public readonly function linearSearch(mixed $search_value)[]: int {
    foreach (vec($this) as $i => $value) {
      if ($value === $search_value) {
        return $i;
      }
    }
    return -1;
  }

  /** Shuffles the values of the Vector randomly in place.
   */
  <<__Native>>
  public function shuffle()[leak_safe]: void;

  /** @return string
   */
  public function __toString()[]: string { return "Vector"; }

  /** Returns a Vector built from the values produced by the specified Iterable.
   * @param mixed $iterable
   * @return object
   */
  public static function fromItems(mixed $iterable)[]: this {
    return new self($iterable);
  }

  /** Returns a Vector built from the keys of the specified container.
   * @param mixed $container
   * @return object
   */
  public static function fromKeysOf(mixed $container)[]: this {
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
final class ImmVector<T> implements \ConstVector<T> {

  /** Returns a ImmVector built from the values produced by the specified
   * Iterable.
   * @param mixed $iterable
   */
  <<__Native>>
  public function __construct(mixed $iterable = null)[]: void;

  /** Returns an ImmVector built from the values produced by the specified
   * Iterable.
   * @param mixed $iterable
   * @return object
   */
  public static function fromItems(mixed $iterable)[]: this {
    return new self($iterable);
  }

  /** Returns a ImmVector built from the keys of the specified container.
   * @param mixed $container
   * @return object
   */
  public static function fromKeysOf(mixed $container)[]: this {
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
  public readonly function isEmpty()[]: bool {
    return !$this->count();
  }

  /** Returns the number of values in the ImmVector.
   * @return int
   */
  public readonly function count()[]: int {
    return \count(vec($this));
  }

  /** Returns an Iterable that produces the values from this ImmVector.
   * @return object
   */
  /* HH_FIXME[2049] */
  public function items()[]: \LazyIterableView {
  /* HH_FIXME[2049] */
    return new \LazyIterableView($this);
  }

  /** Returns true if the specified key is present in the ImmVector, returns
   * false otherwise.
   * @param mixed $key
   * @return bool
   */
  public readonly function containsKey(mixed $key)[]: bool {
    if (!($key is int)) {
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
  public function at(mixed $key)[]: T {
    return $this[HH\FIXME\UNSAFE_CAST<mixed, int>($key)];
  }

  /** Returns the value at the specified key. If the key is not present, null is
   * returned.
   * @param mixed $key
   * @return mixed
   */
  public function get(mixed $key)[]: ?T {
    return HH\FIXME\UNSAFE_CAST<mixed, ?T>(idx($this, $key));
  }

  /** Returns an iterator that points to beginning of this ImmVector.
   * @return object
   */
  <<__Native>>
  public function getIterator()[]: Iterator<T>;

  /** Returns a Vector of the values produced by applying the specified callback
   * on each value from this ImmVector.
   * @param mixed $callback
   * @return object
   */
  public function map<To>(
    (function(T)[_]: To) $callback,
  )[ctx $callback]: ImmVector<To> {
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
  public function mapWithKey<To>(
    (function(int, T)[_]: To) $callback,
  )[ctx $callback]: ImmVector<To> {
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
  public function filter((function(T)[_]: bool) $callback)[ctx $callback]: this {
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
  public function filterWithKey(
    (function(int, T)[_]: bool) $callback,
  )[ctx $callback]: this {
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
  public function zip<To>(
    mixed $iterable,
  )[]: ImmVector<Pair<T, To>> {
    $i = 0;
    $result = vec[];
    $count = $this->count();
    foreach (
      HH\FIXME\UNSAFE_CAST<mixed, Traversable<To>>($iterable) as $value
    ) {
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
  public function take(mixed $n)[]: this {
    if (!($n is int)) {
      throw new \InvalidArgumentException("Parameter n must be an integer");
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
  public function takeWhile(
    (function(T)[_]: bool) $callback,
  )[ctx $callback]: this {
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
  public function skip(mixed $n)[]: this {
    if (!($n is int)) {
      throw new \InvalidArgumentException("Parameter n must be an integer");
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
  public function skipWhile((function(T)[_]: bool) $fn)[ctx $fn]: this {
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
  public function slice(mixed $start, mixed $len)[]: this {
    if (!($start is int) || $start < 0) {
      throw new \InvalidArgumentException(
        "Parameter start must be a non-negative integer");
    }
    if (!($len is int) || $len < 0) {
      throw new \InvalidArgumentException(
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
  public function concat<To super T>(
    mixed $iterable,
  )[]: ImmVector<To> {
    $result = vec($this);
    foreach (
      HH\FIXME\UNSAFE_CAST<mixed, Traversable<To>>($iterable) as $value
    ) {
      $result[] = $value;
    }
    return new self($result);
  }

  /** Returns the first value from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  public function firstValue()[]: mixed {
    return idx($this, 0);
  }

  /** Returns the first key from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  public readonly function firstKey()[]: mixed {
    return $this->count() ? 0 : null;
  }

  /** Returns the last value from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  public function lastValue()[]: mixed {
    $count = $this->count();
    return $count ? $this[$count - 1] : null;
  }

  /** Returns the last key from this ImmVector, or null if this ImmVector is
   * empty.
   * @return mixed
   */
  public readonly function lastKey()[]: mixed {
    $count = $this->count();
    return $count ? ($count - 1) : null;
  }

  /** Returns an Iterable that produces the keys from this ImmVector.
   * @return object
   */
  public function keys()[]: ImmVector<int> {
    return new self($this->toKeysArray());
  }

  /** @return string
   */
  public function __toString()[]: string { return "ImmVector"; }

  /** Returns a Vector built from the values of this ImmVector.
   * @return object
   */
  public function toVector()[]: Vector<T> {
    return new Vector($this);
  }

  /** Returns an immutable version of this collection.
   * @return object
   */
  public function toImmVector()[]: this {
    return $this;
  }

  /** Returns a Map built from the keys and values of this ImmVector.
   * @return object
   */
  /* HH_FIXME[2049] */
  public function toMap()[]: Map<int, T> {
    /* HH_FIXME[2049] */
    return new Map($this);
  }

  /** Returns a ImmMap built from the keys and values of this ImmVector.
   * @return object
   */
  /* HH_FIXME[2049] */
  public function toImmMap()[]: ImmMap<int, T> {
    /* HH_FIXME[2049] */
    return new ImmMap($this);
  }

  /** Returns a Set built from the values of this ImmVector.
   * @return object
   */
  /* HH_FIXME[2049] */
  public function toSet()[]: Set<T> where T as arraykey {
    /* HH_FIXME[2049] */
    return new Set($this);
  }

  /** Returns a ImmSet built from the values of this ImmVector.
   * @return object
   */
  /* HH_FIXME[2049] */
  public function toImmSet()[]: ImmSet<T> where T as arraykey {
    /* HH_FIXME[2049] */
    return new ImmSet($this);
  }

  /** Returns an immutable version of this collection.
   * @return object
   */
  public function immutable()[]: this {
    return $this;
  }

  /** Returns a copy of this ImmVector.
   * @return object
   */
  public function values()[]: this {
    return new self($this);
  }

  /** Returns a lazy iterable view of this ImmVector.
   * @return object
   */
  /* HH_FIXME[2049] */
  public function lazy()[]: \LazyKeyedIterableView {
    /* HH_FIXME[2049] */
    return new \LazyKeyedIterableView($this);
  }

  /** Returns a varray built from the values from this ImmVector.
   * @return varray
   */
  public function toVArray()[]: varray<T> {
    return varray($this);
  }

  public function toDArray()[]: darray<int, T> {
    return darray($this);
  }

  /** Returns a varray built from the keys from this ImmVector.
   * @return varray
   */
  public readonly function toKeysArray()[]: varray<int> {
    $count = $this->count();
    return $count
      ? varray(HH\FIXME\UNSAFE_CAST<mixed, varray<int>>(\range(0, $count - 1)))
      : varray[];
  }

  /** Returns a varray built from the values from this ImmVector.
   * @return varray
   */
  public function toValuesArray()[]: varray<T> {
    return $this->toVArray();
  }

  /** Returns index of the specified value if it is present, -1 otherwise.
   * @param mixed $search_value
   * @return int
   */
  public readonly function linearSearch(mixed $search_value)[]: int {
    foreach (vec($this) as $i => $value) {
      if ($value === $search_value) {
        return $i;
      }
    }
    return -1;
  }
}

} // namespace HH
