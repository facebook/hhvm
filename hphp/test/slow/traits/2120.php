<?php

trait T1 {
 abstract function foo();
 }
trait T2 {
 abstract function foo();
 }
class B {
  function foo() {
 return "hello\n";
 }
}
class C extends B {
  use T1, T2;
}
$o = new C;
echo $o->foo();
