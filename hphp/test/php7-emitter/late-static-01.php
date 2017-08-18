<?php

class A {
  function __construct() {}
  static function what() { return "A"; }

  static function whatwhat() { return static::what(); }

  static function early() {
    return self::whatwhat();
  }

  static function late() {
    return static::whatwhat();
  }
}

class B extends A {
  function __construct() {}
  static function what() { return "B"; }
}

var_dump(B::early());
var_dump(B::late());
$x = "B";
var_dump($x::early());
var_dump($x::late());
