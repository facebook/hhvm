<?php

class C {
  static function f() {
    return "lol";
  }
  static $x = darray['a' => C::f()];
}
