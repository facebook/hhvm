<?php

interface Countable {
  public function count();
}

interface Serializable {
  public function serialize();
  public function unserialize($serialized);
}

interface Traversable {
}

interface SeekableIterator {
  public function seek($position);
}

interface OuterIterator {
  public function getInnerIterator();
}

interface Iterator extends Traversable {
  public function current();
  public function key();
  public function next();
  public function rewind();
  public function valid();
}

class ArrayIterator implements Iterator,
  ArrayAccess, SeekableIterator, Countable {
  private $arr;
  private $flags;

  public function __construct($array, $flags = SORT_REGULAR) {
    $this->arr = $array;
    $this->flags = $flags;
  }

  public function append($value) {
    $this->arr[] = $value;
  }

  public function asort() {
    return asort($this->arr, $this->flags);
  }

  public function count() {
    return count($this->arr);
  }

  public function current() {
    return current($this->arr);
  }

  public function getArrayCopy() {
    return $this->arr;
  }

  public function getFlags() {
    return $this->flags;
  }

  public function key() {
    return key($this->arr);
  }

  public function ksort() {
    return ksort($this->arr, $this->flags);
  }

  public function natcasesort() {
    return natcasesort($this->arr);
  }

  public function natsort() {
    return natsort($this->arr);
  }

  public function next() {
    return next($this->arr);
  }

  public function offsetExists($index) {
    return isset($this->arr[$index]);
  }

  public function offsetGet($index) {
    return $this->arr[$index];
  }

  public function offsetSet($index, $newval) {
    $this->arr[$index] = $newval;
  }

  public function offsetUnset($index) {
    unset($this->arr[$index]);
  }

  public function rewind() {
    return reset($this->arr);
  }

  public function seek($position) {
    reset($this->arr);
    for ($i = 0; $i < $position; $i++) {
      if (!next($this->arr)) {
        break;
      }
    }
  }

  public function setFlags($flags) {
    $this->flags = $flags;
  }

  public function uasort($cmp_function) {
    return uasort($this->arr, $cmp_function);
  }

  public function uksort($cmp_function) {
    return uksort($cmp_function);
  }

  public function valid() {
    return current($this->arr) !== false;
  }
}

interface FilterIterator extends Iterator,
  OuterIterator {
}

interface IteratorAggregate extends Traversable {
  public function getIterator();
}
