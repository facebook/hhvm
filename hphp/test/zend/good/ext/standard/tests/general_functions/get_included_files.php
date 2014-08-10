<?php
/* Prototype: array get_included_files  ( void  )
 * Description: Returns an array with the names of included or required files

*/

echo "*** Testing get_included_files()\n";

echo "\n-- List included files at start --\n";
var_dump(get_included_files());

include(dirname(__FILE__)."/get_included_files_inc1.inc");
echo "\n-- List included files atfter including inc1 -\n";
var_dump(get_included_files());

include(dirname(__FILE__)."/get_included_files_inc2.inc");
echo "\n-- List included files atfter including inc2 which will include inc3 which includes inc1 --\n";
var_dump(get_included_files());

echo "\n-- Error cases --\n";
var_dump(get_included_files(true));

?>
===DONE===
