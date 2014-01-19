<?php
/* Prototype  : string gzuncompress(string data [, int length])
 * Description: Unzip a gzip-compressed string 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

echo "*** Testing gzuncompress() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing gzuncompress() function with Zero arguments --\n";
var_dump( gzuncompress() );

//Test gzuncompress with one more than the expected number of arguments
echo "\n-- Testing gzuncompress() function with more than expected no. of arguments --\n";
$data = 'string_val';
$length = 10;
$extra_arg = 10;
var_dump( gzuncompress($data, $length, $extra_arg) );

echo "\n-- Testing with a buffer that is too small --\n";
$short_len = strlen($data) - 1;
$compressed = gzcompress($data);

var_dump(gzuncompress($compressed, $short_len));

echo "\n-- Testing with incorrect arguments --\n";
var_dump(gzuncompress(123));

class Tester {
    function Hello() {
        echo "Hello\n"; 
    }
}

$testclass = new Tester();
var_dump(gzuncompress($testclass));

var_dump(gzuncompress($compressed, "this is not a number\n"));

?>
===DONE===