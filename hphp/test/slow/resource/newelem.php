<?php

error_reporting(-1);

function lol() { return STDIN; }
function foo() {
  $x = lol();
  $y = ($x[0] = 2);
  var_dump($x);
  var_dump($y);
  $x[] = 2;
  $y = 2;
  var_dump($x);
  var_dump($y);
}

foo();
