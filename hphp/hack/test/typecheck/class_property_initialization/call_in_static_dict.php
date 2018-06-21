<?php

class C {
  public static function f() {
    return "lol";
  }
  static $x = dict['a' => C::f()];
}
