<?php

function foo() {
 var_dump('hello');
 return 789;
}

<<__EntryPoint>>
function main_1908() {
$a = 123 ?: 456;
var_dump($a);
$b[123] = 456;
var_dump(isset($b[123]) ?: false);
var_dump(foo()?:123);
}
