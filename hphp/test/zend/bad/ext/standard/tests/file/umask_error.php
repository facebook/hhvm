<?php
/* Prototype: int umask ( [int $mask] );
   Description: Changes the current umask
*/

echo "*** Testing umask() : error conditions ***\n";

var_dump( umask(0000, true) );  // args > expected

echo "Done\n";
?>