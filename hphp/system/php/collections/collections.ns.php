<?hh // partial
<<file:__EnableUnstableFeatures('readonly')>>

namespace {

<<__Sealed(\HH\Collection::class, ConstMap::class, ConstSet::class, ConstVector::class)>>
interface ConstCollection extends HH\Rx\Countable {
  public readonly function isEmpty()[];
  public readonly function count()[];
  public function items()[];
}

<<__Sealed(\HH\Collection::class)>>
interface OutputCollection {
  public function add($e)[write_props];
  public function addAll(
    $iterable
  )[write_props];
}

}

namespace HH {

<<__Sealed(\MutableMap::class, \MutableSet::class, \MutableVector::class)>>
interface Collection extends \ConstCollection,
                             \OutputCollection {
  public function clear()[write_props];
}

}

namespace {

<<__Sealed(ConstMapAccess::class, SetAccess::class, ConstSet::class)>>
interface ConstSetAccess {
  public function contains($m)[];
}

<<__Sealed(MapAccess::class, MutableSet::class)>>
interface SetAccess extends ConstSetAccess {
  public function remove($m)[write_props];
}

<<__Sealed(ConstMapAccess::class, IndexAccess::class, ConstVector::class)>>
interface ConstIndexAccess {
  public function at($k)[];
  public function get($k)[];
  public function containsKey($k)[];
}

<<__Sealed(MapAccess::class, MutableVector::class)>>
interface IndexAccess extends ConstIndexAccess {
  public function set($k,$v)[write_props];
  public function setAll(
    $iterable
  )[write_props];
  public function removeKey($k)[write_props];
}

<<__Sealed(ConstMap::class, MapAccess::class)>>
interface ConstMapAccess extends ConstSetAccess,
                                 ConstIndexAccess {
}

<<__Sealed(MutableMap::class)>>
interface MapAccess extends ConstMapAccess,
                            SetAccess,
                            IndexAccess {
}

<<__Sealed(ImmVector::class, MutableVector::class, Pair::class)>>
interface ConstVector extends ConstCollection,
                              ConstIndexAccess,
                              \HH\Rx\KeyedIterable,
                              \HH\KeyedContainer {
}

<<__Sealed(Vector::class)>>
interface MutableVector extends ConstVector,
                                \HH\Collection,
                                IndexAccess {
}

<<__Sealed(ImmMap::class, MutableMap::class)>>
interface ConstMap extends ConstCollection,
                           ConstMapAccess,
                           \HH\Rx\KeyedIterable,
                           \HH\KeyedContainer {
}

<<__Sealed(Map::class)>>
interface MutableMap extends ConstMap,
                             \HH\Collection,
                             MapAccess {
}

<<__Sealed(ImmSet::class, MutableSet::class)>>
interface ConstSet extends ConstCollection,
                           ConstSetAccess,
                           \HH\Rx\KeyedIterable,
                           \HH\KeyedContainer {
}

<<__Sealed(Set::class)>>
interface MutableSet extends ConstSet,
                             \HH\Collection,
                             SetAccess {
}

trait StrictIterable<+Tv> implements \HH\Iterable<Tv> {
  <<__ProvenanceSkipFrame>>
  public function toArray() {
    $arr = varray[];
    foreach ($this as $v) {
      $arr[] = $v;
    }
    return $arr;
  }
  <<__ProvenanceSkipFrame>>
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

trait StrictKeyedIterable<Tk, +Tv> implements \HH\KeyedIterable<Tk, Tv> {
  <<__ProvenanceSkipFrame>>
  public function toArray() {
    $arr = darray[];
    foreach ($this as $k => $v) {
      $arr[$k] = $v;
    }
    return $arr;
  }
  <<__ProvenanceSkipFrame>>
  public function toValuesArray() {
    $arr = varray[];
    foreach ($this as $v) {
      $arr[] = $v;
    }
    return $arr;
  }
  <<__ProvenanceSkipFrame>>
  public function toKeysArray() {
    $arr = varray[];
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

trait LazyIterable<+Tv> implements \HH\Iterable<Tv> {
  public function toArray() {
    $arr = varray[];
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

trait LazyKeyedIterable<Tk, +Tv> implements \HH\KeyedIterable<Tk, Tv> {
  public function toArray() {
    $arr = darray[];
    foreach ($this as $k => $v) {
      $arr[$k] = $v;
    }
    return $arr;
  }
  public function toValuesArray() {
    $arr = varray[];
    foreach ($this as $v) {
      $arr[] = $v;
    }
    return $arr;
  }
  public function toKeysArray() {
    $arr = varray[];
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

class LazyMapIterator implements \HH\Iterator {
  private $it;
  private $fn;

  public function __construct($it, $fn)[] {
    $this->it = $it;
    $this->fn = $fn;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->it->rewind();
  }
  public function valid() {
    return $this->it->valid();
  }
  public function next() {
    $this->it->next();
  }
  public function key() {
    return $this->it->key();
  }
  public function current() {
    return ($this->fn)($this->it->current());
  }
}

class LazyMapIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn)[] {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator()[] {
    return new LazyMapIterator($this->iterable->getIterator(), $this->fn);
  }
}

class LazyMapKeyedIterator implements \HH\KeyedIterator {
  private $it;
  private $fn;

  public function __construct($it, $fn)[] {
    $this->it = $it;
    $this->fn = $fn;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->it->rewind();
  }
  public function valid() {
    return $this->it->valid();
  }
  public function next() {
    $this->it->next();
  }
  public function key() {
    return $this->it->key();
  }
  public function current() {
    return ($this->fn)($this->it->current());
  }
}

class LazyMapKeyedIterable implements \HH\KeyedIterable {
  use LazyKeyedIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn)[] {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator()[] {
    return new LazyMapKeyedIterator($this->iterable->getIterator(), $this->fn);
  }
}

class LazyMapWithKeyIterator implements \HH\KeyedIterator {
  private $it;
  private $fn;

  public function __construct($it, $fn)[] {
    $this->it = $it;
    $this->fn = $fn;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->it->rewind();
  }
  public function valid() {
    return $this->it->valid();
  }
  public function next() {
    $this->it->next();
  }
  public function key() {
    return $this->it->key();
  }
  public function current() {
    return ($this->fn)($this->it->key(), $this->it->current());
  }
}

class LazyMapWithKeyIterable implements \HH\KeyedIterable {
  use LazyKeyedIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn)[] {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator()[] {
    return new LazyMapWithKeyIterator($this->iterable->getIterator(),
                                      $this->fn);
  }
}

class LazyFilterIterator implements \HH\Iterator {
  private $it;
  private $fn;

  public function __construct($it, $fn)[] {
    $this->it = $it;
    $this->fn = $fn;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $it = $this->it;
    $fn = $this->fn;
    $it->rewind();
    while ($it->valid() && !$fn($it->current())) {
      $it->next();
    }
  }
  public function valid() {
    return $this->it->valid();
  }
  public function next() {
    $it = $this->it;
    $fn = $this->fn;
    $it->next();
    while ($it->valid() && !$fn($it->current())) {
      $it->next();
    }
  }
  public function key() {
    return $this->it->key();
  }
  public function current() {
    return $this->it->current();
  }
}

class LazyFilterIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn)[] {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator()[] {
    return new LazyFilterIterator($this->iterable->getIterator(), $this->fn);
  }
}

class LazyFilterKeyedIterator implements \HH\KeyedIterator {
  private $it;
  private $fn;

  public function __construct($it, $fn)[] {
    $this->it = $it;
    $this->fn = $fn;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $it = $this->it;
    $fn = $this->fn;
    $it->rewind();
    while ($it->valid() && !$fn($it->current())) {
      $it->next();
    }
  }
  public function valid() {
    return $this->it->valid();
  }
  public function next() {
    $it = $this->it;
    $fn = $this->fn;
    $it->next();
    while ($it->valid() && !$fn($it->current())) {
      $it->next();
    }
  }
  public function key() {
    return $this->it->key();
  }
  public function current() {
    return $this->it->current();
  }
}

class LazyFilterKeyedIterable implements \HH\KeyedIterable {
  use LazyKeyedIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn)[] {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator()[] {
    return
      new LazyFilterKeyedIterator($this->iterable->getIterator(), $this->fn);
  }
}

class LazyFilterWithKeyIterator implements \HH\KeyedIterator {
  private $it;
  private $fn;

  public function __construct($it, $fn)[] {
    $this->it = $it;
    $this->fn = $fn;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $it = $this->it;
    $fn = $this->fn;
    $it->rewind();
    while ($it->valid() && !$fn($it->key(), $it->current())) {
      $it->next();
    }
  }
  public function valid() {
    return $this->it->valid();
  }
  public function next() {
    $it = $this->it;
    $fn = $this->fn;
    $it->next();
    while ($it->valid() && !$fn($it->key(), $it->current())) {
      $it->next();
    }
  }
  public function key() {
    return $this->it->key();
  }
  public function current() {
    return $this->it->current();
  }
}

class LazyFilterWithKeyIterable implements \HH\KeyedIterable {
  use LazyKeyedIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn)[] {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator()[] {
    return
      new LazyFilterWithKeyIterator($this->iterable->getIterator(), $this->fn);
  }
}

class LazyZipIterator implements \HH\Iterator {
  private $it1;
  private $it2;

  public function __construct($it1, $it2)[] {
    $this->it1 = $it1;
    $this->it2 = $it2;
  }
  public function __clone() {
    $this->it1 = clone $this->it1;
    $this->it2 = clone $this->it2;
  }
  public function rewind() {
    $this->it1->rewind();
    $this->it2->rewind();
  }
  public function valid() {
    return ($this->it1->valid() && $this->it2->valid());
  }
  public function next() {
    $this->it1->next();
    $this->it2->next();
  }
  public function key() {
    return $this->it->key();
  }
  public function current() {
    return Pair {$this->it1->current(), $this->it2->current()};
  }
}

class LazyZipIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable1;
  private $iterable2;

  public function __construct($iterable1, $iterable2)[] {
    $this->iterable1 = $iterable1;
    $this->iterable2 = $iterable2;
  }
  public function getIterator()[] {
    return new LazyZipIterator($this->iterable1->getIterator(),
                               $this->iterable2->getIterator());
  }
}

class LazyZipKeyedIterator implements \HH\KeyedIterator {
  private $it1;
  private $it2;

  public function __construct($it1, $it2)[] {
    $this->it1 = $it1;
    $this->it2 = $it2;
  }
  public function __clone() {
    $this->it1 = clone $this->it1;
    $this->it2 = clone $this->it2;
  }
  public function rewind() {
    $this->it1->rewind();
    $this->it2->rewind();
  }
  public function valid() {
    return ($this->it1->valid() && $this->it2->valid());
  }
  public function next() {
    $this->it1->next();
    $this->it2->next();
  }
  public function key() {
    return $this->it1->key();
  }
  public function current() {
    return Pair {$this->it1->current(), $this->it2->current()};
  }
}

class LazyZipKeyedIterable implements \HH\KeyedIterable {
  use LazyKeyedIterable;

  private $iterable1;
  private $iterable2;

  public function __construct($iterable1, $iterable2)[] {
    $this->iterable1 = $iterable1;
    $this->iterable2 = $iterable2;
  }
  public function getIterator()[] {
    return new LazyZipKeyedIterator($this->iterable1->getIterator(),
                                    $this->iterable2->getIterator());
  }
}

class LazyTakeIterator implements \HH\Iterator {
  private $it;
  private $n;
  private $numLeft;

  public function __construct($it, $n)[] {
    $this->it = $it;
    $this->n = $n;
    $this->numLeft = $n;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->it->rewind();
    $this->numLeft = $this->n;
  }
  public function valid() {
    return ($this->numLeft > 0 && $this->it->valid());
  }
  public function next() {
    $this->it->next();
    --$this->numLeft;
  }
  public function key() {
    return $this->it->key();
  }
  public function current() {
    return $this->it->current();
  }
}

class LazyTakeIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $n;

  public function __construct($iterable, $n)[] {
    $this->iterable = $iterable;
    $this->n = $n;
  }
  public function getIterator()[] {
    return new LazyTakeIterator($this->iterable->getIterator(),
                                $this->n);
  }
}

class LazyTakeKeyedIterator implements \HH\KeyedIterator {
  private $it;
  private $n;
  private $numLeft;

  public function __construct($it, $n)[] {
    $this->it = $it;
    $this->n = $n;
    $this->numLeft = $n;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->it->rewind();
    $this->numLeft = $this->n;
  }
  public function valid() {
    return ($this->numLeft > 0 && $this->it->valid());
  }
  public function next() {
    $this->it->next();
    --$this->numLeft;
  }
  public function key() {
    return $this->it->key();
  }
  public function current() {
    return $this->it->current();
  }
}

class LazyTakeKeyedIterable implements \HH\KeyedIterable {
  use LazyKeyedIterable;

  private $iterable;
  private $n;

  public function __construct($iterable, $n)[] {
    $this->iterable = $iterable;
    $this->n = $n;
  }
  public function getIterator()[] {
    return new LazyTakeKeyedIterator($this->iterable->getIterator(),
                                     $this->n);
  }
}

class LazyTakeWhileIterator implements \HH\Iterator {
  private $it;
  private $fn;

  public function __construct($it, $fn)[] {
    $this->it = $it;
    $this->fn = $fn;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->it->rewind();
  }
  public function valid() {
    $it = $this->it;
    return ($it->valid() && ($this->fn)($it->current()));
  }
  public function next() {
    $this->it->next();
  }
  public function key() {
    return $this->it->key();
  }
  public function current() {
    return $this->it->current();
  }
}

class LazyTakeWhileIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn)[] {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator()[] {
    return new LazyTakeWhileIterator($this->iterable->getIterator(),
                                     $this->fn);
  }
}

class LazyTakeWhileKeyedIterator<Tk, +Tv>
    implements \HH\KeyedIterator<Tk, Tv> {
  private $it;
  private $fn;

  public function __construct($it, $fn)[] {
    $this->it = $it;
    $this->fn = $fn;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->it->rewind();
  }
  public function valid() {
    $it = $this->it;
    return ($it->valid() && ($this->fn)($it->current()));
  }
  public function next() {
    $this->it->next();
  }
  public function key() {
    return $this->it->key();
  }
  public function current() {
    return $this->it->current();
  }
}

class LazyTakeWhileKeyedIterable<+Tv> implements \HH\KeyedIterable<Tk, Tv> {
  use LazyKeyedIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn)[] {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator()[] {
    return new LazyTakeWhileKeyedIterator($this->iterable->getIterator(),
                                          $this->fn);
  }
}

class LazySkipIterator implements \HH\Iterator {
  private $it;
  private $n;

