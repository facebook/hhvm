<?php
/* 
Prototype: int fileinode ( string $filename );
Description: Returns the inode number of the file, or FALSE in case of an error.
*/

echo "*** Testing error conditions of fileinode() ***";

/* Non-existing file or dir */
var_dump( fileinode("/no/such/file/dir") );

/* Invalid arguments */
var_dump( fileinode("string") );
var_dump( fileinode(100) );

/* No.of argumetns less than expected */
var_dump( fileinode() );

/* No.of argumetns greater than expected */
var_dump( fileinode(__FILE__, "string") );

echo "\n*** Done ***";
