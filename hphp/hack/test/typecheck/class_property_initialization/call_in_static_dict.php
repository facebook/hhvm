<?php

class C {
  static function f() {
    return "lol";
  }
  static $x = dict['a' => C::f()];
}
