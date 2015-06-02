<?php

function foo($a) {
  echo __FUNCTION__ . "\n";
}

$x = [1];

$f = 'foo';
$f(&$x[0]);
