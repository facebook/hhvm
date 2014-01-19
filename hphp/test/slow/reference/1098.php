<?php

$x = 0;
$foo0 = isset($g) ? "ref" : "val";
$foo1 = isset($g) ? "val" : "ref";
function ref(&$a, $b) {
 echo "$a $b";
 }
function val($a, $b)  {
 echo "$a $b";
 }
$foo0($x, $x = 5);
$foo1($x, $x = 5);
