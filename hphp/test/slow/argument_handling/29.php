<?php

function f($x) {
 $b = $x;
 $b++;
 }
$a = 1;
f(&$a);
var_dump($a);
