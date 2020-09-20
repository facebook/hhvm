<?hh

class A {
  static function f() {
 return new static;
 }
  static function g($o) {
 return $o is this;
 }
}
class B extends A {
 }

<<__EntryPoint>>
function main_1881() {
var_dump(A::g(A::f()));
var_dump(A::g(B::f()));
var_dump(B::g(A::f()));
var_dump(B::g(B::f()));
}
