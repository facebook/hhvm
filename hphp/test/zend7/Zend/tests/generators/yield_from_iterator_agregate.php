<?php
class foo implements \IteratorAggregate {
  public $prop = 1;
  function getIterator() {
    var_dump($this->prop);
    yield;
  }
}
(function(){
  yield from new foo;
})()->next();
?>
