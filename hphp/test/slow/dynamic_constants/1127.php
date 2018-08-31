<?php

function foo($a) {
 return $a + 10;
}

 <<__EntryPoint>>
function main_1127() {
define('TEST', foo(10));
 var_dump(TEST);
}
