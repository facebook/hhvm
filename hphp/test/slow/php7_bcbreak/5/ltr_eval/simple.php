<?php

error_reporting(0);

list($a, $b) = [1, 2];
var_dump($a);
var_dump($b);

$array = [];
list($array[], $array[], $array[]) = [1, 2, 3];
var_dump($array);

$a = [1, 2];
list($a, $b) = $a;
var_dump($a);
var_dump($b);

$b = [1, 2];
list($a, $b) = $b;
var_dump($a);
var_dump($b);

$e = array(0,0);
$f = 0;
$g1 = array(10,11);
$g2 = array(20,21);
$g3 = array(30,31);
$g = array($g1,$g2,$g3);
list($e[$f++],$e[$f++]) = $g[$f];
var_dump($e);

$h = array(1, 2, 3);
$i = 0;
$j[$i++] = $h[$i++];
var_dump($j);
