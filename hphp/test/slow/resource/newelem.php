<?php

error_reporting(-1);

function lol() { return STDIN; }
function foo() {
  $x = lol();
  $y = ($x[0] = 2);
  var_dump($x);
  var_dump($y);
  $y = ($x[][] = 2);
  var_dump($x);
  var_dump($y);
}

foo();
