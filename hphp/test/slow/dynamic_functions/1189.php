<?php

function bar($flag) {
  $ar0 =   array('b' => 3, 'a' => 2, 'c' => 1);
  $ar1 =   array('x' => 6, 'y' => 4, 'z' => 5);
  $ar2 =   array('p' => 8, 'q' => 9, 'r' => 7);
  $arr = array(&$ar0, &$ar1, &$ar2);
  if ($flag) {
 $f = 'var_dump';
 } else {
 $f = 'array_multisort';
 }
  $f(&$ar0, &$ar1, &$ar2);
  var_dump($arr);
}
bar($argc > 100);
