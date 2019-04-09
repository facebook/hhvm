<?php

class C {
  public $max = PHP_INT_MAX;
}

function main() {
  $add = function($a,$b) { return $a + $b; };
  $sub = function($a,$b) { return $a - $b; };
  $mul = function($a,$b) { return $a * $b; };

  $max = PHP_INT_MAX;
  $min = 1 << 63;

  // some initial cases for the simplifier
  var_dump($max + 1);
  var_dump($min - 1);
  var_dump($max * 5);

  $ops = array(
    // initial sanity checks for no overflow
    array($add, 3, 5),
    array($sub, 3, 5),
    array($mul, 7, 4),

    // check runtime operators on just ints
    array($add, $max, 1),
    array($add, $min, -1),

    array($sub, $min, 1),
    array($sub, $max, -1),

    array($mul, $max / 2, 3),
    array($mul, $min, 2),
    array($mul, $max, -2),
    array($mul, $min, -3),

    // check numeric strings
    array($add, "$max", 1),
    array($add, $max, '1'),
    array($add, "$max", '1'),

    // check lexer
    array($add, 987654321987654321987654321, 1),
  );

  foreach ($ops as list($op, $lhs, $rhs)) {
    $res = $op($lhs, $rhs);
    var_dump($res);
  }

  $unary = array($min, $max, -4, 0, 5, "12", 5.2, "1.5", "abc", "", null);

  // inc/dec
  foreach ($unary as $val) {
    $x = $val;
    var_dump(++$x);
    var_dump($x);

    $x = $val;
    var_dump($x++);
    var_dump($x);

    $x = $val;
    var_dump(--$x);
    var_dump($x);

    $x = $val;
    var_dump($x--);
    var_dump($x);
  }

  // arrays
  $array = $unary;
  for ($i = 0; $i < count($array); ++$i) {
    var_dump($array[$i]);
    var_dump($array[$i]++);
    var_dump($array[$i]);
  }

  $array = $unary;
  for ($i = 0; $i < count($array); ++$i) {
    var_dump($array[$i]);
    var_dump(++$array[$i]);
    var_dump($array[$i]);
  }

  $array = $unary;
  for ($i = 0; $i < count($array); ++$i) {
    var_dump($array[$i]);
    var_dump($array[$i]--);
    var_dump($array[$i]);
  }

  $array = $unary;
  for ($i = 0; $i < count($array); ++$i) {
    var_dump($array[$i]);
    var_dump(--$array[$i]);
    var_dump($array[$i]);
  }

  // properties

  $c = new C;
  var_dump($c->max);
  var_dump($c->max++);
  var_dump($c->max);

  $c = new C;
  var_dump($c->max);
  var_dump(++$c->max);
  var_dump($c->max);

  $c = new C;
  var_dump($c->max);
  var_dump($c->max--);
  var_dump($c->max);

  $c = new C;
  var_dump($c->max);
  var_dump(--$c->max);
  var_dump($c->max);
}

main();
