<?php
/* Prototype  : string escapeshellarg  ( string $arg  )
 * Description: Escape a string to be used as a shell argument.
 * Source code: ext/standard/exec.c
 * Alias to functions: 
 */

echo "Simple testcase for escapeshellarg() function\n";

var_dump(escapeshellarg("Mr O'Neil"));
var_dump(escapeshellarg("Mr O\'Neil"));
var_dump(escapeshellarg("%FILENAME"));
var_dump(escapeshellarg(""));

echo "Done\n";
?>