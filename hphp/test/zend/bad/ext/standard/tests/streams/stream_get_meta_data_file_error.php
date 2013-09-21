<?php
/* Prototype  : proto array stream_get_meta_data(resource fp)
 * Description: Retrieves header/meta data from streams/file pointers 
 * Source code: ext/standard/streamsfuncs.c
 * Alias to functions: socket_get_status
 */

echo "*** Testing stream_get_meta_data() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing stream_get_meta_data() function with Zero arguments --\n";
var_dump( stream_get_meta_data() );

//Test stream_get_meta_data with one more than the expected number of arguments
echo "\n-- Testing stream_get_meta_data() function with more than expected no. of arguments --\n";

$fp = null;
$extra_arg = 10;
var_dump( stream_get_meta_data($fp, $extra_arg) );

echo "\n-- Testing stream_get_meta_data() function with invalid stream resource --\n";
$fp = null;
var_dump(stream_get_meta_data($fp));

echo "\n-- Testing stream_get_meta_data() function with closed stream resource --\n";
$fp = fopen(__FILE__, 'r');
fclose($fp);
var_dump(stream_get_meta_data($fp));

echo "Done";
?>