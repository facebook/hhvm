<?php

class A extends Exception {}
class B extends Exception {}
class C extends B {}

echo "1\n";
try {
  echo "2\n";
  try {
    echo "3\n";
    throw new Exception("ble!");
    echo "aaa\n";
  } catch (Exception $e) {
    echo "4\n";
  } finally {
    echo "5\n";
  }
  echo "6\n";
} finally {
  echo "7\n";
  try {
    echo "8\n";
    try {
      echo "9\n";
      throw new A("bla!");
      echo "aaa\n";
    } finally {
      echo "10\n";
      throw new C("ble!");
      echo "bbb\n";
    }
    echo "ccc\n";
  } catch (A $ae) {
    die("ddd\n");
  } catch (B $ab) {
    echo "11\n";
  } catch (Exception $e) {
    die("eee\n");
  } finally {
    echo "12\n";
    try {
      echo "13\n";
      try {
        echo "14\n";
      } finally {
        echo "15\n";
      }
      echo "16\n";
    } finally {
      echo "17\n";
    }
    echo "18\n";
  }
  echo "19\n";
}
echo "20\n";

