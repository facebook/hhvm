<?php
/* Prototype  : string gzencode  ( string $data  [, int $level  [, int $encoding_mode  ]] )
 * Description: Gzip-compress a string 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

/*
 * Test error cases for gzencode
 */

echo "*** Testing gzencode() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing gzencode() function with Zero arguments --\n";
var_dump( gzencode() );

//Test gzencode with one more than the expected number of arguments
echo "\n-- Testing gzencode() function with more than expected no. of arguments --\n";
$data = 'string_val';
$level = 2;
$encoding_mode = FORCE_DEFLATE;
$extra_arg = 10;
var_dump( gzencode($data, $level, $encoding_mode, $extra_arg) );

echo "\n-- Testing with incorrect compression level --\n";
$bad_level = 99;
var_dump(gzencode($data, $bad_level));

echo "\n-- Testing with incorrect encoding_mode --\n";
$bad_mode = 99; 
var_dump(gzencode($data, $level, $bad_mode));

class Tester {
    function Hello() {
        echo "Hello\n"; 
    }
}

echo "\n-- Testing with incorrect parameters --\n";
$testclass = new Tester();
var_dump(gzencode($testclass));
var_dump(gzencode($data, $testclass));
var_dump(gzencode($data, -1, 99.99));
var_dump(gzencode($data, -1, $testclass));
var_dump(gzencode($data, "a very none numeric string\n"));

?>
===Done===