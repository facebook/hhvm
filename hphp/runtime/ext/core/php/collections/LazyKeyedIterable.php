<?hh

trait LazyKeyedIterable<Tk, +Tv> implements \HH\KeyedIterable<Tk, Tv> {
  public function toArray() {
    $arr = dict[];
    foreach ($this as $k => $v) {
      $arr[$k] = $v;
    }
    return $arr;
  }
  public function toValuesArray() {
    $arr = vec[];
    foreach ($this as $v) {
      $arr[] = $v;
    }
    return $arr;
  }
  public function toKeysArray() {
    $arr = vec[];
    foreach ($this as $k => $_) {
      $arr[] = $k;
    }
    return $arr;
  }
  public function toVector() {
    return new Vector($this);
  }
  public function toImmVector() {
    return new ImmVector($this);
  }
  public function toMap() {
    return new Map($this);
  }
  public function toImmMap() {
    return new ImmMap($this);
  }
  public function toSet() {
    return new Set($this);
  }
  public function toImmSet() {
    return new ImmSet($this);
  }
  public function lazy() {
    return $this;
  }
  public function values() {
    return new LazyValuesIterable($this);
  }
  public function keys() {
    return new LazyKeysIterable($this);
  }
  public function map<Tu>(
    (function(Tv)[_]: Tu) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk,Tu> {
    return new LazyMapKeyedIterable($this, $fn);
  }
  public function mapWithKey<Tu>(
    (function(Tk, Tv)[_]: Tu) $fn
  )[ctx $fn]: \HH\KeyedIterable<Tk,Tu> {
    return new LazyMapWithKeyIterable($this, $fn);
  }
  public function filter(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk,Tv> {
    return new LazyFilterKeyedIterable($this, $fn);
  }
  public function filterWithKey(
    (function(Tk, Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk,Tv> {
    return new LazyFilterWithKeyIterable($this, $fn);
  }
  public function zip($iterable) {
    if (HH\is_any_array($iterable)) {
      $iterable = new ImmMap($iterable);
    }
    return new LazyZipKeyedIterable($this, $iterable);
  }
  public function take($n) {
    return new LazyTakeKeyedIterable($this, $n);
  }
  public function takeWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk, Tv> {
    return new LazyTakeWhileKeyedIterable($this, $fn);
  }
  public function skip($n) {
    return new LazySkipKeyedIterable($this, $n);
  }
  public function skipWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk, Tv> {
    return new LazySkipWhileKeyedIterable($this, $fn);
  }
  public function slice($start, $len) {
    return new LazySliceKeyedIterable($this, $start, $len);
  }
  public function concat($iterable) {
    if (HH\is_any_array($iterable)) {
      $iterable = new ImmMap($iterable);
    }
    return new LazyConcatIterable($this, $iterable);
  }
  public function firstValue() {
    foreach ($this as $v) {
      return $v;
    }
    return null;
  }
  public function firstKey() {
    foreach ($this as $k => $_) {
      return $k;
    }
    return null;
  }
  public function lastValue() {
    $v = null;
    foreach ($this as $v) {}
    return $v;
  }
  public function lastKey() {
    $k = null;
    foreach ($this as $k => $_) {}
    return $k;
  }
}
