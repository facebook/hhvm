<?php

function run(&$a, &$b) {
  $b = 2;
  $a = 3;
  return $b;
}

$a = 5;
var_dump(run($a, $a));
var_dump($a);
