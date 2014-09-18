<?php

class X {
  static $y = array();

  function z() {
    self::$y[] = 2;
    return self::$y;
  }
}

var_dump(X::z());
var_dump(X::z());
