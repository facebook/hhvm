<?php

$x = 17;
for (;;) {
  echo $x . "\n";
  if ($x === 1) {
    break;
  }

  if ($x % 2 === 0) {
    $x /= 2;
  } else {
    $x = $x * 3 + 1;
  }
}
