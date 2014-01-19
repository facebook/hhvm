<?php

class A {
 public $prop = 1;
}
 class B {
 public $prop = 5;
}
 $a = 1;
 $a = new A();
 $a->prop++;
 var_dump($a->prop);
