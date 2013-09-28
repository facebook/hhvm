<?php

$a = array(1, 'hello', 3.5);
$b = $a;
$b[4] = 'world';
var_dump($a);
var_dump($b);
$b = 3;
var_dump($a);
var_dump($b);
