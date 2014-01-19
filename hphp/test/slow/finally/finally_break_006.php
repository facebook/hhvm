<?php

function blah() {
  $xs = array(11, 22, 33, 44, 55);
  $ys = array('a', 'b', 'c', 'd', 'e', 'f');

  try {
    echo "try\n";
  } finally {
    echo "begin final finally\n";
    foreach ($ys as $y) {
      echo "begin outer loop $y\n";
      try {
        try {
          foreach ($xs as $x) {
            echo "begin inner loop $x\n";
            if ($x == 22) {
              echo "continue\n";
              continue;
            }
            echo "end inner loop $x\n";
          }
        } finally {
          echo "inner finally\n";
        }
      } finally {
        echo "outer finally\n";
      }
      echo "middle outer loop\n";
      try {
        try {
          foreach ($xs as $x) {
            echo "begin inner loop $x\n";
            if ($x == 22 && $y == 'b') {
              echo "continue 2\n";
              continue 2;
            }
            if ($x == 33) {
              echo "break\n";
              break;
            }
            echo "end inner loop $x\n";
          }
        } finally {
          echo "inner finally 2\n";
        }
      } finally {
        echo "outer finally 2\n";
      }
      echo "end outer loop\n";
    }
    echo "final finally\n";
  }
}

blah();

