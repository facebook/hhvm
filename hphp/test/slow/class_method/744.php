<?php

interface I {
  public function foo($x);
}
abstract class B implements I {
  abstract public function foo($x);
}
class C extends B {
  public function foo($x){
 echo "$x \n";
}
}
$obj = new C;
$obj->foo(1);
