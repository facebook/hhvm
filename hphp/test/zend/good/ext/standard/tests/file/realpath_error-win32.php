<?php
/* Prototype: string realpath ( string $path );
   Description: Returns canonicalized absolute pathname
*/

echo "*** Testing realpath() for error conditions ***\n";
var_dump( realpath() );  // args < expected
var_dump( realpath(1, 2) );  // args > expected

echo "Done\n";
?>