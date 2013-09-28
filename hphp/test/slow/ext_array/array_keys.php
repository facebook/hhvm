<?php
var_dump(array_keys(array(0 => 100, "color" => "red")));
var_dump(array_keys(array("blue", "red", "green", "blue", "blue"),
                    "blue"));
var_dump(array_keys(
  array("color" => array("blue", "red", "green"),
        "size" => array("small", "medium", "large"))
));

$array = array(
  "a" => null,
  "b" => 123,
  "c" => false,
);
var_dump(array_keys($array));
var_dump(array_keys($array, null));
