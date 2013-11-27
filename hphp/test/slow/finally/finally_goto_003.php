<?php

function blah() {

  $xs = array(1, 2, 3, 4);
  $ys = array(1, 2, 3, 4);

  before:
  echo "before label\n";
  foreach ($xs as $x) {
    echo "outer loop\n";
    try {
      echo "outer try\n";
      try {
        echo "inner try\n";
        foreach ($ys as $y) {
          echo "inner loop\n";
          if ($x == 4 && $y == 3) {
            echo "before goto after\n";
            goto after;
            echo "after goto after\n";
          }
          if (empty($flag) && $x == 3 && $y == 2) {
            echo "before goto before\n";
            $flag = 1;
            goto before;
            echo "after goto before\n";
          }
          echo "inner loop end\n";
        }
        echo "inner try end\n";
      } finally {
        echo "inner finally\n";
      }
      echo "inner finally\n";
    } catch (Exception $x) {
      echo "outer catch\n";
    } finally {
      echo "outer finally\n";
    }
    echo "outer loop end\n";
  }
  after:
  echo "after label\n";
}

blah();
