<?php

function test(&$some_ref) {
  $some_ref = 42;
}
test($some_ref = 1);
var_dump($some_ref);
$var = null;
test($var);
var_dump($var);
$var = null;
test($some_ref = $var);
var_dump($some_ref, $var);
$var = null;
test($some_ref = &$var);
var_dump($some_ref, $var);
function test2($some_ref) {
  $some_ref = 42;
}
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
