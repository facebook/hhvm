<?php

class A extends Exception {}
class B extends Exception {}
class C extends B {}

function blah() {
  echo "1\n";
  try {
    echo "2\n";
    try {
      echo "3\n";
      throw new Exception("18");
      echo "aaa\n";
    } finally {
      echo "4\n";
    }
    echo "bbb\n";
  } finally {
    echo "5\n";
    try {
      echo "6\n";
      try {
        echo "7\n";
        throw new A("bla!");
        echo "aaa\n";
      } finally {
        echo "8\n";
        throw new C("ble!");
        echo "bbb\n";
      }
      echo "ccc\n";
    } catch (A $ae) {
      echo("ddd\n");
    } catch (B $ab) {
      echo "9\n";
    } catch (Exception $e) {
      echo("eee\n");
    } finally {
      echo "10\n";
      try {
        echo "11\n";
        try {
          echo "12\n";
        } finally {
          echo "13\n";
        }
        echo "14\n";
      } finally {
        echo "15\n";
      }
      echo "16\n";
    }
    echo "17\n";
  }
  echo "fff\n";
  return 666;
}

var_dump(blah());

