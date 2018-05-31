<?php

class C {
  static function f() {
    return "lol";
  }
  static $x = shape('a' => C::f());
}
