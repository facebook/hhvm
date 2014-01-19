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
      throw new Exception("ble!");
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
      die("ddd\n");
    } finally {
      echo "9\n";
      try {
        echo "10\n";
        try {
          echo "11\n";
          throw new B("14");
          echo "fff\n";
        } finally {
          echo "12\n";
        }
        echo "ggg\n";
      } finally {
        echo "13\n";
      }
      echo "hhh\n";
    }
    echo "iii\n";
  }
  echo "jjj\n";
  return 666;
}

var_dump(blah());

