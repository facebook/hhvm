<?php

<<__EntryPoint>>
function main_array_slice() {
$a = array(1, 2, 3);
var_dump(array_slice($a, 0, PHP_INT_MAX));
}
