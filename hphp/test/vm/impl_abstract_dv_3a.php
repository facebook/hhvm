  <?php
 abstract class A  { abstract public function foo(int $x); }
 interface      I  {                 function foo(int $x); }
 abstract class B extends A implements I  { }
 class C extends B {          public function foo(array $x = null) {} }
 $c = new C;
 $c->foo(null);
 echo "OK\n";

