<?php

$str = '';
$arr1 = array('a', 'b');
$arr2 = $arr1;
foreach ($arr1 as $v1) {
  $str .= $v1;
  switch ($v1) {
  default:
    foreach ($arr2 as $v2) {
      $str .= $v2;
    }
  }
}
var_dump($str);
