<?php

function run() {
  $a = 5;
  $b =& $a;
  $c =& $b;

  $a = 1;
  $b = true;
  $c = 3;

  return $a;
}

var_dump(run());
