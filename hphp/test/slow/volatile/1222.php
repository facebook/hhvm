<?php
class c {
  const A = 'a';
  const B = 'b';
  const C = 'c';
  const D = 'd';
  public static $S = array(    self::A,    self::B,    self::C,    self::D);
}


<<__EntryPoint>>
function main_1222() {
class_exists('c');
var_dump(c::$S);
}
