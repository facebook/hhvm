<?php

function test_print($item,$key) {
  echo "$key: $item\n";
}


<<__EntryPoint>>
function main_array_walk_recursive() {
$sweet = array("a" => "apple", "b" => "banana");
$fruits = array("sweet" => $sweet, "sour" => "lemon");

array_walk_recursive($fruits, "test_print");
}
