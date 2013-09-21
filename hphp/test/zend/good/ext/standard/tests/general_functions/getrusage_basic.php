<?php
/* Prototype  :  array getrusage  ([ int $who  ] )
 * Description: Gets the current resource usages
 * Source code: ext/standard/microtime.c
 * Alias to functions: 
 */

echo "Simple testcase for getrusage() function\n";

$dat = getrusage();

if (!is_array($dat)) {
	echo "TEST FAILED : getrusage shoudl return an array\n";
} 	

// echo the fields which are common to all platforms 
echo "User time used (seconds) " . $dat["ru_utime.tv_sec"] . "\n";
echo "User time used (microseconds) " . $dat["ru_utime.tv_usec"] . "\n"; 
?>
===DONE===