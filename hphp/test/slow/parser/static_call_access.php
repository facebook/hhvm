<?php

class C {
  const X = 1;

  static function f() {
    return new C();
  }
}

var_dump(C::f()::X);
var_dump(C::{'f' . ''}()::X);
