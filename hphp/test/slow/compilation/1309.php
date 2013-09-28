<?php

function foo() {
 return array(1,2,3);
 }
function bar($a, $b, $c) {
 $a = 4;
 }
$x = foo();
bar($x[3][4], $y);
var_dump($x);
