<?php

class X {
  static function g() {
}
}
;
@X::g();
function g($a,$b) {
}
function f() {
 return 3;
 }
@g(f(),f());
