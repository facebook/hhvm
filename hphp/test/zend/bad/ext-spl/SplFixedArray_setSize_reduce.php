<?php
$array = new SplFixedArray(5);
$array[0] = 'a';
$array[1] = 'b';
$array[2] = 'c';
$array[3] = 'd';
$array[4] = 'e';
$array->setSize(3);
print_r($array);
?>