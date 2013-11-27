<?php

function blah() {
  $xs = array(11, 22, 33, 44, 55);
  $ys = array('a', 'b', 'c', 'd', 'e', 'f');

  foreach ($ys as $y) {
    echo "begin outer loop $y\n";
    try {
      try {
        foreach ($xs as $x) {
          echo "begin inner loop $x\n";
          if ($x == 22 && $y == 'c') {
            echo "break 2\n";
            break 2;
          }
          if ($x == 22) {
            echo "break\n";
            break;
          }
          echo "end inner loop $x\n";
        }
      } finally {
        echo "inner finally\n";
      }
    } finally {
      echo "outer finally\n";
    }
    echo "end outer loop\n";
  }
}

blah();

