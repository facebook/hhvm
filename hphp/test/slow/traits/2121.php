<?php

trait T1 {
 abstract function foo();
 }
trait T2 {
 abstract function foo();
 }
abstract class B {
  use T1, T2;
}
class C extends B {
  function foo() {
 return "hello\n";
 }
}
$o = new C;
echo $o->foo();
