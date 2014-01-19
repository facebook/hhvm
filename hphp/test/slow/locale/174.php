<?php

class A {
 public $a;
 }
$a = new A;
 $a->a = 'a';
 $b = new A;
 $b->a = 'b';
 $arr = array($b, $a);
print $arr[0]->a;
sort($arr, SORT_REGULAR, true);
 print $arr[0]->a;
