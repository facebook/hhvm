<?php

function blah() {
  $xs = array(1, 2, 3, 4, 5);

  try {
    foreach ($xs as $x) {
      echo "$x\n";
      if ($x == 2) {
        break;
      }
    }
  } finally {
    echo "3\n";
  }
}

blah();

