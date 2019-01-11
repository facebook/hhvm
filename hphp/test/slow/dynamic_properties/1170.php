<?php

class A {
 }
 function f(&$a) {
 $a = 1000;
 }


 <<__EntryPoint>>
function main_1170() {
$a = new A();
 $f = 10;
 $a->$f = 100;
 var_dump($a);
 var_dump((array)$a);
 $f = 100;
 f(&$a->$f);
 var_dump($a);
}
