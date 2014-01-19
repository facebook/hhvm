<?php

error_reporting(-1);

if (1) {
  function g(&$x) {
    var_dump($x);
    $x = 123;
  }
}

// doesn't fatal, raises strict standards warning

$x = null;
g(eval('return $x;'));
var_dump($x);
