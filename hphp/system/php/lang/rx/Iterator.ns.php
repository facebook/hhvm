<?php

namespace HH\Rx;

interface Iterator extends namespace\Traversable, \HH\Iterator {
  <<__Rx>>
  public function current();
  <<__Rx>>
  public function key();
  <<__Rx, __Mutable>>
  public function next();
  <<__Rx, __Mutable>>
  public function rewind();
  <<__Rx>>
  public function valid();
}
