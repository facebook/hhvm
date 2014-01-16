<?php
function foo() {
  foreach (array(1,2,3) as $x) {
    try {
      echo "A\n";
      throw new Exception("c");
    } finally {
      echo "B\n";
      foreach (array(1,2,3) as $y) { var_dump($y); }
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
