<?php

var_dump(json_encode(array("a" => 1, "b" => 2.3, 3 => "test")));
var_dump(json_encode(array("a", 1, true, false, null)));

var_dump(json_encode("a\xE0"));
var_dump(json_encode("a\xE0", JSON_FB_LOOSE));

var_dump(json_encode(array("0" => "apple", "1" => "banana")));

var_dump(json_encode(array(array("a" => "apple"))));

var_dump(json_encode(array(array("a" => "apple")), JSON_PRETTY_PRINT));

var_dump(json_encode(array(1, 2, 3, array(1, 2, 3)), JSON_PRETTY_PRINT));

$arr = array(
  "a" => 1,
  "b" => array(1, 2),
  "c" => array("d" => 42)
);
var_dump(json_encode($arr, JSON_PRETTY_PRINT));
