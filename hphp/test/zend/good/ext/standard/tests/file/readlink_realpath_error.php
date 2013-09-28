<?php
/* Prototype: string readlink ( string $path );
   Description: Returns the target of a symbolic link

   Prototype: string realpath ( string $path );
   Description: Returns canonicalized absolute pathname
*/

echo "*** Testing readlink(): error conditions ***\n";
var_dump( readlink() );  // args < expected
var_dump( readlink(__FILE__, 2) );  // args > expected

echo "\n*** Testing readlink() on a non-existent link ***\n";
var_dump( readlink(dirname(__FILE__)."/readlink_error.tmp") );

echo "\n*** Testing readlink() on existing file ***\n";
var_dump( readlink(__FILE__) );

echo "\n*** Testing readlink() on existing directory ***\n";
var_dump( readlink(dirname(__FILE__)) );

echo "*** Testing realpath(): error conditions ***\n";
var_dump( realpath() );  // args < expected
var_dump( realpath(1, 2) );  // args > expected

echo "\n*** Testing realpath() on a non-existent file ***\n";
var_dump( realpath(dirname(__FILE__)."/realpath_error.tmp") );

echo "Done\n";
?>