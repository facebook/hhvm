<?php

namespace {

interface ConstCollection extends Countable {
  public function isEmpty();
  public function count();
  public function items();
}

interface OutputCollection {
  public function add($e);
  public function addAll($iterable);
}

}

namespace HH {

interface Collection extends \ConstCollection,
                             \OutputCollection {
  public function clear();
}

}

namespace {

interface ConstSetAccess {
  public function contains($m);
}

interface SetAccess extends ConstSetAccess {
  public function remove($m);
}

interface ConstIndexAccess {
  public function at($k);
  public function get($k);
  public function containsKey($k);
}

interface IndexAccess extends ConstIndexAccess {
  public function set($k,$v);
  public function setAll($iterable);
  public function removeKey($k);
}

interface ConstMapAccess extends ConstSetAccess,
                                 ConstIndexAccess {
}

interface MapAccess extends ConstMapAccess,
                            SetAccess,
                            IndexAccess {
}

interface Indexish extends \HH\KeyedTraversable {
}

interface ConstVector extends ConstCollection,
                              ConstIndexAccess,
                              \HH\KeyedIterable,
                              Indexish {
}

interface MutableVector extends ConstVector,
                                \HH\Collection,
                                IndexAccess {
}

interface ConstMap extends ConstCollection,
                           ConstMapAccess,
                           \HH\KeyedIterable,
                           Indexish {
}

interface MutableMap extends ConstMap,
                             \HH\Collection,
                             MapAccess {
}

interface ConstSet extends ConstCollection,
                           ConstSetAccess,
                           \HH\Iterable {
}

interface MutableSet extends ConstSet,
                             \HH\Collection,
                             SetAccess {
}

trait StrictIterable {
  public function toArray() {
    $arr = array();
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
  public function values() {
    return new Vector($this);
  }
  public function lazy() {
    return new LazyIterableView($this);
  }
  public function map($callback) {
    $res = Vector {};
    foreach ($this as $v) {
      $res[] = $callback($v);
    }
    return $res;
  }
  public function filter($callback) {
    $res = Vector {};
    foreach ($this as $v) {
      if ($callback($v)) $res[] = $v;
    }
    return $res;
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
  public function takeWhile($fn) {
    $res = Vector {};
    foreach ($this as $v) {
      if (!$fn($v)) break;
      $res[] = $v;
    }
    return $res;
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
  public function skipWhile($fn) {
    $res = Vector {};
    $skip = true;
    foreach ($this as $v) {
      if ($skip) {
        if ($fn($v)) continue;
        $skip = false;
      }
      $res[] = $v;
    }
    return $res;
  }
}

trait StrictKeyedIterable {
  public function toArray() {
    $arr = array();
    foreach ($this as $k => $v) {
      $arr[$k] = $v;
    }
    return $arr;
  }
  public function toValuesArray() {
    $arr = array();
    foreach ($this as $v) {
      $arr[] = $v;
    }
    return $arr;
  }
  public function toKeysArray() {
    $arr = array();
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
  public function map($callback) {
    $res = Map {};
    foreach ($this as $k => $v) {
      $res[$k] = $callback($v);
    }
    return $res;
  }
  public function mapWithKey($callback) {
    $res = Map {};
    foreach ($this as $k => $v) {
      $res[$k] = $callback($k, $v);
    }
    return $res;
  }
  public function filter($callback) {
    $res = Map {};
    foreach ($this as $k => $v) {
      if ($callback($v)) $res[$k] = $v;
    }
    return $res;
  }
  public function filterWithKey($callback) {
    $res = Map {};
    foreach ($this as $k => $v) {
      if ($callback($k, $v)) $res[$k] = $v;
    }
    return $res;
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
  public function takeWhile($fn) {
    $res = Map {};
    foreach ($this as $k => $v) {
      if (!$fn($v)) break;
      $res[$k] = $v;
    }
    return $res;
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
  public function skipWhile($fn) {
    $res = Map {};
    $skip = true;
    foreach ($this as $k => $v) {
      if ($skip) {
        if ($fn($v)) continue;
        $skip = false;
      }
      $res[$k] = $v;
    }
    return $res;
  }
  public function keys() {
    $res = Vector {};
    foreach ($this as $k => $_) {
      $res[] = $k;
    }
    return $res;
  }
  public function kvzip() {
    $res = Vector {};
    foreach ($this as $k => $v) {
      $res[] = Pair {$k, $v};
    }
    return $res;
  }
}

trait LazyIterable {
  public function toArray() {
    $arr = array();
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
  public function map($callback) {
    return new LazyMapIterable($this, $callback);
  }
  public function filter($callback) {
    return new LazyFilterIterable($this, $callback);
  }
  public function zip($iterable) {
    return new LazyZipIterable($this, $iterable);
  }
  public function take($n) {
    return new LazyTakeIterable($this, $n);
  }
  public function takeWhile($fn) {
    return new LazyTakeWhileIterable($this, $fn);
  }
  public function skip($n) {
    return new LazySkipIterable($this, $n);
  }
  public function skipWhile($fn) {
    return new LazySkipWhileIterable($this, $fn);
  }
}

trait LazyKeyedIterable {
  public function toArray() {
    $arr = array();
    foreach ($this as $k => $v) {
      $arr[$k] = $v;
    }
    return $arr;
  }
  public function toValuesArray() {
    $arr = array();
    foreach ($this as $v) {
      $arr[] = $v;
    }
    return $arr;
  }
  public function toKeysArray() {
    $arr = array();
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
  public function map($callback) {
    return new LazyMapKeyedIterable($this, $callback);
  }
  public function mapWithKey($callback) {
    return new LazyMapWithKeyIterable($this, $callback);
  }
  public function filter($callback) {
    return new LazyFilterKeyedIterable($this, $callback);
  }
  public function filterWithKey($callback) {
    return new LazyFilterWithKeyIterable($this, $callback);
  }
  public function zip($iterable) {
    return new LazyZipKeyedIterable($this, $iterable);
  }
  public function take($n) {
    return new LazyTakeKeyedIterable($this, $n);
  }
  public function takeWhile($fn) {
    return new LazyTakeWhileKeyedIterable($this, $fn);
  }
  public function skip($n) {
    return new LazySkipKeyedIterable($this, $n);
  }
  public function skipWhile($fn) {
    return new LazySkipWhileKeyedIterable($this, $fn);
  }
  public function keys() {
    return new LazyKeysIterable($this);
  }
  public function kvzip() {
    return new LazyKVZipIterable($this);
  }
}

class LazyMapIterator implements \HH\Iterator {
  private $it;
  private $fn;

  public function __construct($it, $fn) {
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
    return null;
  }
  public function current() {
    return ($this->fn)($this->it->current());
  }
}

class LazyMapIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn) {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator() {
    return new LazyMapIterator($this->iterable->getIterator(), $this->fn);
  }
}

class LazyMapKeyedIterator implements \HH\KeyedIterator {
  private $it;
  private $fn;

  public function __construct($it, $fn) {
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

  public function __construct($iterable, $fn) {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator() {
    return new LazyMapKeyedIterator($this->iterable->getIterator(), $this->fn);
  }
}

class LazyMapWithKeyIterator implements \HH\KeyedIterator {
  private $it;
  private $fn;

  public function __construct($it, $fn) {
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

  public function __construct($iterable, $fn) {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator() {
    return new LazyMapWithKeyIterator($this->iterable->getIterator(),
                                      $this->fn);
  }
}

class LazyFilterIterator implements \HH\Iterator {
  private $it;
  private $fn;

  public function __construct($it, $fn) {
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
    return null;
  }
  public function current() {
    return $this->it->current();
  }
}

class LazyFilterIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn) {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator() {
    return new LazyFilterIterator($this->iterable->getIterator(), $this->fn);
  }
}

class LazyFilterKeyedIterator implements \HH\KeyedIterator {
  private $it;
  private $fn;

  public function __construct($it, $fn) {
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

  public function __construct($iterable, $fn) {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator() {
    return
      new LazyFilterKeyedIterator($this->iterable->getIterator(), $this->fn);
  }
}

class LazyFilterWithKeyIterator implements \HH\KeyedIterator {
  private $it;
  private $fn;

  public function __construct($it, $fn) {
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

  public function __construct($iterable, $fn) {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator() {
    return
      new LazyFilterWithKeyIterator($this->iterable->getIterator(), $this->fn);
  }
}

class LazyZipIterator implements \HH\Iterator {
  private $it1;
  private $it2;

  public function __construct($it1, $it2) {
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
    return null;
  }
  public function current() {
    return Pair {$this->it1->current(), $this->it2->current()};
  }
}

class LazyZipIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable1;
  private $iterable2;

  public function __construct($iterable1, $iterable2) {
    $this->iterable1 = $iterable1;
    $this->iterable2 = $iterable2;
  }
  public function getIterator() {
    return new LazyZipIterator($this->iterable1->getIterator(),
                               $this->iterable2->getIterator());
  }
}

class LazyZipKeyedIterator implements \HH\KeyedIterator {
  private $it1;
  private $it2;

  public function __construct($it1, $it2) {
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

  public function __construct($iterable1, $iterable2) {
    $this->iterable1 = $iterable1;
    $this->iterable2 = $iterable2;
  }
  public function getIterator() {
    return new LazyZipKeyedIterator($this->iterable1->getIterator(),
                                    $this->iterable2->getIterator());
  }
}

class LazyTakeIterator implements \HH\Iterator {
  private $it;
  private $n;
  private $numLeft;

  public function __construct($it, $n) {
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
    return null;
  }
  public function current() {
    return $this->it->current();
  }
}

class LazyTakeIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $n;

  public function __construct($iterable, $n) {
    $this->iterable = $iterable;
    $this->n = $n;
  }
  public function getIterator() {
    return new LazyTakeIterator($this->iterable->getIterator(),
                                $this->n);
  }
}

class LazyTakeKeyedIterator implements \HH\KeyedIterator {
  private $it;
  private $n;
  private $numLeft;

  public function __construct($it, $n) {
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

  public function __construct($iterable, $n) {
    $this->iterable = $iterable;
    $this->n = $n;
  }
  public function getIterator() {
    return new LazyTakeKeyedIterator($this->iterable->getIterator(),
                                     $this->n);
  }
}

class LazyTakeWhileIterator implements \HH\Iterator {
  private $it;
  private $fn;

  public function __construct($it, $fn) {
    $this->it = $it;
    $this->fn = $fn;
    while ($it->valid() && $fn($it->current())) {
      $it->next();
    }
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
    return null;
  }
  public function current() {
    return $this->it->current();
  }
}

class LazyTakeWhileIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn) {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator() {
    return new LazyTakeWhileIterator($this->iterable->getIterator(),
                                     $this->fn);
  }
}

class LazyTakeWhileKeyedIterator implements \HH\Iterator {
  private $it;
  private $fn;

  public function __construct($it, $fn) {
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

class LazyTakeWhileKeyedIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn) {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator() {
    return new LazyTakeWhileKeyedIterator($this->iterable->getIterator(),
                                          $this->fn);
  }
}

class LazySkipIterator implements \HH\Iterator {
  private $it;
  private $n;

  public function __construct($it, $n) {
    $this->it = $it;
    $this->n = $n;
    while ($n > 0 && $it->valid()) {
      $it->next();
      --$n;
    }
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $it = $this->it;
    $n = $this->n;
    $it->rewind();
    while ($n > 0 && $it->valid()) {
      $it->next();
      --$n;
    }
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

class LazySkipIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $n;

  public function __construct($iterable, $n) {
    $this->iterable = $iterable;
    $this->n = $n;
  }
  public function getIterator() {
    return new LazySkipIterator($this->iterable->getIterator(),
                                $this->n);
  }
}

class LazySkipKeyedIterator implements \HH\KeyedIterator {
  private $it;
  private $n;

  public function __construct($it, $n) {
    $this->it = $it;
    $this->n = $n;
    while ($n > 0 && $it->valid()) {
      $it->next();
      --$n;
    }
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $it = $this->it;
    $n = $this->n;
    $it->rewind();
    while ($n > 0 && $it->valid()) {
      $it->next();
      --$n;
    }
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
    return $this->it->current();
  }
}

class LazySkipKeyedIterable implements \HH\KeyedIterable {
  use LazyKeyedIterable;

  private $iterable;
  private $n;

  public function __construct($iterable, $n) {
    $this->iterable = $iterable;
    $this->n = $n;
  }
  public function getIterator() {
    return new LazySkipKeyedIterator($this->iterable->getIterator(),
                                     $this->n);
  }
}

class LazySkipWhileIterator implements \HH\Iterator {
  private $it;
  private $fn;

  public function __construct($it, $fn) {
    $this->it = $it;
    $this->fn = $fn;
    while ($it->valid() && $fn($it->current())) {
      $it->next();
    }
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $it = $this->it;
    $fn = $this->fn;
    $it->rewind();
    while ($it->valid() && $fn($it->current())) {
      $it->next();
    }
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

class LazySkipWhileIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn) {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator() {
    return new LazySkipWhileIterator($this->iterable->getIterator(),
                                     $this->fn);
  }
}

class LazySkipWhileKeyedIterator implements \HH\Iterator {
  private $it;
  private $fn;

  public function __construct($it, $fn) {
    $this->it = $it;
    $this->fn = $fn;
    while ($it->valid() && $fn($it->current())) {
      $it->next();
    }
  }
  public function __clone() {
    $this->it = clone $this->it;
  }
  public function rewind() {
    $it = $this->it;
    $fn = $this->fn;
    $it->rewind();
    while ($it->valid() && $fn($it->current())) {
      $it->next();
    }
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
    return $this->it->current();
  }
}

class LazySkipWhileKeyedIterable implements \HH\Iterable {
  use LazyIterable;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn) {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator() {
    return new LazySkipWhileKeyedIterator($this->iterable->getIterator(),
                                          $this->fn);
  }
}

class LazyKeysIterator implements \HH\Iterator {
  private $it;

  public function __construct($it) {
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

  public function __construct($iterable) {
    $this->iterable = $iterable;
  }
  public function getIterator() {
    return new LazyKeysIterator($this->iterable->getIterator());
  }
}

class LazyValuesIterator implements \HH\Iterator {
  private $it;

  public function __construct($it) {
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

  public function __construct($iterable) {
    $this->iterable = $iterable;
  }
  public function getIterator() {
    return new LazyValuesIterator($this->iterable->getIterator());
  }
}

class LazyKVZipIterator implements \HH\Iterator {
  private $it;

  public function __construct($it) {
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

  public function __construct($iterable) {
    $this->iterable = $iterable;
  }
  public function getIterator() {
    return new LazyKVZipIterator($this->iterable->getIterator());
  }
}

class LazyIterableView implements \HH\Iterable {
  public $iterable;

  public function __construct($iterable) { $this->iterable = $iterable; }
  public function getIterator() { return $this->iterable->getIterator(); }
  public function toArray() {
    $arr = array();
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
  public function map($callback) {
    return new LazyMapIterable($this->iterable, $callback);
  }
  public function filter($callback) {
    return new LazyFilterIterable($this->iterable, $callback);
  }
  public function zip($iterable) {
    return new LazyZipIterable($this->iterable, $iterable);
  }
  public function take($n) {
    return new LazyTakeIterable($this->iterable, $n);
  }
  public function takeWhile($fn) {
    return new LazyTakeWhileIterable($this->iterable, $fn);
  }
  public function skip($n) {
    return new LazySkipIterable($this->iterable, $n);
  }
  public function skipWhile($fn) {
    return new LazySkipWhileIterable($this->iterable, $fn);
  }
}

class LazyKeyedIterableView implements \HH\KeyedIterable {
  public $iterable;

  public function __construct($iterable) { $this->iterable = $iterable; }
  public function getIterator() { return $this->iterable->getIterator(); }
  public function toArray() {
    $arr = array();
    foreach ($this->iterable as $k => $v) {
      $arr[$k] = $v;
    }
    return $arr;
  }
  public function toValuesArray() {
    $arr = array();
    foreach ($this->iterable as $v) {
      $arr[] = $v;
    }
    return $arr;
  }
  public function toKeysArray() {
    $arr = array();
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
  public function map($callback) {
    return new LazyMapKeyedIterable($this->iterable, $callback);
  }
  public function mapWithKey($callback) {
    return new LazyMapWithKeyIterable($this->iterable, $callback);
  }
  public function filter($callback) {
    return new LazyFilterKeyedIterable($this->iterable, $callback);
  }
  public function filterWithKey($callback) {
    return new LazyFilterWithKeyIterable($this->iterable, $callback);
  }
  public function zip($iterable) {
    return new LazyZipKeyedIterable($this->iterable, $iterable);
  }
  public function take($n) {
    return new LazyTakeKeyedIterable($this->iterable, $n);
  }
  public function takeWhile($fn) {
    return new LazyTakeWhileKeyedIterable($this->iterable, $fn);
  }
  public function skip($n) {
    return new LazySkipKeyedIterable($this->iterable, $n);
  }
  public function skipWhile($fn) {
    return new LazySkipWhileKeyedIterable($this->iterable, $fn);
  }
  public function values() {
    return new LazyValuesIterable($this->iterable);
  }
  public function keys() {
    return new LazyKeysIterable($this->iterable);
  }
  public function kvzip() {
    return new LazyKVZipIterable($this->iterable);
  }
}

}
