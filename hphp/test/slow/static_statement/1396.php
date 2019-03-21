<?php

class A {
 static function test() {
  $static_var = 3;
  echo $static_var;
  static $static_var;
  $static_var ++;
  echo $static_var;
}
 }

<<__EntryPoint>>
function main_1396() {
A::test();
}
