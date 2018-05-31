<?php

class C {
  static function f() {
    return "lol";
  }
  static $x = array('a' => C::f());
}
