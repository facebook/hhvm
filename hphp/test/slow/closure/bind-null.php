<?php

class X {
  static function test() {
    $f = function () { };
    $g = $f->bindto(null, null);
    return $g;
  }
}


<<__EntryPoint>>
function main_bind_null() {
$f = X::test();
$f();
}
