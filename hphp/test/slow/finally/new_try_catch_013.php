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
        } finally {
          echo "12\n";
          try {
            echo "13\n";
            throw new A("blu!");
            echo "bbb\n";
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
          echo "ddd\n";
        }
        echo "eee\n";
      } finally {
        echo "21\n";
      }
      echo "fff\n";
    } finally {
      echo "22\n";
    }
    echo "ggg\n";
  } catch (A $e) {
    echo "23\n";
  } finally {
    echo "24\n";
  }
  echo "25\n";
  return 26;
}



<<__EntryPoint>>
function main_new_try_catch_013() {
var_dump(blah());
}
