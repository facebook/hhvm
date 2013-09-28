<?php
/* Prototype  : string gzcompress(string data [, int level, [int encoding]])
 * Description: Gzip-compress a string 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

/*
 * add a comment here to say what the test is supposed to do
 */

echo "*** Testing gzcompress() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing gzcompress() function with Zero arguments --\n";
var_dump( gzcompress() );

//Test gzcompress with one more than the expected number of arguments
echo "\n-- Testing gzcompress() function with more than expected no. of arguments --\n";
$data = 'string_val';
$level = 2;
$encoding = ZLIB_ENCODING_RAW;
$extra_arg = 10;
var_dump( gzcompress($data, $level, $encoding, $extra_arg) );

echo "\n-- Testing with incorrect compression level --\n";
$bad_level = 99;
var_dump(gzcompress($data, $bad_level));

echo "\n-- Testing with invalid encoding --\n";
$data = 'string_val';
$encoding = 99;
var_dump(gzcompress($data, $level, $encoding));

echo "\n-- Testing with incorrect parameters --\n";

class Tester {
    function Hello() {
        echo "Hello\n"; 
    }
}

$testclass = new Tester();
var_dump(gzcompress($testclass));

?>
===Done===