<?php


<<__EntryPoint>>
function main_1424() {
$a = "123.456" - 123;
var_dump($a);
$a = "123.456" - 456.123;
var_dump($a);
$a = "123.456" - "123";
var_dump($a);
$a = "123.456" - "456.123";
var_dump($a);
$a = "123.456";
$a -= 123;
var_dump($a);
$a = "123.456";
$a -= 456.123;
var_dump($a);
$a = "123.456";
$a -= "123";
var_dump($a);
$a = "123.456";
$a -= "456.123";
var_dump($a);
}
