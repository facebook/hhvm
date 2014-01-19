<?php

function blah() {

  $xs = array(1, 2, 3, 4);
  $ys = array(1, 2, 3, 4);

  foreach ($xs as $x) {
    echo "outer loop\n";
    label4:
    try {
      echo "try $x\n";
      foreach ($ys as $y) {
        if ($x == 3 && $y == 3) {
          ++$x;
          echo "before goto\n";
          goto label4;
          echo "after goto\n";
        }
        if ($x == 4 && $y == 2) {
          echo "before goto 2\n";
          goto label2;
          echo "after goto 2\n";
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

  echo "before label\n";
  label:
  echo "after label\n";
}

blah();
