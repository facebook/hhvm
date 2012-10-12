<?php

function fiz($a) { var_dump($a); }
class a {
  static function __callStatic($n, $a) { return $a[0]; }
}

class b extends a {
  static function bar() {
    fiz(parent::foo(42));
  }
}

b::bar();
