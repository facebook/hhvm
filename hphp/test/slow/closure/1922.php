<?php


<<__EntryPoint>>
function main_1922() {
$x = false;
$f = function ($arg0) use (&$x) {
 $x = $arg0;
 }
;
$f(32);
var_dump($x);
}
