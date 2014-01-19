<?php

class A {
 }
$o = new A;
$f = '10';
$o->$f = 100;
$a = (array)$o;
$v = 1;
$a[10] = &$v;
$a[11] = array(&$v);
var_dump($a);
$b = array(10 => 10);
var_dump(array_diff_key($a, $b));
var_dump(array_merge($a, $b));
var_dump(array_merge_recursive($a, $b));
var_dump(array_reverse($a));
var_dump(array_chunk($a, 2));
