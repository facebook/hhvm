<?php

function a() {
  $input = array("a" => 1, "b" => 2);
  array_shift($input);
  var_dump($input);
}

function b() {
  $input = array("a" => 1, 23 => 2);
  array_shift($input);
  var_dump($input);
}

function c() {
  $input = array("a" => 1, -23 => 2);
  array_shift($input);
  var_dump($input);
}

function d() {
  $input = array("orange", "banana", "apple", "raspberry");
  $fruit = array_shift($input);
  var_dump($input);
  var_dump($fruit);
}

a();
b();
c();
d();
