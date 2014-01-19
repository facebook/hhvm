<?php

class A {
 function __construct($a) {
 var_dump($a);
}
 }
 class B extends A {
}
 $a = new B('test');
