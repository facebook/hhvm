<?php

$input = array("a", "b", "c", "d", "e");

var_dump(array_slice($input, 2));
var_dump(array_slice($input, 2, null));
var_dump(array_slice($input, -2, 1));
var_dump(array_slice($input, 0, 3));

// note the differences in the array keys
var_dump(array_slice($input, 2, -1));
var_dump(array_slice($input, 2, -1, true));

var_dump(array_slice(array("a", "b", "c"), 1, 2, true));
var_dump(array_slice(array("a", "b", "c"), 1, 2, false));
$a = array("a" => "g", 0 => "a", 1 => "b", 2 => "c");
unset($a['a']);
var_dump(array_slice($a, 1, 2, true));
var_dump(array_slice($a, 1, 2, false));

$a = array("a" => 123, 0 => "a", 1 => "b", 2 => "c");
unset($a['a']);
var_dump(array_slice($a, 1, 2, true));
var_dump(array_slice($a, 1, 2, false));

var_dump(array_slice(array(123, "b", "c"), 1, 2, true));

var_dump(array_slice(array(123, "b", "c"), 1, 2, false));
