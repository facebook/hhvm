<?php

$fruits = array(
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
);
arsort($fruits);
var_dump($fruits);


$fruits = array(
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
);
ksort($fruits);
var_dump($fruits);


$fruits = array(
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
);
krsort($fruits);
var_dump($fruits);
