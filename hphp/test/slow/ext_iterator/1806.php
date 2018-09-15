<?php

class EvensOnly extends FilterIterator {
  function __construct($it) {
    parent::__construct($it);
  }
  public function accept() {
    return $this->getInnerIterator()->current() % 2 == 0;
  }
}

<<__EntryPoint>>
function main_1806() {
$i = new EvensOnly(new ArrayIterator(range(0, 10)));
foreach ($i as $v) {
  var_dump($v);
}
}
