<?php

// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);

$a = array(0, 1, array(2, 3), 4);
foreach ($a as &$v) {
  print("v: $v\n");
  $v = "v";
}
var_dump($a);

$a = array(0, 1, array(2, 3), 4);
foreach ($a as $k => &$v) {
  print("k: $k, v: $v\n");
  $v = "v";
}
var_dump($a);
