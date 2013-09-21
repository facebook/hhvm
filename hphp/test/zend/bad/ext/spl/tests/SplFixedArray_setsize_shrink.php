<?php
$array = new SplFixedArray(5);
$array[0] = 1;
$array[1] = 1;
$array[2] = 1;
$array[3] = 1;
$array[4] = 1;

$array->setSize(4);
var_dump($array);

?>