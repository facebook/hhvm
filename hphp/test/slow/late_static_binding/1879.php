<?php

class X {
  static function foo() {
 echo "X::foo\n";
 }
  function bar() {
 static::foo();
 }
}
class Y extends X {
  static function foo() {
 echo "Y::foo\n";
 }
  function baz() {
 X::bar();
 }
}
$y = new Y;
$y->baz();
Y::baz();
