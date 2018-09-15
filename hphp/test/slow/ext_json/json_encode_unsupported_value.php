<?php

// This file is ran with -vEval.EnableHipHopSyntax=0

// -----------------------------------------
// Closures
// -----------------------------------------

<<__EntryPoint>>
function main_json_encode_unsupported_value() {
var_dump(json_encode(function() {}));
var_dump(json_encode([1, 2, function() {}]));

// With partial output on errors.
var_dump(json_encode(function() {}, JSON_PARTIAL_OUTPUT_ON_ERROR));
var_dump(json_encode([1, 2, function() {}], JSON_PARTIAL_OUTPUT_ON_ERROR));
}
