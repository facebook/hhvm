<?php

$a = 2;
$b = 3;

$ref =& $b;

$a **= $b;

var_dump($a);

?>
