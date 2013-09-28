<?php

trait T {
  abstract public function foo($x);
}
interface I {
  public function foo($x);
}
abstract class B implements I {
  use T;
}
class C extends B {
  public function foo($x){
 echo "$x \n";
}
}
$obj = new C;
$obj->foo(1);
