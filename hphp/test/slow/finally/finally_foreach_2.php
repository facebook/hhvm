<?php
function foo() {
  foreach (array(1,2,3) as $v) {
    foreach (array(1,2,3) as $w) {
      try {
        echo "A\n";
        throw new Exception("c");
      } finally {
        echo "B\n";
        foreach (array(1,2,3) as $x) {
          foreach (array(1,2,3) as $y) {
            try {
              echo "C\n";
              throw new Exception("d");
            } finally {
              echo "D\n";
              foreach (array(1,2,3) as $z) { var_dump($z); }
            }
          }
        }
      }
    }
  }
}
function main() {
  try {
    foo();
  } catch (Exception $e) {
    echo "Caught exception\n";
  }
}
main();
