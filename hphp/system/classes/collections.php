<?php

class KeysIterator implements Iterator {
  public $it;
  public function __construct($it) {
    $this->it = $it;
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
    throw new RuntimeException(
      "Call to undefined method KeysIterator::keys()");
  }
  public function current() {
    return $this->it->key();
  }
}

class KeysIterable implements Iterable {
  public $it;
  public function __construct($iterable) {
    $this->it = new KeysIterator($iterable->getIterator());
  }
  public function getIterator() {
    return clone $this->it;
  }
}

class MapItemsIterator implements Iterator {
  public $it;
  public function __construct($it) {
    $this->it = $it;
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
    throw new RuntimeException(
      "Call to undefined method MapItemsIterator::keys()");
  }
  public function current() {
    return Pair {$this->it->key(), $this->it->current()};
  }
}

class MapItemsIterable implements Iterable {
  public $it;
  public function __construct($iterable) {
    $this->it = new MapItemsIterator($iterable->getIterator());
  }
  public function getIterator() {
    return clone $this->it;
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

