<?php
/*
   Prototype: void clearstatcache ([bool clear_realpath_cache[, filename]]);
   Description: clears files status cache
*/

echo "*** Testing clearstatcache() function: error conditions ***\n";
var_dump( clearstatcache(0, "/foo/bar", 1) );  //No.of args more than expected
echo "*** Done ***\n";
?>