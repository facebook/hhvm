<?php

class A extends Exception {}
class B extends Exception {}
class C extends B {}

function ble() {
  echo "14\n";
  try {
    echo "15\n";
    throw new C("ble!");
    echo "xxx\n";
  } catch (B $e) {
    echo "16\n";
  } finally {
    echo "17\n";
  }
  echo "18\n";
}

function blah() {
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
        throw new C("24");
        echo "bbb\n";
      }
      echo "ccc\n";
    } catch (A $ae) {
      die("ddd\n");
    } finally {
      echo "11\n";
      try {
        echo "12\n";
        try {
          echo "13\n";
          ble();
          echo "19\n";
        } finally {
          echo "20\n";
        }
        echo "21\n";
      } catch (B $e) {
        echo "yyy\n";
      } finally {
        echo "22\n";
      }
      echo "23\n";
    }
    echo "aaa\n";
  }
  echo "bbb\n";
  return 666;
}

var_dump(blah());

