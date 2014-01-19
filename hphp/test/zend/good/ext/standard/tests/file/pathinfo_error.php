<?php
/* Prototype: mixed pathinfo ( string $path [, int $options] );
   Description: Returns information about a file path
*/

echo "*** Testing pathinfo() for error conditions ***\n";
/* unexpected no. of arguments */
var_dump( pathinfo() );  /* args < expected */
var_dump( pathinfo("/home/1.html", 1, 3) );  /* args > expected */

echo "Done\n";
?>