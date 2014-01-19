<?php
/* Prototype  : proto bool file_exists(string filename)
 * Description: Returns true if filename exists 
 * Source code: ext/standard/filestat.c
 * Alias to functions: 
 */

echo "*** Testing file_exists() : usage variations ***\n";

var_dump(file_exists(NULL));
var_dump(file_exists(false));
var_dump(file_exists(''));
var_dump(file_exists(' '));
var_dump(file_exists('|'));
echo "Done";
?>
