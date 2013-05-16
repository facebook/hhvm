<?php
/* Prototype: array lstat ( string $filename );
   Description: Gives information about a file or symbolic link

   Prototype: array stat ( string $filename );
   Description: Gives information about a file
*/
echo "*** testing stat ***\n";
var_dump(stat(NULL));
var_dump(stat(false));
var_dump(stat(''));
var_dump(stat(' '));
var_dump(stat('|'));

echo "*** testing lstat ***\n";
var_dump(lstat(NULL));
var_dump(lstat(false));
var_dump(lstat(''));
var_dump(lstat(' '));
var_dump(lstat('|'));
echo "Done";
?>