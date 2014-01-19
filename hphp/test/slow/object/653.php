<?php

class A {
 public $a;
}
 $obj1 = new A();
 $obj2 = new A();
 $obj1->a = $obj2;
 $obj2->a = $obj1;
var_dump($obj1);
