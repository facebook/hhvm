<?hh

class A {
  static function f() :mixed{
 return new static;
 }
  static function g($o) :mixed{
 return $o is this;
 }
}
class B extends A {
 }

<<__EntryPoint>>
function main_1881() :mixed{
var_dump(A::g(A::f()));
var_dump(A::g(B::f()));
var_dump(B::g(A::f()));
var_dump(B::g(B::f()));
}
