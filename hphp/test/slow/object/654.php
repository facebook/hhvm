<?php

Object654::$a = 1;
 class A {
 public function t() {

 var_dump(Object654::$a);
}
}
 $obj = new A();
 $obj->t();

abstract final class Object654 {
  public static $a;
}
