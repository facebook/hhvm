<?php

// Check MixedArray::MakeReserveLike on NVT (GitHub #3065)

$x = 0;

function check_global($arg) {
  global $x;
  $x = 1;
  return false;
}

array_map('check_global', $GLOBALS);
print "$x\n";
