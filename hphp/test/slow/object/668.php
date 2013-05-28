<?php
class X {
  static function f($o, $s) {
    return $o instanceof $s;
  }
}
$x = new X;
var_dump(X::f($x, 'self'));
var_dump(X::f($x, 'X'));
