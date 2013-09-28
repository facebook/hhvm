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
A::test();
