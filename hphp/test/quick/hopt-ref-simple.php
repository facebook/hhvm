<?php

function run() {
  $a = 5;
  $b =& $a;

  $b = 3;

  return $a;
}

var_dump(run());
