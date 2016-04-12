<?php

const INT_VAL = 10;
const FLOAT_VAL = 10.5;
const STRING_VAL = "this is a test";
const INT_ADD_VAL = 10 + 15;
const FLOAT_ADD_VAL = 10.5 + 0.2;
const STRING_ADD_VAL = "this" . " is a test";
const NULL_VAL = null;

function int_val(int $a = INT_VAL): int {
    return $a;
}

function float_val(float $a = FLOAT_VAL): float {
    return $a;
}

function string_val(string $a = STRING_VAL): string {
    return $a;
}

function int_add_val(int $a = INT_ADD_VAL): int {
    return $a;
}

function float_add_val(float $a = FLOAT_ADD_VAL): float {
    return $a;
}

function string_add_val(string $a = STRING_ADD_VAL): string {
    return $a;
}

function int_val_default_null(int $a = NULL_VAL) {
    return $a;
}

echo "Testing int val" . PHP_EOL;
var_dump(int_val());

echo "Testing float val" . PHP_EOL;
var_dump(float_val());

echo "Testing string val" . PHP_EOL;
var_dump(string_val());

echo "Testing int add val" . PHP_EOL;
var_dump(int_add_val());

echo "Testing float add val" . PHP_EOL;
var_dump(float_add_val());

echo "Testing string add val" . PHP_EOL;
var_dump(string_add_val());

// These don't work in HHVM, and are unlikely to be fixed. We don't evaluate
// default arguments when we don't need them, PHP 7 evaluates them on all calls
//
//echo "Testing int with default null constant" . PHP_EOL;
//var_dump(int_val_default_null());
//
//echo "Testing int with null null constant" . PHP_EOL;
//var_dump(int_val_default_null(null));

?>
