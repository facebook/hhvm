<?php

$y = 10;
$x = array(
  42,
  $y,
  &$y,
  "var" => &$y,
  "val" => $y,
  array(
    "what" => "arrays",
    "when" => "now",
  )
);
$y = 0;

var_dump($x);

var_dump([2, 3, 4]);

var_dump(["foo" . "x" => 42]);
