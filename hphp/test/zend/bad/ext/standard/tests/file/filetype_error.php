<?php
/*
Prototype: string filetype ( string $filename );
Description: Returns the type of the file. Possible values are fifo, char,
             dir, block, link, file, and unknown. 
*/

echo "*** Testing error conditions ***";
/* non-existing file or dir */
print( filetype("/no/such/file/dir") );

/* unknown type */
print( filetype("string") );
print( filetype(100) );

/* No.of args less than expected */
print( filetype() );

/* No.of args greater than expected */
print( filetype("file", "file") );

echo "\n*** Done ***\n";
?>