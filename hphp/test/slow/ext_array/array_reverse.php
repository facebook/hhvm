<?php

function a() {
  $input = array("php", 4.0, array("green", "red"));
  $result = array_reverse($input);
  var_dump($result);
  $result_keyed = array_reverse($input, true);
  var_dump($result_keyed);
}

function b() {
  $input = array("php" => 4.0, 10 => 5.0, "blab" =>"b");
  $result = array_reverse($input);
  var_dump($result);
}

a();
b();

