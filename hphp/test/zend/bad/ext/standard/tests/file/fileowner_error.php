<?php
/* Prototype: int fileowner ( string $filename )
 * Description: Returns the user ID of the owner of the file, or
 *              FALSE in case of an error.
 */

echo "*** Testing fileowner(): error conditions ***\n";
/* Non-existing file or dir */
var_dump( fileowner("/no/such/file/dir") );

/* Invalid arguments */
var_dump( fileowner("string") );
var_dump( fileowner(100) );

/* Invalid no.of arguments */
var_dump( fileowner() );  // args < expected
var_dump( fileowner("/no/such/file", "root") );  // args > expected

echo "\n*** Done ***\n";
?>
