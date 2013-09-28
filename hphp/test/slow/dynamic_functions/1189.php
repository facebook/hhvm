<?php

function bar($flag) {
  $arr = array(array('b' => 3, 'a' => 2, 'c' => 1),               array('x' => 6, 'y' => 4, 'z' => 5),               array('p' => 8, 'q' => 9, 'r' => 7));
  if ($flag) {
 $f = 'var_dump';
 }
 else {
 $f = 'array_multisort';
 }
  $f($arr[0], $arr[1], $arr[2]);
  var_dump($arr);
}
bar($argc > 100);
