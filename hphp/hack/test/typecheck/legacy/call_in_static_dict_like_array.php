<?php

class C {
  public static function f() {
    return "lol";
  }
  static $x = array('a' => C::f());
}
