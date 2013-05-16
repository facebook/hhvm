<?php

trait IterableTrait {
  public function view() {
    return $this;
  }
  public function map($callback) {
    return new MappedIterable($this, $callback);
  }
  public function filter($callback) {
    return new FilteredIterable($this, $callback);
  }
  public function zip($iterable) {
    return new ZippedIterable($this, $iterable);
  }
}

trait KeyedIterableTrait {
  public function view() {
    return $this;
  }
  public function map($callback) {
    return new MappedKeyedIterable($this, $callback);
  }
  public function filter($callback) {
    return new FilteredKeyedIterable($this, $callback);
  }
  public function zip($iterable) {
    return new ZippedKeyedIterable($this, $iterable);
  }
  public function keys() {
    return new KeysIterable($this);
  }
  public function kvzip() {
    return new KVZippedIterable($this);
  }
}

class MappedIterator implements Iterator {
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

class MappedIterable implements Iterable {
  use IterableTrait;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn) {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator() {
    return new MappedIterator($this->iterable->getIterator(), $this->fn);
  }
}

class MappedKeyedIterator implements KeyedIterator {
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

class MappedKeyedIterable implements KeyedIterable {
  use KeyedIterableTrait;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn) {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator() {
    return new MappedKeyedIterator($this->iterable->getIterator(), $this->fn);
  }
}

class FilteredIterator implements Iterator {
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

class FilteredIterable implements Iterable {
  use IterableTrait;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn) {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator() {
    return new FilteredIterator($this->iterable->getIterator(), $this->fn);
  }
}

class FilteredKeyedIterator implements KeyedIterator {
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

class FilteredKeyedIterable implements KeyedIterable {
  use KeyedIterableTrait;

  private $iterable;
  private $fn;

  public function __construct($iterable, $fn) {
    $this->iterable = $iterable;
    $this->fn = $fn;
  }
  public function getIterator() {
    return new FilteredKeyedIterator($this->iterable->getIterator(), $this->fn);
  }
}

class ZippedIterator implements Iterator {
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

class ZippedIterable implements Iterable {
  use IterableTrait;

  private $iterable1;
  private $iterable2;

  public function __construct($iterable1, $iterable2) {
    $this->iterable1 = $iterable1;
    $this->iterable2 = $iterable2;
  }
  public function getIterator() {
    return new ZippedIterator($this->iterable1->getIterator(),
                              $this->iterable2->getIterator());
  }
}

class ZippedKeyedIterator implements KeyedIterator {
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

class ZippedKeyedIterable implements KeyedIterable {
  use KeyedIterableTrait;
  
  private $iterable1;
  private $iterable2;

  public function __construct($iterable1, $iterable2) {
    $this->iterable1 = $iterable1;
    $this->iterable2 = $iterable2;
  }
  public function getIterator() {
    return new ZippedKeyedIterator($this->iterable1->getIterator(),
                                   $this->iterable2->getIterator());
  }
}

class KeysIterator implements Iterator {
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

class KeysIterable implements Iterable {
  use IterableTrait;

  private $iterable;

  public function __construct($iterable) {
    $this->iterable = $iterable;
  }
  public function getIterator() {
    return new KeysIterator($this->iterable->getIterator());
  }
}

class KVZippedIterator implements Iterator {
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

class KVZippedIterable implements Iterable {
  use IterableTrait;

  private $iterable;

  public function __construct($iterable) {
    $this->iterable = $iterable;
  }
  public function getIterator() {
    return new KVZippedIterator($this->iterable->getIterator());
  }
}

interface ConstCollection extends Countable {
  public function isEmpty();
  public function count();
  public function items();
}

interface OutputCollection {
  public function add($e);
  public function addAll($iterable);
}

interface Collection extends ConstCollection,
                             OutputCollection {
  public function clear();
}

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

interface ConstVector extends ConstCollection,
                              ConstIndexAccess,
                              KeyedIterable {
}

interface MutableVector extends ConstVector,
                                Collection,
                                IndexAccess {
}

interface ConstMap extends ConstCollection,
                           ConstMapAccess,
                           KeyedIterable {
}

interface MutableMap extends ConstMap,
                             Collection,
                             MapAccess {
}

interface ConstSet extends ConstCollection,
                           ConstSetAccess,
                           Iterable {
}

interface MutableSet extends ConstSet,
                             Collection,
                             SetAccess {
}

class IterableView implements Iterable {
  public $iterable;

  public function __construct($iterable) { $this->iterable = $iterable; }
  public function getIterator() { return $this->iterable->getIterator(); }
  public function view() {
    return $this;
  }
  public function map($callback) {
    return new MappedIterable($this->iterable, $callback);
  }
  public function filter($callback) {
    return new FilteredIterable($this->iterable, $callback);
  }
  public function zip($iterable) {
    return new ZippedIterable($this->iterable, $iterable);
  }
}

class KeyedIterableView implements KeyedIterable {
  public $iterable;

  public function __construct($iterable) { $this->iterable = $iterable; }
  public function getIterator() { return $this->iterable->getIterator(); }
  public function view() {
    return $this;
  }
  public function map($callback) {
    return new MappedKeyedIterable($this->iterable, $callback);
  }
  public function filter($callback) {
    return new FilteredKeyedIterable($this->iterable, $callback);
  }
  public function zip($iterable) {
    return new ZippedKeyedIterable($this->iterable, $iterable);
  }
  public function keys() {
    return new KeysIterable($this->iterable);
  }
  public function kvzip() {
    return new KVZippedIterable($this->iterable);
  }
}

