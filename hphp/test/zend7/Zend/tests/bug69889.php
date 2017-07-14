<?php

$foo = "test";
var_dump($foo[0] ?? "default");

var_dump($foo[5] ?? "default");
var_dump(isset($foo[5]) ? $foo[5] : "default");

var_dump($foo["str"] ?? "default");
var_dump(isset($foo["str"]) ? $foo["str"] : "default");

?>
