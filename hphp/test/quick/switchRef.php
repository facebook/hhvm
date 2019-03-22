<?php

abstract final class Switchref {
  public static $randBits;
}

/*
 * Check that the translator correctly adapts to callsites with variable
 * argument reffiness.
 */

// 100 pseudo-random bits generated offline.
Switchref::$randBits = array(
0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0,
1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1,
1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1,
0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1);

function byVal($a1, $a2) {
  echo "byVal ";
  return $a1 * $a2;
}

function byRef($a1, &$va2) {
  echo "byRef ";
  $va2 *= $a1;
  return $va2;
}

function main() {
  $funcs = array("byVal", "byRef");
  $b = 2;
  $c = 3;
  foreach(Switchref::$randBits as $idx => $bit) {
    try {
      $funcs[$bit]($b, $c);
    } catch (InvalidArgumentException $e) {
      echo "byRef ";
      $c *= $b;
    }
    echo "$idx: $b, $c\n";
  }
  $b = 2;
  $c = 3;
  foreach(Switchref::$randBits as $idx => $bit) {
    try {
      $funcs[$bit]($b, &$c);
    } catch (InvalidArgumentException $e) {
      echo "byVal ";
    }
    echo "$idx: $b, $c\n";
  }
}

main();

