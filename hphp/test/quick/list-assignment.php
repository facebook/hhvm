<?php

$array = array(1, 2, 3);

// Easy
list($a, $b, $c) = $array;
var_dump($a, $b, $c);

// Medium
$bucket = array();
list($bucket[0], $bucket[1], $bucket[2]) = $array;
var_dump($bucket);

// Medium 2
function f() {
  echo "f\n";
  return array(4, 5, 6);
}
list($a, $b, $c) = f();
var_dump($a, $b, $c);

// Medium 3
list($a, $b) = array();
var_dump($a, $b);

// Hard
list($a, list(list($b), $c)) = array(1, array(array(2), 3));
var_dump($a, $b, $c);

// WTF
$c = array(1, 2, "derp");
list($a, $b, $c) = $c;
var_dump($a, $b, $c);
