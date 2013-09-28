<?php

$i = 'gi';
 $s = 'gs';
 class A {
 public static function &dyn_test(&$a) {
 global $s;
 $a = $s;
 return $s;
}
}
 $f = 'dyn_test';
 $e = A::$f($d);
 var_dump($d);
 var_dump($e);
