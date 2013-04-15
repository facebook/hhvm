<?php

function foo($x) {
  $x = $x + 1;
  return $x;
}

function main() {
  $y = foo(1);
  return $y;
}

var_dump(main());