  public function __construct($it, $n)[] {
    $this->it = $it;
    $this->n = $n;
  }
  public function __clone() {
    $this->impureInit();
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->impureInit();
    $it = $this->it;
    $n = $this->n;
    $it->rewind();
    while ($n > 0 && $it->valid()) {
      $it->next();
      --$n;
    }
  }
  public function valid() {
    $this->impureInit();
    return $this->it->valid();
  }
  public function next() {
    $this->impureInit();
    $this->it->next();
  }
  public function key() {
    $this->impureInit();
    return $this->it->key();
  }
  public function current() {
    $this->impureInit();
    return $this->it->current();
  }
  <<__Memoize>>
  private function impureInit(): void {
    $it = $this->it;
    $n = $this->n;
    while ($n > 0 && $it->valid()) {
      $it->next();
      --$n;
    }
  }
}

class LazySkipIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $n;

  public function __construct($iterable, $n)[] {
    $this->iterable = $iterable;
    $this->n = $n;
  }
  public function getIterator()[] {
    return new LazySkipIterator($this->iterable->getIterator(),
                                $this->n);
  }
}

class LazySkipKeyedIterator implements \HH\KeyedIterator {
  private $it;
  private $n;

  public function __construct($it, $n)[] {
    $this->it = $it;
    $this->n = $n;
  }
  public function __clone() {
    $this->impureInit();
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->impureInit();
    $it = $this->it;
    $n = $this->n;
    $it->rewind();
    while ($n > 0 && $it->valid()) {
      $it->next();
      --$n;
    }
  }
  public function valid() {
    $this->impureInit();
    return $this->it->valid();
  }
  public function next() {
    $this->impureInit();
    $this->it->next();
  }
  public function key() {
    $this->impureInit();
    return $this->it->key();
  }
  public function current() {
    $this->impureInit();
    return $this->it->current();
  }
  <<__Memoize>>
  private function impureInit(): void {
    $it = $this->it;
    $n = $this->n;
    while ($n > 0 && $it->valid()) {
      $it->next();
      --$n;
    }
  }
}

class LazySkipKeyedIterable implements \HH\KeyedIterable {
  use LazyKeyedIterable;

