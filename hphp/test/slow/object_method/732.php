<?php

class c {
  function x() {
    var_dump($this);
    $t = 'this';
    var_dump($$t);
  }
}

<<__EntryPoint>>
function main_732() {
$x = new c;
$x->x();
}
