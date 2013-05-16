<?php
/* Prototype  : string readdir([resource $dir_handle])
 * Description: Read directory entry from dir_handle 
 * Source code: ext/standard/dir.c
 */

/*
 * Open a file pointer using fopen and pass to readdir() to test behaviour
 */

echo "*** Testing readdir() : usage variations ***\n";

// get a resource variable
var_dump($fp = fopen(__FILE__, "r"));
var_dump( readdir($fp) );

?>
===DONE===