<?php
/* Prototype  : bool stream_filter_remove(resource stream_filter)
 * Description: Flushes any data in the filter's internal buffer, removes it from the chain, and frees the resource 
 * Source code: ext/standard/streamsfuncs.c
 * Alias to functions: 
 */

$file = dirname( __FILE__ ) . DIRECTORY_SEPARATOR . 'streamfilterTest.txt';
touch( $file );
$fp = fopen( $file, 'w+' );
$filter = stream_filter_append( $fp, "string.rot13", STREAM_FILTER_WRITE );

echo "*** Testing stream_filter_remove() : error conditions ***\n";

echo "\n-- Testing stream_filter_remove() function with Zero arguments --\n";
var_dump( stream_filter_remove() );

echo "\n-- Testing stream_filter_remove() function with more than expected no. of arguments --\n";
$arg = 'bogus arg';
var_dump( stream_filter_remove( $filter, $arg ) );

echo "\n-- Testing stream_filter_remove() function with unexisting stream filter --\n";
var_dump( stream_filter_remove( "fakefilter" ) );

echo "\n-- Testing stream_filter_remove() function with bad resource --\n";
var_dump( stream_filter_remove( $fp ) );

echo "\n-- Testing stream_filter_remove() function with an already removed filter --\n";
// Double remove it
var_dump( stream_filter_remove( $filter ) );
var_dump( stream_filter_remove( $filter ) );

fclose( $fp );

?>
===DONE===
<?php

$file = dirname( __FILE__ ) . DIRECTORY_SEPARATOR . 'streamfilterTest.txt';
unlink( $file );

?>