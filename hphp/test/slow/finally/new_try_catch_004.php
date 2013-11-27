<?php

class A extends Exception {}
class B extends Exception {}
class C extends B {}

function blah() {
  echo "1\n";
  try {
    echo "2\n";
  } finally {
    echo "3\n";
    try {
      echo "4\n";
      try {
        echo "5\n";
        throw new A("5");
        echo "aaa\n";
      } finally {
        echo "6\n";
        throw new C("6");
        echo "bbb\n";
      }
    } catch (A $ae) {
      echo("ccc\n");
    } catch (B $ab) {
      echo "7\n";
    } catch (Exception $e) {
      echo("ddd\n");
    } finally {
      echo "8\n";
    }
    echo "9\n";
  }
  echo "10\n";
  return 11;
}

var_dump(blah());

