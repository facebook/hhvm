<?php

function foo() {
 return 42;
 }

<<__EntryPoint>>
function main_1117() {
$a = foo();
var_dump((unset)foo());
var_dump((unset)$a);
var_dump($a);
}
