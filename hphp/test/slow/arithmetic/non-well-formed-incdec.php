<?php


<<__EntryPoint>>
function main_non_well_formed_incdec() {
$x = "100 apples";
++$x;
var_dump($x);
$x++;
var_dump($x);
--$x;
var_dump($x);
$x--;
var_dump($x);
}
