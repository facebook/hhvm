<?php

class c {
  protected $cm = 'get';
  function x() {
    var_dump($this->cm);
  }
}
class c2 extends c {
}
$y = new c;
$y->x();
$z = clone $y;
$z->x();
$y = new c2;
$y->x();
$z = clone $y;
$z->x();
