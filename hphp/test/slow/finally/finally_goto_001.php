<?php

function blah() {

  $xs = array(1, 2, 3, 4);
  $ys = array(1, 2, 3, 4);

  foreach ($xs as $x) {
    echo "outer loop\n";
    try {
      echo "try\n";
      foreach ($ys as $y) {
        if ($x == 3 && $y == 3) {
          echo "before goto\n";
          goto label;
          echo "after goto\n";
        }
        label3:
        echo "inside inner loop\n";
      }
    } finally {
      echo "finally\n";
    }
    label2:
    echo "inside outer loop\n";
  }

  echo "before if\n";
  if (2 == 3) {
    echo "before label\n";
    label:
    echo "after label\n";
  }
  echo "after if\n";
}

blah();
