<?php

$s = " ";
$a = "hello";
$a .= $s;
$a .= "world";
var_dump($a);
$a = "a";
$a .= "b";
$a .= $a;
var_dump($a);
$a = 3;
echo 0 + "1$a";
