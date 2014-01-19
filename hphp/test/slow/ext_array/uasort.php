<?php

function reverse_strcasecmp($s1,$s2) {
  return strcasecmp($s2,$s1);
}

$fruits = array(
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
);
uasort($fruits, 'reverse_strcasecmp');
var_dump($fruits);

$fruits = array(
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
);
uksort($fruits, 'reverse_strcasecmp');
var_dump($fruits);

uasort($fruits, "undefined_function_");
uksort($fruits, "undefined_function_");
