<?php

$arr = array(0,1,2);
function f($val,$key) {
  global $arr;
  echo "k=$key v=$val\n";
  if ($key == 0) {
    unset($arr[1]);
  }
}
array_walk($arr,'f');
var_dump($arr);
