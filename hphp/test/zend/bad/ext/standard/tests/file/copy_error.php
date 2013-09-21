<?php
/* Prototype: bool copy ( string $source, string $dest );
 * Description: Makes a copy of the file source to dest.
 *              Returns TRUE on success or FALSE on failure.
 */

echo "*** Testing copy() function: error conditions --\n"; 
/* Invalid args */
var_dump( copy("/no/file", "file") );

/* No.of args less than expected */
var_dump( copy() );
var_dump( copy(__FILE__) );

/* No.of args less than expected */
var_dump( copy(__FILE__, "file1", "file1") );

echo "*** Done ***\n";
?>