  private $iterable;
  private $n;

  public function __construct($iterable, $n)[] {
    $this->iterable = $iterable;
    $this->n = $n;
  }
  public function getIterator()[] {
    return new LazySkipKeyedIterator($this->iterable->getIterator(),
                                     $this->n);
  }
}

class LazySkipWhileIterator implements \HH\Iterator {
  private $it;
  private $fn;

  public function __construct($it, $fn)[] {
    $this->it = $it;
    $this->fn = $fn;
  }
  public function __clone() {
    $this->impureInit();
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->impureInit();
    $it = $this->it;
    $fn = $this->fn;
    $it->rewind();
    while ($it->valid() && $fn($it->current())) {
      $it->next();
    }
  }
  public function valid() {
    $this->impureInit();
    return $this->it->valid();
  }
  public function next() {
    $this->impureInit();
    $this->it->next();
  }
  public function key() {
    $this->impureInit();
    return $this->it->key();
  }
  public function current() {
    $this->impureInit();
    return $this->it->current();
  }
  <<__Memoize>>
  private function impureInit(): void {
    $it = $this->it;
    $fn = $this->fn;
    while ($it->valid() && $fn($it->current())) {
      $it->next();
    }
  }
}

class LazySkipWhileIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn)[] {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator()[] {
    return new LazySkipWhileIterator($this->iterable->getIterator(),
                                     $this->fn);
  }
}

class LazySkipWhileKeyedIterator<Tk, +Tv>
    implements \HH\KeyedIterator<Tk, Tv> {
  private $it;
  private $fn;

  public function __construct($it, $fn)[] {
    $this->it = $it;
    $this->fn = $fn;
  }
  public function __clone() {
    $this->impureInit();
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->impureInit();
    $it = $this->it;
    $fn = $this->fn;
    $it->rewind();
    while ($it->valid() && $fn($it->current())) {
      $it->next();
    }
  }
  public function valid() {
    $this->impureInit();
    return $this->it->valid();
  }
  public function next() {
    $this->impureInit();
    $this->it->next();
  }
  public function key() {
    $this->impureInit();
    return $this->it->key();
  }
  public function current() {
    $this->impureInit();
    return $this->it->current();
  }
  <<__Memoize>>
  private function impureInit(): void {
    $it = $this->it;
    $fn = $this->fn;
    while ($it->valid() && $fn($it->current())) {
      $it->next();
    }
  }
}

class LazySkipWhileKeyedIterable<Tk, +Tv>
    implements \HH\KeyedIterable<Tk, Tv> {
  use LazyKeyedIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn)[] {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator()[] {
    return new LazySkipWhileKeyedIterator($this->iterable->getIterator(),
                                          $this->fn);
  }
}

class LazySliceIterator implements \HH\Iterator {
  private $it;
  private $start;
  private $len;
  private $currentLen;

