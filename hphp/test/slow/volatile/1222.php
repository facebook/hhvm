<?php

class_exists('c');
class c {
  const A = 'a';
  const B = 'b';
  const C = 'c';
  const D = 'd';
  public static $S = array(    self::A,    self::B,    self::C,    self::D);
}
var_dump(c::$S);
