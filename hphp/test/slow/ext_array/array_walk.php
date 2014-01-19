<?php

function test_print($item,$key) {
  echo "$key: $item\n";
}

function test_alter(&$item,$key,$prefix) {
  $item = $prefix.': '.$item;
}

$fruits = array("d" => "lemon",
                "a" => "orange",
                "b" => "banana",
                "c" => "apple");

array_walk($fruits, "test_print");
array_walk($fruits, "test_alter", "fruit");
array_walk($fruits, "test_print");
