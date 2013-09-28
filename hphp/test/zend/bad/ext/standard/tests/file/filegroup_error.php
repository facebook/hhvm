<?php
/* Prototype: int filegroup ( string $filename )
 *  Description: Returns the group ID of the file, or FALSE in case of an error.
 */

echo "*** Testing filegroup(): error conditions ***\n";

/* Non-existing file or dir */
var_dump( filegroup("/no/such/file/dir") );

/* Invalid arguments */
var_dump( filegroup("string") );
var_dump( filegroup(100) );

/* Invalid no.of arguments */
var_dump( filegroup() );  // args < expected
var_dump( filegroup("/no/such/file", "root") );  // args > expected

echo "\n*** Done ***\n";
?>
