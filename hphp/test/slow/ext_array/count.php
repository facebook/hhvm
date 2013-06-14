<?php

$a = array();
$a[0] = 1;
$a[1] = 3;
$a[2] = 5;
var_dump(count($a));

$b = array();
$b[0] = 7;
$b[5] = 9;
$b[10] = 11;
var_dump(count($b));

var_dump(count(null));
var_dump(count(false));

$food = array(
  "fruits" => array("orange", "banana", "apple"),
  "veggie" => array("carrot", "collard", "pea")
);
var_dump(count($food, COUNT_RECURSIVE));
var_dump(count($food));
