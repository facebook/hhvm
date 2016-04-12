<?php

$n=0;
$a=array();

$foo = function($i) use (&$n) {
    echo "foo($i): n = $n\n";
    return 0;
};

$bar = function($i) use (&$n) {
    echo "bar($i): n = $n\n";
    return 0;
};

list(
  $a[$n++ + $foo(0)],
  list(
    $a[$n++ + $foo(10)],
    $a[$n++ + $foo(20)],
  ),
  $a[$n++ + $foo(2)]
) = array(
  "S0: n = " . ($n++ + $bar(0)),
  array(
    "T0: n = " . ($n++ + $bar(10)),
    "T1: n = " . ($n++ + $bar(20)),
  ),
  "S2: n = " . ($n++ + $bar(2))
);

var_dump($a);
