<?php

$a = array(0);
$b = array(1);
$b[0] = &$a[0];
$a[0] = "hi";
var_dump($a);
var_dump($b);
