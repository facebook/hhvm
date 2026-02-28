<?hh

trait LazyIterable<+Tv> implements \HH\Iterable<Tv> {
  public function toArray() {
    $arr = vec[];
    foreach ($this as $v) {
      $arr[] = $v;
    }
    return $arr;
  }
  public function toValuesArray() {
    return $this->toArray();
  }
  public function toVector() {
    return new Vector($this);
  }
  public function toImmVector() {
    return new ImmVector($this);
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
  public function map<Tu>(
    (function(Tv)[_]: Tu) $fn,
  )[ctx $fn]: \HH\Iterable<Tu> {
    return new LazyMapIterable($this, $fn);
  }
  public function filter(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\Iterable<Tv> {
    return new LazyFilterIterable($this, $fn);
  }
  public function zip($iterable)[] {
    if (HH\is_any_array($iterable)) {
      $iterable = new ImmMap($iterable);
    }
    return new LazyZipIterable($this, $iterable);
  }
  public function take($n) {
    return new LazyTakeIterable($this, $n);
  }
  public function takeWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\Iterable<Tv> {
    return new LazyTakeWhileIterable($this, $fn);
  }
  public function skip($n) {
    return new LazySkipIterable($this, $n);
  }
  public function skipWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\Iterable<Tv> {
    return new LazySkipWhileIterable($this, $fn);
  }
  public function slice($start, $len) {
    return new LazySliceIterable($this, $start, $len);
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
  public function lastValue() {
    $v = null;
    foreach ($this as $v) {}
    return $v;
  }
}
