<?php

function a() {
  $input = array("orange", "banana", "apple", "raspberryu");
  $fruit = array_pop($input);
  var_dump($input);
}

function b() {
  $input = array("orange");
  $fruit = array_pop($input);
  array_push($input, "banana");
  var_dump($input);
  var_dump($fruit);
}

a();
b();

