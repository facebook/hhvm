<?php

function run() {
  $a = 5;
  $b =& $a;

  $a = 1;
  $a = true;

  return $a;
}

var_dump(run());
