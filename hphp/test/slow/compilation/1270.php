<?php

class A {
 function __call($a, $b) {
 $b = 'a';
 $b = 1;
 var_dump($a, $b[0], $b[1]);
}
}
 $obj = new A();
 $a = 1;
 $b = 'a';
 $b = 2;
 $obj->test($a, $b);
