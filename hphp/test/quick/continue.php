<?php

function bare_break_continue() {
  // This should emit properly; it's not an include-time fatal
  break;
  continue;
}

function test() {
  $three = array(1, 2, 3);
  $four  = array(1, 2, 3, 4);

  foreach ($three as $x) {
    if ($x == 2) {
      continue;
    }
    echo $x;
  }
  echo "\n";

  foreach ($three as $x) {
    foreach ($four as $y) {
      if ($y == 3) {
        continue 2;
      }
      echo $y;
    }
  }
  echo "\n";

  foreach ($three as $x) {
    foreach ($four as $y) {
      if ($y == 3) {
        continue (100 - 98);
      }
      echo $y;
    }
  }
  echo "\n";
}

test();
