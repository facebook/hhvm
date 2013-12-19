<?php
ini_set('include_path', .);

/* Prototype: string get_include_path  ( void  )
 * Description: Gets the current include_path configuration option

*/

echo "*** Testing get_include_path()\n";

var_dump(get_include_path());

if (ini_get("include_path") == get_include_path()) {
	echo "PASSED\n";
} else {
	echo "FAILED\n";
}		

echo "\nError cases:\n";
var_dump(get_include_path(TRUE));


?>
===DONE===