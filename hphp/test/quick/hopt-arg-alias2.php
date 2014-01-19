<?php

function run(&$a, &$b) {
  $b = 2;
  $a = 3;
  return $b;
}

$a = 5;
$b = 4;
var_dump(run($a, $b));
var_dump($a);
var_dump($b);
