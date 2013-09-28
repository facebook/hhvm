<?php

function foo() {
 return $GLOBALS['g'];
 }
$g = 0;
$a =& foo();
$a++;
var_dump($a, $g);
