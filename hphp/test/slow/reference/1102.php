<?php

function test(&$some_ref) {
 $some_ref = 42;
 }
function bar() {
 return 'test';
 }

<<__EntryPoint>>
function main_1102() {
$p = bar();
$p($some_ref = 1);
var_dump($some_ref);
$p($some_ref = &$q);
var_dump($some_ref,$q);
}
