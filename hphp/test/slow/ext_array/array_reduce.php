<?php

function rsum($s1,$s2) {
  return (int)$s1 + (int)$s2;
}
function rmul($s1,$s2) {
  return (int)$s1 * (int)$s2;
}

$a = array(1, 2, 3, 4, 5);
$b = array_reduce($a, "rsum");
var_dump($b);
$c = array_reduce($a, "rmul", 10);
var_dump($c);
$d = array_reduce($a, "rmul");
var_dump($d);

$x = array();
$e = array_reduce($x, "rsum", 1);
var_dump($e);

