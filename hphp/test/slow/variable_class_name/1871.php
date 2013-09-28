<?php

class B {
  function f4($arguments) {
    var_dump($arguments);
  }
}
class G extends B {
  function f4($a) {
    $b='B';
    $b::f4(5);
 // __call
  }
}
$g = new G(5);
$g->f4(3);
