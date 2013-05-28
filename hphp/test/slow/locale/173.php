<?php

class A {
 public $a;
 function __toString() {
 return $this->a;
}
}
 $a = new A;
 $a->a = 'a';
 $b = new A;
 $b->a = 'b';
 $arr = array($a, $b);
 sort($arr, SORT_REGULAR, true);
 print ((string)$arr[0]);
