<?php
// When we are out of callee-saved registers, we should still do
// pre-coloring opt instead of picking a random callee-saved register.
function foo($t0, $t1, $t2, $t3) {
  echo $t0;
  echo $t2;
  echo $t3;
  echo $t1;
  echo "\n";
}

foo(1, 2, 3, 4);
