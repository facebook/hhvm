<?php
class X {
  static function f($o) {
    $s = 'self';
    return $o instanceof $s;
  }
}
var_dump(X::f(new X));
