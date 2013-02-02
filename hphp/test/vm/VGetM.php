<?php

$a = array(0, 1, 2);
$ref0 = &$a[0];
$ref0 = "hi";
$ref1 = &$a[1];
var_dump($a);
