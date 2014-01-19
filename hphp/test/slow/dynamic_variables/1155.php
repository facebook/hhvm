<?php

$a = 123;
$b = 456;
function foo() {
  global $a;
  $b = &$GLOBALS['b'];
  $d = 789;
 $e = 111;
  $c = &$d;
 $arr = get_defined_vars();
 ksort($arr);
 var_dump($arr);
 return $arr;
}
function bar($arr) {
  extract($arr, EXTR_REFS);
  var_dump($a, $b, $c, $d, $e);
  $a = 'aaa';
 $b = 'bbb';
 $c = 'ccc';
  var_dump($d);
}
bar(foo());
var_dump($a, $b);
