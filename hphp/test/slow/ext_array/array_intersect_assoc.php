<?php

$array1 = array(
  "a" => "green",
  "b" => "brown",
  "c" => "blue",
  "red"
);
$array2 = array(
  "a" => "green",
  "yellow",
  "red"
);
var_dump(array_intersect_assoc($array1, $array2));
