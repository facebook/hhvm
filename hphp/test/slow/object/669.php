<?php
class X {
  static function f($o) {
    $s = 'self';
    return $o instanceof $s;
  }
}

<<__EntryPoint>>
function main_669() {
var_dump(X::f(new X));
}
