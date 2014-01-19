<?php

$a = array('a'=>0);
$ref = &$a['a'];
var_dump($a);
$b = $a;
var_dump($a,$b);
$b['a'] = 1;
var_dump($a,$b);
$a = array(0);
$ref = &$a[0];
var_dump($a);
$b = $a;
var_dump($a,$b);
$b[0] = 1;
var_dump($a,$b);
