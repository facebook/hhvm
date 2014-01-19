<?php
/* Prototype: bool fnmatch ( string $pattern, string $string [, int $flags] )
   Description: fnmatch() checks if the passed string would match 
     the given shell wildcard pattern. 
*/

echo "*** Testing error conditions for fnmatch() ***";

/* Invalid arguments */
var_dump( fnmatch(array(), array()) );

$file_handle = fopen(__FILE__, "r");
var_dump( fnmatch($file_handle, $file_handle) );
fclose( $file_handle );

$std_obj = new stdClass();
var_dump( fnmatch($std_obj, $std_obj) );


/* No.of arguments less than expected */
var_dump( fnmatch("match.txt") );
var_dump( fnmatch("") );

/* No.of arguments greater than expected */
var_dump( fnmatch("match.txt", "match.txt", TRUE, 100) );

echo "\n*** Done ***\n";
?>