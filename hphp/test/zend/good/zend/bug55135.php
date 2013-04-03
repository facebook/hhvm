<?php
// This fails.
$array = array(1 => 2);
$a = "1";
unset($array[$a]);
print_r($array);

// Those works.
$array = array(1 => 2);
$a = 1;
unset($array[$a]);
print_r($array);

$array = array(1 => 2);
unset($array[1]);
print_r($array);

$array = array(1 => 2);
$a = 1;
unset($array["1"]);
print_r($array);
?>