  public function __construct($it, $start, $len)[] {
    $this->it = $it;
    $this->start = $start;
    $this->len = $len;
    $this->currentLen = $len;
  }
  public function __clone() {
    $this->impureInit();
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->impureInit();
    $it = $this->it;
    $start = $this->start;
    $len = $this->len;
    $it->rewind();
    $this->currentLen = $len;
    while ($start !== 0 && $it->valid()) {
      $it->next();
      --$start;
    }
  }
  public function valid() {
    $this->impureInit();
    return $this->it->valid() && $this->currentLen !== 0;
  }
  public function next() {
    $this->impureInit();
    $this->it->next();
    if ($this->currentLen !== 0) {
      --$this->currentLen;
    }
  }
  public function key() {
    $this->impureInit();
    return $this->it->key();
  }
  public function current() {
    $this->impureInit();
    return $this->it->current();
  }
  <<__Memoize>>
  private function impureInit(): void {
    $it = $this->it;
    $start = $this->start;
    while ($start !== 0 && $it->valid()) {
      $it->next();
      --$start;
    }
  }
}

class LazySliceIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $start;
  private $len;

  public function __construct($iterable, $start, $len)[] {
    $this->iterable = $iterable;
    $this->start = $start;
    $this->len = $len;
  }
  public function getIterator()[] {
    return new LazySliceIterator($this->iterable->getIterator(),
                                 $this->start,
                                 $this->len);
  }
}

