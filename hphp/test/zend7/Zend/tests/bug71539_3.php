<?php
$array = [];
$array[0][0] =& $array[''];
$array[0][0] = 42;
var_dump($array);
?>
