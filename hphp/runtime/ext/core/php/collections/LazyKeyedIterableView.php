<?hh

class LazyKeyedIterableView<Tk, +Tv> implements \HH\KeyedIterable<Tk, Tv> {
  public $iterable;

  public function __construct($iterable)[] { $this->iterable = $iterable; }
  public function getIterator()[] { return $this->iterable->getIterator(); }
  public function toArray() {
    $arr = dict[];
    foreach ($this->iterable as $k => $v) {
      $arr[$k] = $v;
    }
    return $arr;
  }
  public function toValuesArray() {
    $arr = vec[];
    foreach ($this->iterable as $v) {
      $arr[] = $v;
    }
    return $arr;
  }
  public function toKeysArray() {
    $arr = vec[];
    foreach ($this->iterable as $k => $_) {
      $arr[] = $k;
    }
    return $arr;
  }
  public function toVector() {
    return $this->iterable->toVector();
  }
  public function toImmVector() {
    return $this->iterable->toImmVector();
  }
  public function toMap() {
    return $this->iterable->toMap();
  }
  public function toImmMap() {
    return $this->iterable->toImmMap();
  }
  public function toSet() {
    return $this->iterable->toSet();
  }
  public function toImmSet() {
    return $this->iterable->toImmSet();
  }
  public function lazy() {
    return $this;
  }
  public function values() {
    return new LazyValuesIterable($this->iterable);
  }
  public function keys() {
    return new LazyKeysIterable($this->iterable);
  }
  public function map<Tu>(
    (function(Tv)[_]: Tu) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk, Tu> {
    return new LazyMapKeyedIterable($this->iterable, $fn);
  }
  public function mapWithKey<Tu>(
    (function(Tk, Tv)[_]: Tu) $fn
  )[ctx $fn]: \HH\KeyedIterable<Tk, Tu> {
    return new LazyMapWithKeyIterable($this->iterable, $fn);
  }
  public function filter(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk, Tv> {
    return new LazyFilterKeyedIterable($this->iterable, $fn);
  }
  public function filterWithKey(
    (function(Tk, Tv)[_]: bool) $fn
  )[ctx $fn]: \HH\KeyedIterable<Tk, Tv> {
    return new LazyFilterWithKeyIterable($this->iterable, $fn);
  }
  public function zip($iterable) {
    if (HH\is_any_array($iterable)) {
      $iterable = new ImmMap($iterable);
    }
    return new LazyZipKeyedIterable($this->iterable, $iterable);
  }
  public function take($n) {
    return new LazyTakeKeyedIterable($this->iterable, $n);
  }
  public function takeWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk, Tv> {
    return new LazyTakeWhileKeyedIterable($this->iterable, $fn);
  }
  public function skip($n) {
    return new LazySkipKeyedIterable($this->iterable, $n);
  }
  public function skipWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk, Tv> {
    return new LazySkipWhileKeyedIterable($this->iterable, $fn);
  }
  public function slice($start, $len) {
    return new LazySliceKeyedIterable($this->iterable, $start, $len);
  }
  public function concat($iterable) {
    if (HH\is_any_array($iterable)) {
      $iterable = new ImmMap($iterable);
    }
    return new LazyConcatIterable($this->iterable, $iterable);
  }
  public function firstValue() {
    foreach ($this->iterable as $v) {
      return $v;
    }
    return null;
  }
  public function firstKey() {
    foreach ($this->iterable as $k => $_) {
      return $k;
    }
    return null;
  }
  public function lastValue() {
    $v = null;
    foreach ($this->iterable as $v) {}
    return $v;
  }
  public function lastKey() {
    $k = null;
    foreach ($this->iterable as $k => $_) {}
    return $k;
  }
}
