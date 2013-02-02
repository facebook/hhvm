<?php

function run($a) {
  $b = 5;
  $c =& $b;

  $d = $a;

  return $c;
}

$a = "abc";
$a .= 1;
$a .= "\n";
var_dump(run($a));
