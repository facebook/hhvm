<?php

$array1 = array(
  "a" => "green",
  "red",
  "blue",
  "red"
);
$array2 = array(
  "b" => "green",
  "yellow",
  "red"
);

$result = array_diff($array1, $array2);
var_dump($result);

$a = array("b");
$b = array("b", "c");
var_dump(array_diff($b, $a));
