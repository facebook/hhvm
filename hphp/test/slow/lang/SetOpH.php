<?php

$lefts = array(
  0,
  123,
  true,
  123.456,
  "non-numeric",
  "789",
  null,
);
$rights = $lefts;

foreach ($lefts as $left) {
  foreach ($rights as $right) {
    var_dump($left, $right);

    echo "  + ";
    $a = $left;
    var_dump($a += $right);
    var_dump($a);

    echo "  - ";
    $a = $left;
    var_dump($a -= $right);
    var_dump($a);

    echo "  * ";
    $a = $left;
    var_dump($a *= $right);
    var_dump($a);

    echo "  . ";
    $a = $left;
    var_dump($a .= $right);
    var_dump($a);

    echo "  / ";
    $a = $left;
    var_dump($a /= $right);
    var_dump($a);

    echo "  % ";
    $a = $left;
    var_dump($a %= $right);
    var_dump($a);

    echo "  & ";
    $a = $left;
    var_dump($a &= $right);
    var_dump($a);

    echo "  | ";
    $a = $left;
    var_dump($a |= $right);
    var_dump($a);

    echo "  ^ ";
    $a = $left;
    var_dump($a ^= $right);
    var_dump($a);

    echo "  << ";
    $a = $left;
    var_dump($a <<= $right);
    var_dump($a);

    echo "  >> ";
    $a = $left;
    var_dump($a >>= $right);
    var_dump($a);
  }
}