class LazySliceKeyedIterator implements \HH\KeyedIterator {
  private $it;
  private $start;
  private $len;
  private $currentLen;

  public function __construct($it, $start, $len)[] {
    $this->it = $it;
    $this->start = $start;
    $this->len = $len;
    $this->currentLen = $len;
  }
  public function __clone() {
    $this->impureInit();
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->impureInit();
    $it = $this->it;
    $start = $this->start;
    $len = $this->len;
    $it->rewind();
    $this->currentLen = $len;
    while ($start !== 0 && $it->valid()) {
      $it->next();
      --$start;
    }
  }
  public function valid() {
    $this->impureInit();
    return $this->it->valid() && $this->currentLen !== 0;
  }
  public function next() {
    $this->impureInit();
    $this->it->next();
    if ($this->currentLen !== 0) {
      --$this->currentLen;
    }
  }
  public function key() {
    $this->impureInit();
    return $this->it->key();
  }
  public function current() {
    $this->impureInit();
    return $this->it->current();
  }
  <<__Memoize>>
  private function impureInit(): void {
    $it = $this->it;
    $start = $this->start;
    while ($start !== 0 && $it->valid()) {
      $it->next();
      --$start;
    }
  }
}

class LazySliceKeyedIterable implements \HH\KeyedIterable {
  use LazyKeyedIterable;

