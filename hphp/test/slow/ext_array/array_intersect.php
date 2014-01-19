<?php

$array1 = array(
  "a" => "green",
  "red",
  "blue"
);
$array2 = array(
  "b" => "green",
  "yellow",
  "red"
);
var_dump(array_intersect($array1, $array2));
