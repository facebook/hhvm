<?php
/* Prototype: int get_magic_quotes_runtime  ( void  )
 * This function is not supported anymore and will always return false
 */

echo "Simple testcase for get_magic_quotes_runtime() function\n";

var_dump(get_magic_quotes_runtime());

echo "\n-- Error cases --\n"; 
// no checks on number of args
var_dump(get_magic_quotes_runtime(true)); 

?>
===DONE===