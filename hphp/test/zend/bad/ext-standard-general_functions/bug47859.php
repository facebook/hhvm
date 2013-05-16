<?php
var_dump(parse_ini_string('*key = "*value"'));
var_dump(parse_ini_string('-key = "-value"'));
var_dump(parse_ini_string('_key = "_value"'));

var_dump(parse_ini_string('key* = "value*"'));
var_dump(parse_ini_string('key.*.* = "value.*.*"'));
var_dump(parse_ini_string('*.*.key = "*.*.value"'));
var_dump(parse_ini_string('k*e*y = "v*a*lue"'));
?>