<?php

class X {
  static function foo() {
 global$g;
 return $g;
 }
}
function bar() {
  var_dump('Intercepted');
}
function test() {
  X::foo();
  fb_intercept('X::foo', 'bar', 'bar');
  X::foo();
}

<<__EntryPoint>>
function main_1200() {
test();
}
