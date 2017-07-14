<?php
$array = [];
$array[0] =& $array[''];
$array[0] = 42;
var_dump($array);
?>
