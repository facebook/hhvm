<?php

  function run() {
    $a = 5;
    $b =& $a;
    $c =& $b;

    $a = 1;
    $b = 2;
    $c = 3;

    return $a;
  }

var_dump(run());
