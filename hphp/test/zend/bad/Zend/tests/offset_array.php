<?php

$arr = array(1,2,3,4,5,6,7,8);

var_dump($arr[1]);
var_dump($arr[0.0836]);
var_dump($arr[NULL]);
var_dump($arr["run away"]);

var_dump($arr[TRUE]);
var_dump($arr[FALSE]);

$fp = fopen(__FILE__, "r");
var_dump($arr[$fp]);

$obj = new stdClass;
var_dump($arr[$obj]);

$arr1 = Array(1,2,3);
var_dump($arr[$arr1]);

echo "Done\n";
?>