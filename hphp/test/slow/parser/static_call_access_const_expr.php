<?php

class C {
  const X = 1;

  static function f() {
    return new C();
  }

  const Y = C::f()::X;
}
