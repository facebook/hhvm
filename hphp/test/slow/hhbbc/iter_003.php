<?php

class A { function heh() { echo "heh\n"; } }
function foo() {
  $x = array('foo' => new A, 'bar' => new A);
  foreach ($x as $k => $v) {
    var_dump($k);
    var_dump($v);
  }
}

<<__EntryPoint>>
function main_iter_003() {
foo();
}
