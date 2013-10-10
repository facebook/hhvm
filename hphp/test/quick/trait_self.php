<?php

trait T {
  static function foo() { self::bar(); }
  static function bar() { var_dump(__METHOD__); }
}

T::foo();
