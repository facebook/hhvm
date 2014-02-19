<?php

function blah() {
  $xs = array(1, 2, 3);
  $ys = array(1, 2, 3);
  $zs = array(1, 2, 3);
  try {
    foreach ($xs as $x) {
      foreach ($ys as $y) {
        foreach ($zs as $z) {
          echo "$x\n";
          echo "$y\n";
          echo "$z\n";
          if ($x == 2 && $y == 2 && $z == 2) {
            echo "4\n";
            throw new Exception("blah!");
            echo "aaa\n";
          }
        }
      }
    }
  } catch (Exception $e) {
    echo empty($x) ? "bbb\n" : "5\n";
    echo empty($y) ? "ccc\n" : "6\n";
    echo empty($z) ? "ddd\n" : "7\n";
    echo "8\n";
  }
  return 9;
}

var_dump(blah());

