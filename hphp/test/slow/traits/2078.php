<?php

trait T {
  public function m() {
 echo "original
";
 }
}
class A {
 use T;
 }
class B {
 use T;
 }

<<__EntryPoint>>
function main_2078() {
$a1 = new A;
$a1->m();
fb_intercept("A::m", function() {
 echo "new
";
 }
);
$a2 = new A;
$a2->m();
$b1 = new B;
$b1->m();
T::m();
}
