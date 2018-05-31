<?php

class C {
  static function f() {
    return "lol";
  }
  static $x = vec[C::f()];
}
