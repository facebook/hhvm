<?php

class A {
  const i1= -1;
  const i2= -2;
  static $s = -4;
}
;
class B {
  static $s = -5;
}
;
$attr=array();
$attr[a::i1]='abc';
$attr[a::i2]='def';
$attr[-3]='ghi';
$attr[a::$s]='jkl';
$attr[b::$s]='mno';
var_dump($attr);
