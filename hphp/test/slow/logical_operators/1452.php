<?php

function foo($a) {
 echo "foo";
 return $a;
 }
$x = true;
$x = $x and foo(false);
var_dump($x);
$x = $x && foo(false);
var_dump($x);
$x = false;
$x = $x or foo(true);
var_dump($x);
$x = $x || foo(true);
var_dump($x);
