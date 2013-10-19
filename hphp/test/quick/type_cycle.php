<?php

// A fun test for type inference.
function foo() {
  $a1 = 0;
  $a2 = 0;
  $a3 = 0;
  $a4 = 0;
  $a5 = 0;
  $a6 = 0;
  $a7 = 0;
  $a8 = 0;
  $a9 = 0;

  while (!is_string($a1)) {
    echo "$a1\n";
    echo "$a2\n";
    echo "$a3\n";
    echo "$a4\n";
    echo "$a5\n";
    echo "$a6\n";
    echo "$a7\n";
    echo "$a8\n";
    echo "$a9\n";
    echo "-----------\n";
    $a1 = $a2;
    $a2 = $a3;
    $a3 = $a4;
    $a4 = $a5;
    $a5 = $a6;
    $a6 = $a7;
    $a7 = $a8;
    $a8 = $a9;
    $a9 = "foo";

    $b1 = 0;
    $b2 = 0;
    $b3 = 0;
    while (!is_string($b1)) {
      echo "   $b1\n";
      echo "   $b2\n";
      echo "   $b3\n";
      echo "   ===\n";
      $b1 = $b2;
      $b2 = $b3;
      $b3 = "boo";
    }

    echo "   $b1\n";
    echo "   $b2\n";
    echo "   $b3\n";
    echo "   ===\n";
  }

  if (is_int($a1)) echo "haha\n";
  echo $a1 . "\n";
}

foo();
