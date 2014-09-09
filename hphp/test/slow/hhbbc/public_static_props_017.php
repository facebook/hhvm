<?php

class X {
  static $y = array(1,2,3);

  static function go() {
    unset(self::$y[0]);
  }
  static function y() {
    return self::$y;
  }
}

X::go();
var_dump(X::y());
