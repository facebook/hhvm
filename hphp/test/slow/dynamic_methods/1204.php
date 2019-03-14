<?php

$i = 'gi';
 DynamicMethods1204::$s = 'gs';
 class A {
 public static function dyn_test(&$a) {

 $a = DynamicMethods1204::$s;
 return DynamicMethods1204::$s;
}
}
 $f = 'dyn_test';
 $e = A::$f(&$d);
 var_dump($d);
 var_dump($e);

abstract final class DynamicMethods1204 {
  public static $s;
}
