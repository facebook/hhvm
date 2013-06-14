<?php
function cube($s1) { return $s1*$s1*$s1; }
$a = array(1, 2, 3, 4, 5);
$b = array_map("cube", $a);
var_dump($b);
$b = array_map(null, $a);
var_dump($b);
$b = array_map(null, array('x' => 6, 0 => 7));
var_dump($b);
var_dump(
  array_map(
    null,
    array('x' => 6, 0 => 7),
    array(array('a', 'b'), true)
  )
);

