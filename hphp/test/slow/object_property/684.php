<?php

function f() {
 return false;
 }
class B {
 var $a = 1;
 static $b = array(1, 2, 3);
 }

<<__EntryPoint>>
function main_684() {
if (f()) {
 class A {
 }
 }
else {
 class A {
 static $a = 100;
 var $b = 1000;
 }
 }
$vars = get_class_vars('A');
 asort($vars);
 var_dump($vars);
A::$a = 1;
$vars = get_class_vars('A');
 asort($vars);
 var_dump($vars);
$vars = get_class_vars('B');
 asort($vars);
 var_dump($vars);
}
