<?php

$i = 'gi';
 $s = 'gs';
 class A {
 public function &dyn_test(&$a) {
 global $i;
 $a = $i;
 return $i;
}
}
 $obj = new A();
 $f = 'dyn_test';
 $c = &$obj->$f($b);
 var_dump($b);
 var_dump($c);
