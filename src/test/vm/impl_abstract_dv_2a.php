  <?php
 abstract class A  { abstract public function foo(int $x); }
 class B extends A {          public function foo(int $x)   {} }
 class C extends B {          public function foo(array $x) {} }
 $o = new C;
 $o->foo(array());
 echo "OK\n";

