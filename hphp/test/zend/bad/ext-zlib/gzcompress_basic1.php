<?php
/* Prototype  : string gzcompress(string data [, int level, [int encoding]])
 * Description: Gzip-compress a string 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

/*
 * add a comment here to say what the test is supposed to do
 */

include(dirname(__FILE__) . '/data.inc');

echo "*** Testing gzcompress() : basic functionality ***\n";

// Initialise all required variables

$smallstring = "A small string to compress\n";


// Calling gzcompress() with all possible arguments

// Compressing a big string
for($i = -1; $i < 10; $i++) {
    echo "-- Compression level $i --\n";
    $output = gzcompress($data, $i);
    var_dump(md5($output));
	var_dump(strcmp(gzuncompress($output), $data));
}

// Compressing a smaller string
for($i = -1; $i < 10; $i++) {
    echo "-- Compression level $i --\n";
    $output = gzcompress($smallstring, $i);
    var_dump(bin2hex($output));
	var_dump(strcmp(gzuncompress($output), $smallstring));
}

// Calling gzcompress() with mandatory arguments
echo "\n-- Testing with no specified compression level --\n";
var_dump( bin2hex(gzcompress($smallstring) ));

?>
===Done===