  private $iterable;
  private $start;
  private $len;

  public function __construct($iterable, $start, $len)[] {
    $this->iterable = $iterable;
    $this->start = $start;
    $this->len = $len;
  }
  public function getIterator()[] {
    return new LazySliceKeyedIterator($this->iterable->getIterator(),
                                      $this->start,
                                      $this->len);
  }
}

class LazyKeysIterator implements \HH\Iterator {
  private $it;

  public function __construct($it)[] {
    $this->it = $it;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->it->rewind();
  }
  public function valid() {
    return $this->it->valid();
  }
  public function next() {
    $this->it->next();
  }
  public function key() {
    return null;
  }
  public function current() {
    return $this->it->key();
  }
}

class LazyKeysIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;

  public function __construct($iterable)[] {
    $this->iterable = $iterable;
  }
  public function getIterator()[] {
    return new LazyKeysIterator($this->iterable->getIterator());
  }
}

class LazyValuesIterator implements \HH\Iterator {
  private $it;

  public function __construct($it)[] {
    $this->it = $it;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->it->rewind();
  }
  public function valid() {
    return $this->it->valid();
  }
  public function next() {
    $this->it->next();
  }
  public function key() {
    return null;
  }
  public function current() {
    return $this->it->current();
  }
}

class LazyValuesIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;

  public function __construct($iterable)[] {
    $this->iterable = $iterable;
  }
  public function getIterator()[] {
    return new LazyValuesIterator($this->iterable->getIterator());
  }
}

