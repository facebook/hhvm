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

///////////////////////////////////////////////////////////////////////////////

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


///////////////////////////////////////////////////////////////////////////////
// http://www.php.net/~helly/php/ext/spl/appenditerator_8inc-source.html

class AppendIterator implements OuterIterator {
  private $iterators;

  function __construct() {
    $this->iterators = new ArrayIterator();
  }

  function append(Iterator $it) {
    $this->iterators->append($it);
  }

  function getInnerIterator() {
    return $this->iterators->current();
  }

  function rewind() {
    $this->iterators->rewind();
    if ($this->iterators->valid()) {
      $this->getInnerIterator()->rewind();
    }
  }

  function valid() {
    return $this->iterators->valid() && $this->getInnerIterator()->valid();
  }

  function current() {
    /* Using $this->valid() would be exactly the same; it would omit
     * the access to a non valid element in the inner iterator. Since
     * the user didn't respect the valid() return value false this
     * must be intended hence we go on. */
    return $this->iterators->valid() ?
      $this->getInnerIterator()->current() : NULL;
  }

  function key() {
    return $this->iterators->valid() ? $this->getInnerIterator()->key() : NULL;
  }

  function next() {
    if (!$this->iterators->valid()){
      return; /* done all */
    }
    $this->getInnerIterator()->next();
    if ($this->getInnerIterator()->valid()) {
      return; /* found valid element in current inner iterator */
    }
    $this->iterators->next();
    while ($this->iterators->valid()) {
      $this->getInnerIterator()->rewind();
      if ($this->getInnerIterator()->valid()) {
        return; /* found element as first elemet in another iterator */
      }
      $this->iterators->next();
    }
  }

  function __call($func, $params) {
    return call_user_func_array(array($this->getInnerIterator(), $func),
                                $params);
  }
}
