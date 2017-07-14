<?php
$array = [0=>[]];
$array[0][0] =& $array[0][''];
$array[0][0] = 42;
var_dump($array);
?>
