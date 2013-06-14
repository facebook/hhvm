<?php

function a() {
  $input = array(
    "a" => "green",
    "red",
    "b" => "green",
    "blue",
    "red"
  );
  $result = array_unique($input);
  var_dump($result);
}

function b() {
  $input = array(
    4,
    "4",
    "3",
    4,
    3
  );
  $result = array_unique($input);
  var_dump($result);
}

function c() {
  $input = array(
    "a" => "A",
    "b" => "C",
    0 => "1",
    2 => "01",
    1 => 1,
    "c" => "C"
  );
  var_dump(array_unique($input, SORT_STRING));
  var_dump(array_unique($input, SORT_NUMERIC));
  var_dump(array_unique($input, SORT_REGULAR));
}

function d() {
  $input = array(
    1 => 1,
    'a' => 'A',
    'b' => 'C',
    0 => '1',
    2 => '01',
    'c' => 'C'
  );
  var_dump(array_unique($input, SORT_REGULAR));
}

a();
b();
c();
d();
