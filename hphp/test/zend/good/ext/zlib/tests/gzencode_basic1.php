<?php
/* Prototype  : string gzencode  ( string $data  [, int $level  [, int $encoding_mode  ]] )
 * Description: Gzip-compress a string 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

/*
 * Test basic function of gzencode
 */

include(dirname(__FILE__) . '/data.inc');

echo "*** Testing gzencode() : basic functionality ***\n";

// Initialise all required variables

$smallstring = "A small string to compress\n";


// Calling gzencode() with various compression levels

// Compressing a big string
for($i = -1; $i < 10; $i++) {
    echo "-- Compression level $i --\n";
    $output = gzencode($data, $i);
    
    // Clear OS byte before encode 
	$output[9] = "\x00";
	
    var_dump(md5($output));
}

// Compressing a smaller string
for($i = -1; $i < 10; $i++) {
    echo "-- Compression level $i --\n";
    $output = gzencode($smallstring, $i);
    
    // Clear OS byte before encode 
	$output[9] = "\x00";
	
    var_dump(md5($output));
}

// Calling gzencode() with mandatory arguments
echo "\n-- Testing with no specified compression level --\n";
var_dump(bin2hex(gzencode($smallstring)));

echo "\n-- Testing gzencode with mode specified --\n";
var_dump(bin2hex(gzencode($smallstring, -1, FORCE_GZIP)));

?>
===Done===