<?php

namespace HH\Rx {

interface IteratorAggregate extends namespace\Traversable, \IteratorAggregate {
  <<__Rx, __MutableReturn>>
  public function getIterator();
}

interface Iterable extends namespace\IteratorAggregate, \HH\Iterable {}

}
