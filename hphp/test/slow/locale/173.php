<?php

class A {
 public $a;
 function __toString() {
 return $this->a;
}
}

 <<__EntryPoint>>
function main_173() {
$a = new A;
 $a->a = 'a';
 $b = new A;
 $b->a = 'b';
 $arr = array($a, $b);
 sort(&$arr, SORT_REGULAR);
 print ((string)$arr[0]);
}
