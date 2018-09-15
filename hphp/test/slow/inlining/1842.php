<?php

function foo() {
 return $GLOBALS['g'];
 }

<<__EntryPoint>>
function main_1842() {
$g = 0;
$a =& foo();
$a++;
var_dump($a, $g);
}
