<?php

class AB {
  static function foo() {
    var_dump('foo');
  }
}
function f($x) {
  $a = $x.'B';
  $a::foo();
}
f('A');
