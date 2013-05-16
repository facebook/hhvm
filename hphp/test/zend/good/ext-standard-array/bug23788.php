<?php
$numeric = 123;
$bool = true;
$foo = array(&$numeric, &$bool);
var_dump($foo);
str_replace("abc", "def", $foo);
var_dump($foo);
?>