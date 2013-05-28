<?php

class A {
 function f($a) {
 var_dump($a === null);
 }
 }
$a = true;
 $a = new A();
$a->f(array());
