<?php

$array = array('a', 'b');
array_splice($array, 0, 2);
$array[] = 'c';
var_dump($array);

$array = array('a', 'b');
array_shift($array);
array_shift($array);
$array[] = 'c';
var_dump($array);

?>
