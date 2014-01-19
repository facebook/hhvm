<?php

var_dump(min(array(3, 1, 6, 7)));
var_dump(min(array(2, 4, 5)));
var_dump(min(0, array("hello")));
var_dump(min("hello", array(0)));
var_dump(min("hello", array(-1)));
var_dump(min(array(2, 4, 8), array(array(2, 5, 1))));
var_dump(min("string", array(array(2, 5, 7), 42)));
var_dump(min(array(1 => "1236150163")));
