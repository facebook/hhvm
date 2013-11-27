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
          return 666;
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
                throw new A("bli!");
                echo "ddd\n";
              } finally {
                echo "17\n";
              }
              echo "eee\n";
            } finally {
              echo "18\n";
            }
            echo "eee\n";
          }
          echo "fff\n";
        }
        echo "ggg\n";
      } finally {
        echo "19\n";
      }
      echo "hhh\n";
    } finally {
      echo "20\n";
    }
    echo "iii\n";
  } catch (A $e) {
    echo "21\n";
  } finally {
    echo "22\n";
  }
  echo "23\n";
  return 24;
}

var_dump(blah());

