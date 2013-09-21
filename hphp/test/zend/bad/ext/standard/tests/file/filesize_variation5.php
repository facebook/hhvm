<?php
/* 
 * Prototype   : int filesize ( string $filename );
 * Description : Returns the size of the file in bytes, or FALSE 
 *               (and generates an error of level E_WARNING) in case of an error.
 */

echo "*** Testing filesize(): usage variations ***\n"; 

/* null, false, "", " " */
var_dump( filesize(NULL) );
var_dump( filesize(false) );
var_dump( filesize('') );
var_dump( filesize(' ') );
var_dump( filesize('|') );
echo "*** Done ***\n";
?>