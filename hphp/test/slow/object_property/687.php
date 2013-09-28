<?php

class A {
 }
$a = new A();
$f = 20;
$a->$f = 100;
var_dump($a);
unset($a->$f);
var_dump($a);
