<?php

var_dump(max(array(3, 1, 6, 7)));
var_dump(max(array(2, 4, 5)));
var_dump(max(0, array("hello")));
var_dump(max("hello", array(0)));
var_dump(max("hello", array(-1)));
var_dump(max(array(2, 4, 8), array(array(2, 5, 1))));
var_dump(max("string", array(array(2, 5, 7), 42)));
var_dump(max(array(1 => "1236150163")));
