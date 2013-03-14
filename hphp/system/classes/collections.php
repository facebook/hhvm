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
    return Tuple {$this->it->key(), $this->it->current()};
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

