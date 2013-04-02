<?php

function breaker(&$x) {
  $x = (string)mt_rand();
}

function foo() {
  $x = "";
  breaker($x);
  // Bug #2240782: HHIR needs to think of $x as a BoxedStr, not a
  // BoxedStaticStr here.
  echo "Num: ";
  echo $x;
  echo "\n";
}

foo();
