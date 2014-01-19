<?php

class C {
  public $foo = 123;
  public function __unset($k) {
 echo "__unset $k\n";
 }
}
$obj = new C;
unset($obj->foo);
