<?php
$params = array("a" => "aaa", "0" => "apple");
unset($params['a']);
array_splice($params, 0, 0, array(123 => "test"));
var_dump($params);

$params = array("a" => "aaa", "1" => "apple");
unset($params['a']);
array_splice($params, 0, 0, array(123 => "test"));
var_dump($params);

$input = array("red", "green", "blue", "yellow");
array_splice($input, 2);
var_dump($input);

$input = array("red", "green", "blue", "yellow");
array_splice($input, 2, null);
var_dump($input, array("red", "green"));

$input = array("red", "green", "blue", "yellow");
array_splice($input, 1, -1);
var_dump($input, array("red", "yellow"));

$input = array("red", "green", "blue", "yellow");
array_splice($input, 1, 4, "orange");
var_dump($input);

$input = array("red", "green", "blue", "yellow");
array_splice($input, -1, 1, array("black", "maroon"));
var_dump($input);

$input = array("red", "green", "blue", "yellow");
array_splice($input, 3, 0, "purple");
var_dump($input);
