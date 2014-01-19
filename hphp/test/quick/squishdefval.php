<?php

function foo() {
  return 1 + 2;
}

function bar($x = 10) {
  return $x + (1 + 2);
}

echo bar() . "\n";

