<?php

function &foo() {
  global $x;
  return $x;
}

function unintuitive(&$ref) {
  global $x;
  $x = &$ref;
  return $x;
}

$y = &foo();
$y = 1;
var_dump($y);

$z = &foo();
$z = 2;
var_dump($z);
var_dump($y);

$new = 20;
var_dump(unintuitive($new));
var_dump($x);
