<?php
/* Prototype  : string gzdeflate(string data [, int level, [int encoding]])
 * Description: Gzip-compress a string 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

/*
 * add a comment here to say what the test is supposed to do
 */

include(dirname(__FILE__) . '/data.inc');

echo "*** Testing gzdeflate() : basic functionality ***\n";

// Initialise all required variables

$smallstring = "A small string to compress\n";


// Calling gzdeflate() with all possible arguments

// Compressing a big string
for($i = -1; $i < 10; $i++) {
    echo "-- Compression level $i --\n";
    $output = gzdeflate($data, $i);
    var_dump(md5($output));
	var_dump(strcmp(gzinflate($output), $data));
}

// Compressing a smaller string
for($i = -1; $i < 10; $i++) {
    echo "-- Compression level $i --\n";
    $output = gzdeflate($smallstring, $i);
    var_dump(bin2hex($output));
	var_dump(strcmp(gzinflate($output), $smallstring));
}

// Calling gzdeflate() with just mandatory arguments
echo "\n-- Testing with no specified compression level --\n";
var_dump( bin2hex(gzdeflate($smallstring) ));

?>
===Done===