<?php

$value = 'one';
$array = array($value);
$array = $ref =& $array;
var_dump($array);

?>
