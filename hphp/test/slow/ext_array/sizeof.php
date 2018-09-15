<?php


<<__EntryPoint>>
function main_sizeof() {
$a = array();
$a[0] = 1;
$a[1] = 3;
$a[2] = 5;
var_dump(sizeof($a));

$b = array();
$b[0] = 7;
$b[5] = 9;
$b[10] = 11;
var_dump(sizeof($b));

var_dump(sizeof(null));
var_dump(sizeof(false));

$food = array(
  "fruits" => array("orange", "banana", "apple"),
  "veggie" => array("carrot", "collard", "pea")
);
var_dump(sizeof($food, COUNT_RECURSIVE));
var_dump(sizeof($food));
}
