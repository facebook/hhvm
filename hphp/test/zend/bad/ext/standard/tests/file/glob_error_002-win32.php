<?php
/* Prototype: array glob ( string $pattern [, int $flags] );
   Description: Find pathnames matching a pattern
*/

echo "*** Testing glob() : error condition - pattern too long. ***\n";

var_dump(glob(str_repeat('x', 3000)));

echo "Done";
?>