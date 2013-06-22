<?php
/*
   Prototype: int fileatime ( string $filename );
   Description: Returns the time the file was last accessed, or FALSE
     in case of an error. The time is returned as a Unix timestamp.

   Prototype: int filemtime ( string $filename );
   Description: Returns the time the file was last modified, or FALSE
     in case of an error.

   Prototype: int filectime ( string $filename );
   Description: Returns the time the file was last changed, or FALSE
     in case of an error. The time is returned as a Unix timestamp.

   Prototype: bool touch ( string $filename [, int $time [, int $atime]] );
   Description: Attempts to set the access and modification times of the file
     named in the filename parameter to the value given in time.
*/

echo "*** Testing error conditions ***\n";

echo "\n-- Testing with  Non-existing files --";
/* Both invalid arguments */
var_dump( fileatime("/no/such/file/or/dir") );
var_dump( filemtime("/no/such/file/or/dir") );
var_dump( filectime("/no/such/file/or/dir") );
var_dump( touch("/no/such/file/or/dir", 10) );

/* Only one invalid argument */
var_dump( fileatime(__FILE__, "string") );
var_dump( filemtime(__FILE__, 100) );
var_dump( filectime(__FILE__, TRUE) );
var_dump( touch(__FILE__, 10, 100, 123) );

echo "\n-- Testing No.of arguments less than expected --";
var_dump( fileatime() );
var_dump( filemtime() );
var_dump( filectime() );
var_dump( touch() );

echo "\n-- Testing No.of arguments greater than expected --";
/* Both invalid arguments */
var_dump( fileatime("/no/such/file/or/dir", "string") );
var_dump( filemtime("/no/such/file/or/dir", 100) );
var_dump( filectime("/no/such/file/or/dir", TRUE) );
var_dump( touch("/no/such/file/or/dir", 10, 100, 123) );

/* Only one invalid argument */
var_dump( fileatime(__FILE__, "string") );
var_dump( filemtime(__FILE__, 100) );
var_dump( filectime(__FILE__, TRUE) );
var_dump( touch(__FILE__, 10, 100, 123) );

echo "\nDone";
?>