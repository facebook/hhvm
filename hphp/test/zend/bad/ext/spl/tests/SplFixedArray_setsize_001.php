<?php
$array = new SplFixedArray(5);
$array[0] = 'one';
$array[1] = 'two';
$array[2] = 'three';
$array[3] = 'four';
$array[4] = 'five';
$array->setSize(2);
var_dump($array);
?>