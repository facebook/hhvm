<?php


<<__EntryPoint>>
function main_array_combine_byref() {
$a = [123, 456];
var_dump($a);
$tmp =& $a[1];
var_dump($a);
$b = array_combine($a, $a);
var_dump($a);
var_dump($b);
}
