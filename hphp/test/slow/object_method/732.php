<?php

class c {
  function x() {
    var_dump($this);
    $t = 'this';
    var_dump($$t);
  }
}
$x = new c;
$x->x();
