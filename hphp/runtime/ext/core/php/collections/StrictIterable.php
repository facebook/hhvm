<?hh

trait StrictIterable<+Tv> implements \HH\Iterable<Tv> {
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
    return new LazyIterableView($this);
  }
  public function values() {
    return new Vector($this);
  }
  public function map<Tu>(
    (function(Tv)[_]: Tu) $fn,
  )[ctx $fn]: \HH\Iterable<Tu> {
    $res = vec[];
    foreach ($this as $v) {
      $res[] = $fn($v);
    }
    return new Vector($res);
  }
  public function filter(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\Iterable<Tv> {
    $res = vec[];
    foreach ($this as $v) {
      if ($fn($v)) $res[] = $v;
    }
    return new Vector($res);
  }
  public function zip($iterable) {
    $res = Vector {};
    $it = $iterable->getIterator();
    foreach ($this as $v) {
      if (!$it->valid()) break;
      $res[] = Pair {$v, $it->current()};
      $it->next();
    }
    return $res;
  }
  public function take($n) {
    $res = Vector {};
    if ($n <= 0) return $res;
    foreach ($this as $v) {
      $res[] = $v;
      if (--$n === 0) break;
    }
    return $res;
  }
  public function takeWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\Iterable<Tv> {
    $res = vec[];
    foreach ($this as $v) {
      if (!$fn($v)) break;
      $res[] = $v;
    }
    return new Vector($res);
  }
  public function skip($n) {
    $res = Vector {};
    foreach ($this as $v) {
      if ($n <= 0) {
        $res[] = $v;
      } else {
        --$n;
      }
    }
    return $res;
  }
  public function skipWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\Iterable<Tv> {
    $res = vec[];
    $skip = true;
    foreach ($this as $v) {
      if ($skip) {
        if ($fn($v)) continue;
        $skip = false;
      }
      $res[] = $v;
    }
    return new Vector($res);
  }
  public function slice($start, $len) {
    $res = Vector {};
    if ($len <= 0) return $res;
    foreach ($this as $v) {
      if ($start !== 0) {
        --$start;
        continue;
      }
      $res[] = $v;
      if (--$len === 0) break;
    }
    return $res;
  }
  public function concat($iterable) {
    $res = Vector {};
    foreach ($this as $v) {
      $res[] = $v;
    }
    foreach ($iterable as $v) {
      $res[] = $v;
    }
    return $res;
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
