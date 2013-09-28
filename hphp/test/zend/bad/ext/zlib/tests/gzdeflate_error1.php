<?php
/* Prototype  : string gzdeflate(string data [, int level, [int encoding]])
 * Description: Gzip-compress a string 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

/*
 * add a comment here to say what the test is supposed to do
 */

echo "*** Testing gzdeflate() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing gzdeflate() function with Zero arguments --\n";
var_dump( gzdeflate() );

//Test gzdeflate with one more than the expected number of arguments
echo "\n-- Testing gzdeflate() function with more than expected no. of arguments --\n";
$data = 'string_val';
$level = 2;
$encoding = ZLIB_ENCODING_RAW;
$extra_arg = 10;
var_dump( gzdeflate($data, $level, $encoding, $extra_arg) );

echo "\n-- Testing with incorrect compression level --\n";
$bad_level = 99; 
var_dump(gzdeflate($data, $bad_level));

echo "\n-- Testing with incorrect encoding --\n";
$bad_encoding = 99; 
var_dump(gzdeflate($data, $level, $bad_encoding));

class Tester {
    function Hello() {
        echo "Hello\n"; 
    }
}

echo "\n-- Testing with incorrect parameters --\n";
$testclass = new Tester();
var_dump(gzdeflate($testclass));
var_dump(gzdeflate($data, $testclass));

?>
===Done===