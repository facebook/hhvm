<?php

function run(&$a) {
  $b =& $a;

  $b = 3;
  return $a;
}

$a = 5;
run($a);
var_dump($a);
