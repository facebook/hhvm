<?php

class C {
  static function f() {
    return "lol";
  }
  static $x = array(C::f());
}
