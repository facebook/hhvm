<?php

var_dump(constant());
var_dump(constant("", ""));
var_dump(constant(""));

var_dump(constant(array()));

const TEST_CONST = 1;
var_dump(constant("TEST_CONST"));

const TEST_CONST2 = "test";
var_dump(constant("TEST_CONST2"));

echo "Done\n";
?>
