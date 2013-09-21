<?php

var_dump(constant());
var_dump(constant("", ""));
var_dump(constant(""));

var_dump(constant(array()));

define("TEST_CONST", 1);
var_dump(constant("TEST_CONST"));

define("TEST_CONST2", "test");
var_dump(constant("TEST_CONST2"));

echo "Done\n";
?>