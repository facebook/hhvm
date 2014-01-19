<?php
/* Prototype  : bool chdir(string $directory)
 * Description: Change the current directory 
 * Source code: ext/standard/dir.c
 */

/*
 * Pass a directory that does not exist as $directory to chdir() to test behaviour
 */

echo "*** Testing chdir() : error conditions ***\n";

$directory = __FILE__ . '/idonotexist';

var_dump(chdir($directory));
?>
===DONE===