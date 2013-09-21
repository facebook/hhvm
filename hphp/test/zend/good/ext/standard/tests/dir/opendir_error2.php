<?php
/* Prototype  : mixed opendir(string $path[, resource $context])
 * Description: Open a directory and return a dir_handle 
 * Source code: ext/standard/dir.c
 */

/*
 * Pass a non-existent directory as $path argument to opendir() to test behaviour
 */

echo "*** Testing opendir() : error conditions ***\n";

echo "\n-- Pass a non-existent absolute path: --\n";
$path = dirname(__FILE__) . "/idonotexist";
var_dump(opendir($path));

echo "\n-- Pass a non-existent relative path: --\n";
chdir(dirname(__FILE__));
var_dump(opendir('idonotexist'));
?>
===DONE===