class LazyKVZipIterator implements \HH\Iterator {
  private $it;

  public function __construct($it)[] {
    $this->it = $it;
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $this->it->rewind();
  }
  public function valid() {
    return $this->it->valid();
  }
  public function next() {
    $this->it->next();
  }
  public function key() {
    return null;
  }
  public function current() {
    return Pair {$this->it->key(), $this->it->current()};
  }
}

class LazyKVZipIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;

  public function __construct($iterable)[] {
    $this->iterable = $iterable;
  }
  public function getIterator()[] {
    return new LazyKVZipIterator($this->iterable->getIterator());
  }
}

class LazyConcatIterator implements \HH\Iterator {
  private $it1;
  private $it2;
  private $currentIt;
  private $state;

  public function __construct($it1, $it2)[] {
    $this->it1 = $it1;
    $this->it2 = $it2;
  }
  public function __clone() {
    $this->impureInit();
    $this->it1 = clone $this->it1;
    $this->it2 = clone $this->it2;
    $this->currentIt = ($this->state === 1) ? $this->it1 : $this->it2;
  }
  public function rewind() {
    $this->impureInit();
    $this->it1->rewind();
    $this->it2->rewind();
    $this->currentIt = $this->it1;
    $this->state = 1;
    if (!$this->currentIt->valid()) {
      $this->currentIt = $this->it2;
      $this->state = 2;
    }
  }
  public function valid() {
    $this->impureInit();
    return $this->currentIt->valid();
  }
  public function next() {
    $this->impureInit();
    $this->currentIt->next();
    if ($this->state === 1 && !$this->currentIt->valid()) {
      $this->currentIt = $this->it2;
      $this->state = 2;
    }
  }
  public function key() {
    $this->impureInit();
    return $this->currentIt->key();
  }
  public function current() {
    $this->impureInit();
    return $this->currentIt->current();
  }
  <<__Memoize>>
  private function impureInit(): void {
    $this->currentIt = $this->it1;
    $this->state = 1;
    if (!$this->currentIt->valid()) {
      $this->currentIt = $this->it2;
      $this->state = 2;
    }
  }
}

class LazyConcatIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable1;
  private $iterable2;

  public function __construct($iterable1, $iterable2)[] {
    $this->iterable1 = $iterable1;
    $this->iterable2 = $iterable2;
  }
  public function getIterator()[] {
    return new LazyConcatIterator($this->iterable1->getIterator(),
                                  $this->iterable2->getIterator());
  }
}

class LazyIterableView<+Tv> implements \HH\Iterable<Tv> {
  public $iterable;

  public function __construct($iterable)[] { $this->iterable = $iterable; }
  public function getIterator()[] { return $this->iterable->getIterator(); }
  <<__ProvenanceSkipFrame>>
  public function toArray() {
    $arr = varray[];
    foreach ($this->iterable as $v) {
      $arr[] = $v;
    }
    return $arr;
  }
  <<__ProvenanceSkipFrame>>
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

class LazyKeyedIterableView<Tk, +Tv> implements \HH\KeyedIterable<Tk, Tv> {
  public $iterable;

  public function __construct($iterable)[] { $this->iterable = $iterable; }
  public function getIterator()[] { return $this->iterable->getIterator(); }
  public function toArray() {
    $arr = darray[];
    foreach ($this->iterable as $k => $v) {
      $arr[$k] = $v;
    }
    return $arr;
  }
  public function toValuesArray() {
    $arr = varray[];
    foreach ($this->iterable as $v) {
      $arr[] = $v;
    }
    return $arr;
  }
  public function toKeysArray() {
    $arr = varray[];
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

}
