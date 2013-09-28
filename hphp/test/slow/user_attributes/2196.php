<?php

final class B {
  final public function foo() {
 echo "B::foo\n";
 }
  final public static function bar() {
 echo "B::bar\n";
 }
}
<< __MockClass >>
class C extends B {
  public function foo() {
 echo "C::foo\n";
 }
  public static function bar() {
 echo "C::bar\n";
 }
}
function test() {
  $obj = new C;
  $obj->foo();
  C::bar();
}
test();
