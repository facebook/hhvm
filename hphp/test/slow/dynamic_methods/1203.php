<?php

DynamicMethods1203::$i = 'gi';
 $s = 'gs';
 class A {
 public function dyn_test(&$a) {

 $a = DynamicMethods1203::$i;
 return DynamicMethods1203::$i;
}
}
 $obj = new A();
 $f = 'dyn_test';
 $c = $obj->$f(&$b);
 var_dump($b);
 var_dump($c);

abstract final class DynamicMethods1203 {
  public static $i;
}
