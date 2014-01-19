<?php

class A extends Exception {}
class B extends Exception {}
class C extends B {}

echo "1\n";
try {
  echo "2\n";
  try {
    echo "3\n";
  } finally {
    echo "4\n";
  }
  echo "5\n";
  try {
    echo "6\n";
    try {
      echo "7\n";
      try {
        echo "8\n";
        throw new C("ble!");
        echo "aaa\n";
      } catch (C $e) {
        echo "9\n";
      }
      echo "10\n";
      try {
        echo "11\n";
        return 25;
      } finally {
        echo "12\n";
        try {
          echo "13\n";
        } catch (C $e) {
          echo "ccc\n";
        } finally {
          echo "14\n";
          try {
            echo "15\n";
            try {
              echo "16\n";
            } finally {
              echo "17\n";
            }
            echo "18\n";
          } finally {
            echo "19\n";
          }
          echo "20\n";
        }
        echo "21\n";
      }
      echo "bbb\n";
    } finally {
      echo "22\n";
    }
    echo "ccc\n";
  } finally {
    echo "23\n";
  }
  echo "ddd\n";
} catch (A $e) {
  echo "eee\n";
} finally {
  echo "24\n";
}
echo "fff\n";

