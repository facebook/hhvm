<?php

class X {
  static $y = array();

  function z() {
    self::$y[] = 2;
    return self::$y;
  }
}


<<__EntryPoint>>
function main_public_static_props_016() {
var_dump(X::z());
var_dump(X::z());
}
