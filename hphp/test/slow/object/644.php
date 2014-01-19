<?php

class A {
 public $a = 0;
}
 class B extends A {
}
$obj1 = new A();
 $obj2 = new A();
 $obj2->a++;
 $obj3 = new B();
 $obj3->a = 10;
var_dump($obj1->a);
var_dump($obj1);
var_dump($obj2->a);
var_dump($obj2);
var_dump($obj3);
var_dump($obj1 instanceof A);
var_dump($obj3 instanceof A);
var_dump($obj1 instanceof B);
var_dump($obj3 instanceof B);
