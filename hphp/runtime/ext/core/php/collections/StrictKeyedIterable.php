<?hh

trait StrictKeyedIterable<Tk, +Tv> implements \HH\KeyedIterable<Tk, Tv> {
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
    return new LazyKeyedIterableView($this);
  }
  public function values() {
    return new Vector($this);
  }
  public function keys() {
    $res = Vector {};
    foreach ($this as $k => $_) {
      $res[] = $k;
    }
    return $res;
  }
  public function map<Tu>(
    (function(Tv)[_]: Tu) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk,Tu> {
    $res = dict[];
    foreach ($this as $k => $v) {
      $res[$k] = $fn($v);
    }
    return new Map($res);
  }
  public function mapWithKey<Tu>(
    (function(Tk, Tv)[_]: Tu) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk,Tu> {
    $res = dict[];
    foreach ($this as $k => $v) {
      $res[$k] = $fn($k, $v);
    }
    return new Map($res);
  }
  public function filter(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk,Tv> {
    $res = dict[];
    foreach ($this as $k => $v) {
      if ($fn($v)) $res[$k] = $v;
    }
    return new Map($res);
  }
  public function filterWithKey(
    (function(Tk, Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk,Tv> {
    $res = dict[];
    foreach ($this as $k => $v) {
      if ($fn($k, $v)) $res[$k] = $v;
    }
    return new Map($res);
  }
  public function zip($iterable) {
    $res = Map {};
    $it = $iterable->getIterator();
    foreach ($this as $k => $v) {
      if (!$it->valid()) break;
      $res[$k] = Pair {$v, $it->current()};
      $it->next();
    }
    return $res;
  }
  public function take($n) {
    $res = Map {};
    if ($n <= 0) return $res;
    foreach ($this as $k => $v) {
      $res[$k] = $v;
      if (--$n === 0) break;
    }
    return $res;
  }
  public function takeWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk, Tv> {
    $res = dict[];
    foreach ($this as $k => $v) {
      if (!$fn($v)) break;
      $res[$k] = $v;
    }
    return new Map($res);
  }
  public function skip($n) {
    $res = Map {};
    foreach ($this as $k => $v) {
      if ($n <= 0) {
        $res[$k] = $v;
      } else {
        --$n;
      }
    }
    return $res;
  }
  public function skipWhile(
    (function(Tv)[_]: bool) $fn,
  )[ctx $fn]: \HH\KeyedIterable<Tk, Tv> {
    $res = dict[];
    $skip = true;
    foreach ($this as $k => $v) {
      if ($skip) {
        if ($fn($v)) continue;
        $skip = false;
      }
      $res[$k] = $v;
    }
    return new Map($res);
  }
  public function slice($start, $len) {
    $res = Map {};
    if ($len <= 0) return $res;
    foreach ($this as $k => $v) {
      if ($start !== 0) {
        --$start;
        continue;
      }
      $res[$k] = $v;
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
