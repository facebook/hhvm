<?php
/* Prototype  : string readdir([resource $dir_handle])
 * Description: Read directory entry from dir_handle 
 * Source code: ext/standard/dir.c
 */

/*
 * Pass incorrect number of arguments to readdir() to test behaviour
 */

echo "*** Testing readdir() : error conditions ***\n";


//Test readdir with one more than the expected number of arguments
echo "\n-- Testing readdir() function with more than expected no. of arguments --\n";

$path = dirname(__FILE__) . "/readdir_error";
mkdir($path);
$dir_handle = opendir($path);
$extra_arg = 10;

try { var_dump( readdir($dir_handle, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// close the handle so can remove dir in CLEAN section
closedir($dir_handle);
?>
===DONE===
<?php error_reporting(0); ?>
<?php
$path = dirname(__FILE__) . "/readdir_error";
rmdir($path);
?> 