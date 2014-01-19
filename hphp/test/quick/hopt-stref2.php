<?php

  function run() {
    $a = 5;
    $b =& $a;
    $c =& $b;

    $a = "hello";
    $b = 2;
    $c = array();

    return $a;
  }

var_dump(run());
