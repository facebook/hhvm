<?php

class X {
  static function test() {
    $f = function () { get_called_class(); };
    $g = $f->bindto(null, null);
    return $g;
  }
}

$f = X::test();
$f();
