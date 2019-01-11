<?php

function test(&$some_ref) {
  $some_ref = 42;
}
function test2($some_ref) {
  $some_ref = 42;
}

<<__EntryPoint>>
function main_1091() {
$var = null;
test(&$var);
var_dump($var);
$var = null;
$some_ref = &$var;
test(&$var);
var_dump($some_ref, $var);
test2($some_ref = 1);
var_dump($some_ref);
$var = null;
test2($var);
var_dump($var);
$var = null;
test2($some_ref = $var);
var_dump($some_ref, $var);
$var = null;
test2($some_ref = &$var);
var_dump($some_ref, $var);
}
