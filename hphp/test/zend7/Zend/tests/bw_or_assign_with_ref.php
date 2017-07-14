<?php

$num1 = 1;
$num2 = '2';
$ref =& $num2;
$num1 |= $num2;
var_dump($num1);

?>
