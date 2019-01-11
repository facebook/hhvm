<?php

class A {
 public $a;
 }

<<__EntryPoint>>
function main_174() {
$a = new A;
 $a->a = 'a';
 $b = new A;
 $b->a = 'b';
 $arr = array($b, $a);
print $arr[0]->a;
sort(&$arr, SORT_REGULAR);
 print $arr[0]->a;
}
