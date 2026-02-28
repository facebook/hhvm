<?hh

class LazyIterableView<+Tv> implements \HH\Iterable<Tv> {
  public $iterable;

  public function __construct($iterable)[] { $this->iterable = $iterable; }
  public function getIterator()[] { return $this->iterable->getIterator(); }
  public function toArray() {
    $arr = vec[];
    foreach ($this->iterable as $v) {
      $arr[] = $v;
    }
    return $arr;
  }
  public function toValuesArray() {
    return $this->toArray();
  }
  public function toVector() {
    return $this->iterable->toVector();
  }
  public function toImmVector() {
    return $this->iterable->toImmVector();
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
  public function map<Tu>(
    (function(Tv)[_]: Tu) $fn,
  )[ctx $fn]: \HH\Iterable<Tu> {
    return new LazyMapIterable($this->iterable, $fn);
  }
  public function filter(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\Iterable<Tv> {
    return new LazyFilterIterable($this->iterable, $fn);
  }
  public function zip($iterable) {
    if (HH\is_any_array($iterable)) {
      $iterable = new ImmMap($iterable);
    }
    return new LazyZipIterable($this->iterable, $iterable);
  }
  public function take($n) {
    return new LazyTakeIterable($this->iterable, $n);
  }
  public function takeWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\Iterable<Tv> {
    return new LazyTakeWhileIterable($this->iterable, $fn);
  }
  public function skip($n) {
    return new LazySkipIterable($this->iterable, $n);
  }
  public function skipWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\Iterable<Tv> {
    return new LazySkipWhileIterable($this->iterable, $fn);
  }
  public function slice($start, $len) {
    return new LazySliceIterable($this->iterable, $start, $len);
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
  public function lastValue() {
    $v = null;
    foreach ($this->iterable as $v) {}
    return $v;
  }
}
