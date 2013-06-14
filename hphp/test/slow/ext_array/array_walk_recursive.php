<?php

function test_print($item,$key) {
  echo "$key: $item\n";
}

$sweet = array("a" => "apple", "b" => "banana");
$fruits = array("sweet" => $sweet, "sour" => "lemon");

array_walk_recursive($fruits, "test_print");
